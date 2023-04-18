#include "Graph.h"

#include <algorithm>
#include <utility>

#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Scene/Scene.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Fence.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Image.h"
#include "Vulkan/Queue.h"

namespace Hermes
{
	static void SplitResourceName(const String& ResourceName, String& PassName, String& OwnName)
	{
		auto DotIndex = ResourceName.find_first_of('.');
		HERMES_ASSERT(DotIndex != ResourceName.npos);
		PassName = String(ResourceName.begin(),
		                  ResourceName.begin() + static_cast<std::string::difference_type>(DotIndex));
		OwnName = String(ResourceName.begin() + static_cast<std::string::difference_type>(DotIndex) + 1,
		                 ResourceName.end());
	}

	static VkImageLayout PickImageLayoutForBindingMode(BindingMode Mode)
	{
		switch (Mode)
		{
		case BindingMode::DepthStencilAttachment:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case BindingMode::ColorAttachment:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case BindingMode::SampledImage:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		HERMES_ASSERT(false)
	}

	void FrameGraphScheme::AddPass(const String& Name, const PassDesc& Desc)
	{
		HERMES_ASSERT_LOG(!Passes.contains(Name), "Trying to duplicate pass with name %s", Name.c_str());
		Passes[Name] = Desc;
	}

	void FrameGraphScheme::AddLink(const String& From, const String& To)
	{
		HERMES_ASSERT_LOG(!BackwardLinks.contains(To),
		                  "Trying to create link from %s to %s while link from %s to %s already exists",
		                  From.c_str(), To.c_str(), BackwardLinks[To].c_str(), To.c_str());

		BackwardLinks[To] = From;
		ForwardLinks[From] = To;
	}

	void FrameGraphScheme::AddResource(const String& Name, const ImageResourceDescription& Description, bool IsExternal)
	{
		ImageResources.emplace_back(Name, Description, IsExternal);
	}

	void FrameGraphScheme::AddResource(const String& Name, const BufferResourceDescription& Description, bool IsExternal)
	{
		BufferResources.emplace_back(Name, Description, IsExternal);
	}

	std::unique_ptr<FrameGraph> FrameGraphScheme::Compile() const
	{
		if (!Validate())
		{
			return nullptr;
		}

		return std::unique_ptr<FrameGraph>(new FrameGraph(*this));
	}

	// TODO: check that images are linked to images and buffers are linked to buffers
	bool FrameGraphScheme::Validate() const
	{
		// Step 1: check for any unacceptable names of passes, resources and attachments
		for (const auto& Pass : Passes)
		{
			// Pass name cannot have '$' character as it is reserved for pass that provides external resources
			if (Pass.first.find(L'$') != String::npos)
			{
				HERMES_LOG_ERROR("Invalid render pass name %s", Pass.first.c_str());
				return false;
			}
			// Pass name cannot have '.' character as it is used as separator between pass and attachment name
			if (Pass.first.find(L'.') != String::npos)
			{
				HERMES_LOG_ERROR("Invalid render pass name %s", Pass.first.c_str());
				return false;
			}

			// Check every attachment of this pass
			for (const auto& Attachment : Pass.second.Attachments)
			{
				// Attachment name cannot have '.' character as it is used as separator
				// between pass and attachment name
				if (Attachment.Name.find(L'.') != String::npos)
				{
					HERMES_LOG_ERROR("Invalid attachment name %s in render pas %s", Attachment.Name.c_str(),
					                 Pass.first.c_str());
					return false;
				}
			}
		}
		for (const auto& Resource : ImageResources)
		{
			// Resource name cannot have '.' character as it is used as separator between pass and attachment name
			if (Resource.Name.find(L'.') != String::npos)
			{
				HERMES_LOG_ERROR("Invalid resource name %s", Resource.Name.c_str());
				return false;
			}
		}

		// Step 2: check that all links have valid start and end pass and attachment name
		for (const auto& Link : ForwardLinks)
		{
			// There must be a dot in the link from and to names
			if (Link.first.find(L'.') == String::npos || Link.second.find(L'.') == String::npos)
			{
				HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s'", Link.first.c_str(), Link.second.c_str());
				return false;
			}

			String FirstPassName, FirstResourceName, SecondPassName, SecondResourceName;
			SplitResourceName(Link.first, FirstPassName, FirstResourceName);
			SplitResourceName(Link.second, SecondPassName, SecondResourceName);

			// Checking the first resource name (link 'from')
			if (FirstPassName == "$")
			{
				bool Found = false;
				// If the first pass is the pass that contains external resources then check if
				// such resource exists
				if (std::ranges::find_if(ImageResources, [&](const auto& Element)
				{
					return Element.Name == FirstResourceName;
				}) != ImageResources.end())
				{
					Found = true;
				}
				// Also check buffer resources
				if (std::ranges::find_if(BufferResources, [&](const auto& Element)
				{
					return Element.Name == FirstResourceName;
				}) != BufferResources.end())
				{
					Found = true;
				}

				if (!Found)
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': resource '%s' does not exist",
					                 Link.first.c_str(), Link.second.c_str(), FirstResourceName.c_str());
					return false;
				}
			}
			else
			{
				// Otherwise, check if the render pass and the attachment with such name exists
				auto FirstPass = Passes.find(FirstPassName);
				if (FirstPass == Passes.end())
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': pass '%s' does not exist", Link.first.c_str(),
					                 Link.second.c_str(), FirstPassName.c_str());
					return false;
				}

				bool Found = false;
				// Check attachments
				if (std::ranges::find_if(FirstPass->second.Attachments, [&](const auto& Element)
				{
					return Element.Name == FirstResourceName;
				}) != FirstPass->second.Attachments.end())
				{
					Found = true;
				}
				// And buffer inputs
				if (std::ranges::find_if(FirstPass->second.BufferInputs, [&](const auto& Element)
				{
					return Element.Name == FirstResourceName;
				}) != FirstPass->second.BufferInputs.end())
				{
					Found = true;
				}

				if (!Found)
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': pass '%s' does not contain attachmet '%s'",
					                 Link.first.c_str(), Link.second.c_str(), FirstPassName.c_str(),
					                 FirstResourceName.c_str());
					return false;
				}
			}

			// Checking the second resource name (link 'to')
			if (SecondPassName == "$")
			{
				// If the second pass is the external pass (pass that contains external resources)
				// then check if the attachment name is equal to FINAL_IMAGE as it is the
				// only resource that can be pointed to by some render pass
				if (SecondResourceName != "FINAL_IMAGE")
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': only FINAL_IMAGE is allowed as attachment name for the external render pass",
					                 Link.first.c_str(), Link.second.c_str());
					return false;
				}
			}
			else
			{
				// Otherwise, check if the render pass and the resource with such name exists
				auto SecondPass = Passes.find(SecondPassName);
				if (SecondPass == Passes.end())
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': pass '%s' does not exist", Link.first.c_str(),
					                 Link.second.c_str(), SecondPassName.c_str());
					return false;
				}

				bool Found = false;
				// Look in the attachment list
				if (std::ranges::find_if(SecondPass->second.Attachments, [&](const auto& Element)
				{
					return Element.Name == SecondResourceName;
				}) != SecondPass->second.Attachments.end())
				{
					Found = true;
				}
				// And input buffer list
				if (std::ranges::find_if(SecondPass->second.BufferInputs, [&](const auto& Element)
				{
					return Element.Name == SecondResourceName;
				}) != SecondPass->second.BufferInputs.end())
				{
					Found = true;
				}

				if (!Found)
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': pass '%s' does not contain attachmet '%s'",
					                 Link.first.c_str(), Link.second.c_str(), SecondPassName.c_str(),
					                 SecondResourceName.c_str());
					return false;
				}
			}
		}

		// TODO : add more checks (acyclic graphs, more than one component etc.)

		return true;
	}

	void FrameGraph::BindExternalResource(const String& Name, const Vulkan::Image& Image, const Vulkan::ImageView& View, VkImageLayout CurrentLayout)
	{
		HERMES_ASSERT(ImageResources.contains(Name) && ImageResources[Name].IsExternal);

		ImageResources[Name].ExternalImage = &Image;
		ImageResources[Name].ExternalView = &View;
		ImageResources[Name].CurrentLayout = CurrentLayout;
	}

	void FrameGraph::BindExternalResource(const String& Name, const Vulkan::Buffer& Buffer)
	{
		HERMES_ASSERT(BufferResources.contains(Name) && BufferResources[Name].IsExternal);

		BufferResources[Name].ExternalBuffer = &Buffer;
	}

	void FrameGraph::Execute(const Scene& Scene, const GeometryList& GeometryList, Vec2ui ViewportDimensions)
	{
		HERMES_PROFILE_FUNC();

		if (ViewportDimensions.X == 0 || ViewportDimensions.Y == 0)
			return;

		if (ViewportDimensions != CurrentViewportDimensions)
		{
			CurrentViewportDimensions = ViewportDimensions;

			RecreateResources();
		}

		std::vector<std::unique_ptr<Vulkan::Fence>> Fences;
		std::vector<std::unique_ptr<Vulkan::CommandBuffer>> CommandBuffers;
		for (const auto& PassName : PassExecutionOrder)
		{
			HERMES_PROFILE_SCOPE("Hermes::FrameGraph::Execute per pass loop");
			const auto& Pass = Passes[PassName];

			VkQueueFlags QueueType = 0;
			if (Scheme.Passes[PassName].Type == PassType::Graphics)
				QueueType = VK_QUEUE_GRAPHICS_BIT;
			else if (Scheme.Passes[PassName].Type == PassType::Compute)
				QueueType = VK_QUEUE_COMPUTE_BIT;
			auto& Queue = Renderer::GetDevice().GetQueue(QueueType);

			auto CommandBuffer = Renderer::GetDevice().GetQueue(VK_QUEUE_GRAPHICS_BIT).CreateCommandBuffer();
			CommandBuffer->BeginRecording();

			std::vector<VkImageMemoryBarrier> Barriers(Pass.ImageResourceLayouts.size());
			size_t BarrierIndex = 0;
			for (const auto& [ResourceName, Layout] : Pass.ImageResourceLayouts)
			{
				auto& Resource = ImageResources[ResourceName];

				Barriers[BarrierIndex].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				Barriers[BarrierIndex].image = Resource.GetImage().GetImage();

				Barriers[BarrierIndex].subresourceRange = Resource.Image->GetFullSubresourceRange();
				Barriers[BarrierIndex].oldLayout = Resource.CurrentLayout;
				Barriers[BarrierIndex].newLayout = Layout;

				// TODO : this is a general solution that covers all required synchronization cases, but we should rather
				// detect or require from user which types of operations would be performed and implement proper barrier
				Barriers[BarrierIndex].srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
				Barriers[BarrierIndex].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

				Resource.CurrentLayout = Layout;
				BarrierIndex++;
			}
			VkPipelineStageFlags ImageBarrierSourceStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			CommandBuffer->InsertImageMemoryBarriers({ Barriers.begin(), Barriers.end() }, ImageBarrierSourceStages,
			                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

			std::vector<VkBufferMemoryBarrier> BufferBarriers;
			for (const auto& [BufferName, ResourceName] :Pass.BufferInputResourceNames)
			{
				const auto& Buffer = BufferResources[ResourceName];

				VkBufferMemoryBarrier Barrier = {};
				Barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				Barrier.buffer = Buffer.GetBuffer().GetBuffer();

				// TODO : this is a general solution that covers all required synchronization cases, but we should rather
				// detect or require from user which types of operations would be performed and implement proper barrier
				Barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
				Barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

				Barrier.offset = 0;
				Barrier.size = static_cast<uint32>(Buffer.GetBuffer().GetSize());

				BufferBarriers.push_back(Barrier);
			}

			// TODO: do we need more? (also see the TODO above)
			VkPipelineStageFlags BufferBarrierSourceStages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
			CommandBuffer->InsertBufferMemoryBarriers({ BufferBarriers.begin(), BufferBarriers.end() }, BufferBarrierSourceStages, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

			if (Scheme.Passes[PassName].Type == PassType::Graphics)
			{
				VkRect2D RenderingArea = {
					.offset = { 0, 0 },
					.extent = { ViewportDimensions.X, ViewportDimensions.Y }
				};

				std::vector<VkRenderingAttachmentInfo> ColorAttachmentsInfo;
				for (const auto& [Name, Info] : Pass.ColorAttachments)
					ColorAttachmentsInfo.push_back(Info);

				auto DepthAttachmentInfo = Pass.DepthAttachment.has_value() ? std::make_optional(Pass.DepthAttachment.value().second) : std::nullopt;
				CommandBuffer->BeginRendering(RenderingArea, ColorAttachmentsInfo, DepthAttachmentInfo, std::nullopt);
			}

			std::unordered_map<String, PassResourceVariant> PassResources;
			for (const auto& [AttachmentName, ResourceName] : Pass.ImageAttachmentResourceNames)
			{
				const auto& Resource = ImageResources.at(ResourceName);
				PassResources[AttachmentName] = &Resource.GetView();
			}
			for (const auto& [BufferName, ResourceName] : Pass.BufferInputResourceNames)
			{
				const auto& Resource = BufferResources.at(ResourceName);
				PassResources[BufferName] = &Resource.GetBuffer();
			}

			PassCallbackInfo CallbackInfo = {
				.CommandBuffer = *CommandBuffer,
				.Resources = PassResources,
				.Scene = Scene,
				.GeometryList = GeometryList,
			};

			Pass.Callback(CallbackInfo);

			if (Scheme.Passes[PassName].Type == PassType::Graphics)
			{
				CommandBuffer->EndRendering();
			}
			CommandBuffer->EndRecording();

			auto Fence = Renderer::GetDevice().CreateFence();
			Queue.SubmitCommandBuffer(*CommandBuffer, Fence.get());
			Fences.push_back(std::move(Fence));

			// NOTE: this way we can delay the destruction of command buffer until the end of function scope after we wait for it to finish
			CommandBuffers.push_back(std::move(CommandBuffer));
		}

		for (const auto& Fence : Fences)
			Fence->Wait(UINT64_MAX);
	}

	std::pair<const Vulkan::Image*, VkImageLayout> FrameGraph::GetFinalImage() const
	{
		HERMES_ASSERT(!FinalImageResourceName.empty());
		const auto& Resource = ImageResources.at(FinalImageResourceName);
		return std::make_pair(&Resource.GetImage(), Resource.CurrentLayout);
	}

	FrameGraph::FrameGraph(FrameGraphScheme InScheme)
		: Scheme(std::move(InScheme))
	{
		for (const auto& Resource : Scheme.ImageResources)
		{
			ImageResourceContainer Container = {};

			Container.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			Container.Desc = Resource.Desc;
			Container.IsExternal = Resource.IsExternal;

			ImageResources[Resource.Name] = std::move(Container);
		}

		for (const auto& Resource : Scheme.BufferResources)
		{
			BufferResourceContainer Container = {};

			Container.Desc = Resource.Desc;
			Container.IsExternal = Resource.IsExternal;

			if (!Resource.IsExternal)
				Container.Buffer = Renderer::GetDevice().CreateBuffer(Resource.Desc.Size, TraverseBufferResourceUsageType(Resource.Name), TraverseCheckIfBufferIsMappable(Resource.Name));

			BufferResources[Resource.Name] = std::move(Container);
		}

		for (const auto& [PassName, PassDesc] : Scheme.Passes)
		{
			PassContainer NewPassContainer = {};
			NewPassContainer.Callback = PassDesc.Callback;

			// TODO : clean this code up
			NewPassContainer.ClearColors.reserve(PassDesc.Attachments.size());
			NewPassContainer.ImageResourceLayouts.reserve(PassDesc.Attachments.size());
			for (const auto& Attachment : PassDesc.Attachments)
			{
				String FullResourceName = TraverseResourceName(PassName + "." + Attachment.Name);

				String Dummy, ResourceOwnName;
				SplitResourceName(FullResourceName, Dummy, ResourceOwnName);
				
				VkRenderingAttachmentInfo AttachmentInfo = {
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.pNext = nullptr,
					.imageView = VK_NULL_HANDLE,
					.imageLayout = PickImageLayoutForBindingMode(Attachment.Binding),
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.resolveImageView = VK_NULL_HANDLE,
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.loadOp = Attachment.LoadOp,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE, // FIXME: can we really just set it to a fixed value
					.clearValue = Attachment.ClearColor
				};

				if (Attachment.Binding == BindingMode::ColorAttachment)
					NewPassContainer.ColorAttachments.emplace_back(ResourceOwnName, AttachmentInfo);
				else if (Attachment.Binding == BindingMode::DepthStencilAttachment)
					NewPassContainer.DepthAttachment = std::make_pair(ResourceOwnName, AttachmentInfo);

				NewPassContainer.ClearColors.push_back(Attachment.ClearColor);
				NewPassContainer.ImageResourceLayouts.emplace_back(ResourceOwnName, PickImageLayoutForBindingMode(Attachment.Binding));

				NewPassContainer.ImageAttachmentResourceNames.emplace_back(Attachment.Name, ResourceOwnName);
			}

			for (const auto& BufferInput : PassDesc.BufferInputs)
			{
				String FullResourceName = TraverseResourceName(PassName + "." + BufferInput.Name);

				String Dummy, ResourceOwnName;
				SplitResourceName(FullResourceName, Dummy, ResourceOwnName);

				NewPassContainer.BufferInputResourceNames.emplace_back(BufferInput.Name, ResourceOwnName);
			}

			Passes[PassName] = std::move(NewPassContainer);
		}

		HERMES_ASSERT_LOG(Scheme.BackwardLinks.contains("$.FINAL_IMAGE"),
		                  "Render graph scheme does not contain link that points to a final image");
		auto FinalImageResource = TraverseResourceName("$.FINAL_IMAGE");
		String DummyDollarSign;
		SplitResourceName(FinalImageResource, DummyDollarSign, FinalImageResourceName);

		// NOTE : topological sort to get a linear sequence of render passes where every resource for render pass
		//        is either fresh image or was generated by a previous render pass

		// Step 1: make a copy of the graph
		auto LocalForwardLinks = Scheme.ForwardLinks;
		auto LocalPasses = Scheme.Passes;

		// Step 2: remove any links from and to the '$' render pass
		for (auto Link = LocalForwardLinks.begin(); Link != LocalForwardLinks.end();)
		{
			if (Link->first.starts_with("$.") || Link->second.starts_with("$."))
			{
				Link = LocalForwardLinks.erase(Link);
			}
			else
			{
				++Link;
			}
		}

		// Step 3: until there is at least one pass left untouched
		while (!LocalPasses.empty())
		{
			// Step 3.1: iterate over all untouched passes
			for (auto CurrentPass = LocalPasses.begin(); CurrentPass != LocalPasses.end();)
			{
				// Step 3.2: check if this pass does not have any outgoing links
				bool HasOutgoingConnections = false;
				for (const auto& Link : LocalForwardLinks)
				{
					if (Link.first.starts_with(CurrentPass->first + "."))
					{
						HasOutgoingConnections = true;
						break;
					}
				}

				if (!HasOutgoingConnections)
				{
					// If it does not have any outgoing links then:

					// 1. Add this pass to the beginning of the pass execution order list then:
					PassExecutionOrder.insert(PassExecutionOrder.begin(), CurrentPass->first);

					// 2. Remove all links that point to it
					for (auto Link = LocalForwardLinks.begin(); Link != LocalForwardLinks.end();)
					{
						if (Link->second.starts_with(CurrentPass->first + "."))
						{
							Link = LocalForwardLinks.erase(Link);
						}
						else
						{
							++Link;
						}
					}

					// And remove it from the local graph copy
					CurrentPass = LocalPasses.erase(CurrentPass);
				}
				else
				{
					// Otherwise keep iterating
					++CurrentPass;
				}
			}
		}
	}

	String FrameGraph::TraverseResourceName(const String& FullAttachmentName)
	{
		String CurrentAttachmentName = Scheme.BackwardLinks[FullAttachmentName];
		while (CurrentAttachmentName.find_first_of('$') != 0)
		{
			CurrentAttachmentName = Scheme.BackwardLinks[CurrentAttachmentName];
		}
		return CurrentAttachmentName;
	}

	VkImageUsageFlags FrameGraph::TraverseImageResourceUsageType(const String& ResourceName) const
	{
		VkImageUsageFlags Result = 0;
		String CurrentAttachmentName = "$." + ResourceName;
		while (true)
		{
			if (!Scheme.ForwardLinks.contains(CurrentAttachmentName))
				break;
			auto NextAttachmentName = Scheme.ForwardLinks.at(CurrentAttachmentName);
			String CurrentAttachmentRenderPassName, CurrentAttachmentOwnName;
			SplitResourceName(NextAttachmentName, CurrentAttachmentRenderPassName, CurrentAttachmentOwnName);

			if (NextAttachmentName == "$.FINAL_IMAGE")
			{
				Result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				break;
			}

			auto CurrentAttachmentRenderPass = Scheme.Passes.at(CurrentAttachmentRenderPassName);
			auto CurrentAttachment = std::ranges::find_if(CurrentAttachmentRenderPass.Attachments,
			                                              [&](const Attachment& Element)
			                                              {
				                                              return Element.Name == CurrentAttachmentOwnName;
			                                              });

			switch (CurrentAttachment->Binding)
			{
			case BindingMode::ColorAttachment:
				Result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				break;
			case BindingMode::SampledImage:
				Result |= VK_IMAGE_USAGE_SAMPLED_BIT;
				break;
			case BindingMode::DepthStencilAttachment:
				Result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				break;
			default:
				HERMES_ASSERT(false)
			}

			if (!Scheme.ForwardLinks.contains(NextAttachmentName))
				break;

			CurrentAttachmentName = NextAttachmentName;
		}

		return Result;
	}

	VkBufferUsageFlags FrameGraph::TraverseBufferResourceUsageType(const String& ResourceName) const
	{
		VkBufferUsageFlags Result = 0;

		auto CurrentResourceName = "$." + ResourceName;
		while (true)
		{
			if (!Scheme.ForwardLinks.contains(CurrentResourceName))
				break;

			const auto& CurrentInputName = Scheme.ForwardLinks.at(CurrentResourceName);
			String CurrentPassName, CurrentInputOwnName;
			SplitResourceName(CurrentInputName, CurrentPassName, CurrentInputOwnName);

			const auto& CurrentPass = Scheme.Passes.at(CurrentPassName);
			const auto& CurrentBufferInput = std::ranges::find_if(CurrentPass.BufferInputs,
			                                                      [&](const BufferInput& Value)
			                                                      {
				                                                      return Value.Name == CurrentInputOwnName;
			                                                      });

			Result |= CurrentBufferInput->Usage;

			CurrentResourceName = Scheme.ForwardLinks.at(CurrentResourceName);
		}

		return Result;
	}

	bool FrameGraph::TraverseCheckIfBufferIsMappable(const String& ResourceName) const
	{
		auto CurrentResourceName = "$." + ResourceName;
		while (true)
		{
			if (!Scheme.ForwardLinks.contains(CurrentResourceName))
				break;

			const auto& CurrentInputName = Scheme.ForwardLinks.at(CurrentResourceName);
			String CurrentPassName, CurrentInputOwnName;
			SplitResourceName(CurrentInputName, CurrentPassName, CurrentInputOwnName);

			const auto& CurrentPass = Scheme.Passes.at(CurrentPassName);
			const auto& CurrentBufferInput = std::ranges::find_if(CurrentPass.BufferInputs,
			                                                      [&](const BufferInput& Value)
			                                                      {
				                                                      return Value.Name == CurrentInputOwnName;
			                                                      });

			if (CurrentBufferInput->RequiresMapping)
				return true;

			CurrentResourceName = Scheme.ForwardLinks.at(CurrentResourceName);
		}

		return false;
	}

	VkFormat FrameGraph::TraverseAttachmentDataFormat(const String& AttachmentName) const
	{
		auto CurrentAttachment = Scheme.BackwardLinks.at(AttachmentName);
		while (CurrentAttachment[0] != L'$')
		{
			CurrentAttachment = Scheme.BackwardLinks.at(CurrentAttachment);
		}

		String ResourcePassName, ResourceOwnName;
		SplitResourceName(CurrentAttachment, ResourcePassName, ResourceOwnName);
		HERMES_ASSERT(ResourcePassName == "$");

		const auto& Resource = ImageResources.at(ResourceOwnName);
		return Resource.Desc.Format;
	}

	void FrameGraph::RecreateResources()
	{
		for (auto& Resource : ImageResources)
		{
			if (Resource.second.Desc.Dimensions.IsRelative() && !Resource.second.IsExternal)
			{
				Resource.second.Image = Renderer::GetDevice().CreateImage(Resource.second.Desc.Dimensions.GetAbsoluteDimensions(CurrentViewportDimensions), TraverseImageResourceUsageType(Resource.first), Resource.second.Desc.Format, Resource.second.Desc.MipLevels);
				Resource.second.View = Resource.second.Image->CreateDefaultImageView();

				Resource.second.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
		}

		// Now we need to update VkRenderingAttachmentInfos for every render pass we have
		for (auto& [PassName, Pass] : Passes)
		{
			for (auto& [ResourceName, AttachmentInfo] : Pass.ColorAttachments)
			{
				auto& Resource = ImageResources.at(ResourceName);
				AttachmentInfo.imageView = Resource.View->GetImageView();
			}

			if (Pass.DepthAttachment.has_value())
			{
				auto& Resource = ImageResources.at(Pass.DepthAttachment.value().first);
				Pass.DepthAttachment.value().second.imageView = Resource.View->GetImageView();
			}
		}
	}

	const Vulkan::Image& FrameGraph::ImageResourceContainer::GetImage() const
	{
		if (IsExternal)
			return *ExternalImage;
		else
			return *Image;
	}

	const Vulkan::ImageView& FrameGraph::ImageResourceContainer::GetView() const
	{
		if (IsExternal)
			return *ExternalView;
		else
			return *View;
	}

	const Vulkan::Buffer& FrameGraph::BufferResourceContainer::GetBuffer() const
	{
		if (IsExternal)
			return *ExternalBuffer;
		else
			return *Buffer;
	}
}
