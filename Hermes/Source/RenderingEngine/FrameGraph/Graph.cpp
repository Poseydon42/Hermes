#include "Graph.h"

#include <algorithm>
#include <utility>

#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Scene/Scene.h"
#include "Vulkan/Swapchain.h"
#include "Vulkan/Fence.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/RenderPass.h"
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
		case BindingMode::InputAttachment:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		HERMES_ASSERT(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
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

	void FrameGraphScheme::AddResource(const String& Name, const ImageResourceDescription& Description)
	{
		ImageResources.emplace_back(Name, Description, false);
	}

	void FrameGraphScheme::DeclareExternalResource(const String& Name, const ImageResourceDescription& Description)
	{
		ImageResources.emplace_back(Name, Description, true);
	}

	std::unique_ptr<FrameGraph> FrameGraphScheme::Compile() const
	{
		if (!Validate())
		{
			return nullptr;
		}

		return std::unique_ptr<FrameGraph>(new FrameGraph(*this));
	}

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

			String FirstPassName, FirstAttachmentName, SecondPassName, SecondAttachmentName;
			SplitResourceName(Link.first, FirstPassName, FirstAttachmentName);
			SplitResourceName(Link.second, SecondPassName, SecondAttachmentName);

			// Checking the first resource name (link 'from')
			if (FirstPassName == "$")
			{
				// If the first pass is the pass that contains external resources then check if
				// such resource exists
				if (std::ranges::find_if(ImageResources, [&](const auto& Element)
				{
					return Element.Name == FirstAttachmentName;
				}) == ImageResources.end())
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': resource '%s' does not exist",
					                 Link.first.c_str(), Link.second.c_str(), FirstAttachmentName.c_str());
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
				if (std::ranges::find_if(FirstPass->second.Attachments, [&](const auto& Element)
				{
					return Element.Name == FirstAttachmentName;
				}) == FirstPass->second.Attachments.end())
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': pass '%s' does not contain attachmet '%s'",
					                 Link.first.c_str(), Link.second.c_str(), FirstPassName.c_str(),
					                 FirstAttachmentName.c_str());
					return false;
				}
			}

			// Checking the second resource name (link 'to')
			if (SecondPassName == "$")
			{
				// If the second pass is the external pass (pass that contains external resources)
				// then check if the attachment name is equal to BLIT_TO_SWAPCHAIN as it is the
				// only resource that can be pointed to by some render pass
				if (SecondAttachmentName != "BLIT_TO_SWAPCHAIN")
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': only BLIT_TO_SWAPCHAIN is allowed as attachment name for the external render pass",
					                 Link.first.c_str(), Link.second.c_str());
					return false;
				}
			}
			else
			{
				// Otherwise, check if the render pass and the attachment with such name exists
				auto SecondPass = Passes.find(SecondPassName);
				if (SecondPass == Passes.end())
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': pass '%s' does not exist", Link.first.c_str(),
					                 Link.second.c_str(), SecondPassName.c_str());
					return false;
				}
				if (std::ranges::find_if(SecondPass->second.Attachments, [&](const auto& Element)
				{
					return Element.Name == SecondAttachmentName;
				}) == SecondPass->second.Attachments.end())
				{
					HERMES_LOG_ERROR("Ill-formed link from '%s' to '%s': pass '%s' does not contain attachmet '%s'",
					                 Link.first.c_str(), Link.second.c_str(), SecondPassName.c_str(),
					                 SecondAttachmentName.c_str());
					return false;
				}
			}
		}

		// TODO : add more checks (acyclic graphs, more than one component etc.)

		return true;
	}

	void FrameGraph::BindExternalResource(const String& Name, std::shared_ptr<Vulkan::Image> Image,
	                                      std::shared_ptr<Vulkan::ImageView> View,
	                                      VkImageLayout CurrentLayout)
	{
		auto& Resource = ImageResources.at(Name);
		HERMES_ASSERT_LOG(Resource.IsExternal, "Trying to rebind non-external frame graph resource");

		Resource.Image = std::move(Image);
		Resource.View = std::move(View);
		Resource.CurrentLayout = CurrentLayout;

		// TODO : recreate render targets that were using this resource instead of all
		// TODO : recreate render targets in Execute() on flag so that we do not recreate them multiple times
		//        in the same frame if multiple resources were changed
		RecreateFramebuffers();
	}

	FrameMetrics FrameGraph::Execute(const Scene& Scene, const GeometryList& GeometryList)
	{
		HERMES_PROFILE_FUNC();
		FrameMetrics Metrics = {};

		if (FramebuffersNeedsInitialization)
		{
			RecreateFramebuffers();
			FramebuffersNeedsInitialization = false;
		}

		auto& Swapchain = Renderer::Get().GetSwapchain();

		auto SwapchainImageAcquiredFence = Renderer::Get().GetActiveDevice().CreateFence();

		bool SwapchainWasRecreated = false;
		auto SwapchainImageIndex = Swapchain.AcquireImage(UINT64_MAX, *SwapchainImageAcquiredFence,
		                                                  SwapchainWasRecreated);
		if (SwapchainWasRecreated)
		{
			RecreateResources();
			RecreateFramebuffers();
		}

		auto& GraphicsQueue = Renderer::Get().GetActiveDevice().GetQueue(VK_QUEUE_GRAPHICS_BIT);

		for (const auto& PassName : PassExecutionOrder)
		{
			HERMES_PROFILE_SCOPE("Hermes::FrameGraph::Execute per pass loop");
			const auto& Pass = Passes[PassName];

			auto& CommandBuffer = Pass.CommandBuffer;
			CommandBuffer->BeginRecording();

			std::vector<VkImageMemoryBarrier> Barriers(Pass.AttachmentLayouts.size());
			size_t BarrierIndex = 0;
			for (const auto& Attachment : Pass.AttachmentLayouts)
			{
				auto& Resource = ImageResources[Attachment.first];

				Barriers[BarrierIndex].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				Barriers[BarrierIndex].image = Resource.Image->GetImage();

				Barriers[BarrierIndex].subresourceRange = Resource.Image->GetFullSubresourceRange();
				Barriers[BarrierIndex].oldLayout = Resource.CurrentLayout;
				Barriers[BarrierIndex].newLayout = Attachment.second;

				// TODO : this is a general solution that covers all required synchronization cases, but we should rather
				// detect or require from user which types of operations would be performed and implement proper barrier
				Barriers[BarrierIndex].srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
				Barriers[BarrierIndex].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

				Resource.CurrentLayout = Attachment.second;
				BarrierIndex++;
			}

			VkPipelineStageFlags SourceStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			CommandBuffer->InsertImageMemoryBarriers({ Barriers.begin(), Barriers.end() }, SourceStages,
			                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

			CommandBuffer->BeginRenderPass(*Pass.Pass, *Pass.Framebuffer, {
				                               // const_cast because vector::data() returns const pointer by default
				                               const_cast<VkClearValue*>(Pass.ClearColors.data()),
				                               Pass.ClearColors.size()
			                               });

			std::vector<std::pair<const Vulkan::Image*, const Vulkan::ImageView*>> Attachments(Pass.Attachments.size());
			for (size_t AttachmentIndex = 0; AttachmentIndex < Attachments.size(); AttachmentIndex++)
			{
				Attachments[AttachmentIndex] = {
					Pass.Attachments[AttachmentIndex], Pass.Views[AttachmentIndex]
				};
			}

			bool ResourcesWereRecreatedTmp = ResourcesWereRecreated; // TODO : better way to fix this maybe?
			Pass.Callback(*CommandBuffer, *Pass.Pass, Attachments, Scene, GeometryList, Metrics,
			              std::move(ResourcesWereRecreatedTmp));
			ResourcesWereRecreated = false;

			CommandBuffer->EndRenderPass();
			CommandBuffer->EndRecording();

			GraphicsQueue.SubmitCommandBuffer(*CommandBuffer, {});
		}

		{
			HERMES_PROFILE_SCOPE("Hermes::FrameGraph::Execute presentation");
			auto& Queue = Renderer::Get().GetActiveDevice().GetQueue(VK_QUEUE_GRAPHICS_BIT);
			auto BlitAndPresentCommandBuffer = Queue.CreateCommandBuffer(true);

			auto& BlitToSwapchainResource = ImageResources[BlitToSwapchainResourceOwnName];

			SwapchainImageAcquiredFence->Wait(UINT64_MAX);
			if (!SwapchainImageIndex.has_value())
			{
				HERMES_LOG_ERROR("Swapchain did not return valid image index");
				// Waiting for previously submitted rendering command buffers to finish
				// execution on rendering queue
				// TODO : any more efficient way to do this?
				GraphicsQueue.WaitForIdle();
				return Metrics; // Skip presentation of current frame
			}
			const auto& SwapchainImage = Swapchain.GetImage(SwapchainImageIndex.value());

			BlitAndPresentCommandBuffer->BeginRecording();

			VkImageMemoryBarrier SourceImageToTransferSourceBarrier = {};
			SourceImageToTransferSourceBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			SourceImageToTransferSourceBarrier.image = BlitToSwapchainResource.Image->GetImage();
			SourceImageToTransferSourceBarrier.oldLayout = BlitToSwapchainResource.CurrentLayout;
			SourceImageToTransferSourceBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			SourceImageToTransferSourceBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
			SourceImageToTransferSourceBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			SourceImageToTransferSourceBarrier.subresourceRange = BlitToSwapchainResource.Image->GetFullSubresourceRange();
			BlitAndPresentCommandBuffer->InsertImageMemoryBarrier(SourceImageToTransferSourceBarrier,
			                                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			                                                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
			                                                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			                                                      VK_PIPELINE_STAGE_TRANSFER_BIT);
			BlitToSwapchainResource.CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			VkImageMemoryBarrier SwapchainImageToTransferDestinationBarrier = {};
			SwapchainImageToTransferDestinationBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			SwapchainImageToTransferDestinationBarrier.image = SwapchainImage.GetImage();
			SwapchainImageToTransferDestinationBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			SwapchainImageToTransferDestinationBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			SwapchainImageToTransferDestinationBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			SwapchainImageToTransferDestinationBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			SwapchainImageToTransferDestinationBarrier.subresourceRange = SwapchainImage.GetFullSubresourceRange();
			BlitAndPresentCommandBuffer->InsertImageMemoryBarrier(SwapchainImageToTransferDestinationBarrier,
			                                                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			                                                      VK_PIPELINE_STAGE_TRANSFER_BIT);

			VkImageBlit BlitRegion = {};
			BlitRegion.srcSubresource.mipLevel = 0;
			BlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			BlitRegion.srcSubresource.layerCount = 1;
			BlitRegion.srcOffsets[0] = { 0, 0, 0 };
			BlitRegion.srcOffsets[1] = {
				static_cast<int32>(BlitToSwapchainResource.Image->GetDimensions().X),
				static_cast<int32>(BlitToSwapchainResource.Image->GetDimensions().Y),
				1
			};
			BlitRegion.dstSubresource.mipLevel = 0;
			BlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			BlitRegion.dstSubresource.layerCount = 1;
			BlitRegion.dstOffsets[0] = { 0, 0, 0 };
			BlitRegion.dstOffsets[1] = {
				static_cast<int32>(SwapchainImage.GetDimensions().X),
				static_cast<int32>(SwapchainImage.GetDimensions().Y),
				1
			};

			BlitAndPresentCommandBuffer->BlitImage(*BlitToSwapchainResource.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			                                       SwapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			                                       { &BlitRegion, 1 }, VK_FILTER_LINEAR);

			VkImageMemoryBarrier SwapchainImageToReadyForPresentationBarrier = {};
			SwapchainImageToReadyForPresentationBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			SwapchainImageToReadyForPresentationBarrier.image = SwapchainImage.GetImage();
			SwapchainImageToReadyForPresentationBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			SwapchainImageToReadyForPresentationBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			SwapchainImageToReadyForPresentationBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			SwapchainImageToReadyForPresentationBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			SwapchainImageToReadyForPresentationBarrier.subresourceRange = SwapchainImage.GetFullSubresourceRange();
			BlitAndPresentCommandBuffer->InsertImageMemoryBarrier(SwapchainImageToReadyForPresentationBarrier,
			                                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
			                                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

			BlitAndPresentCommandBuffer->EndRecording();
			auto PresentationFence = Renderer::Get().GetActiveDevice().CreateFence();
			Queue.SubmitCommandBuffer(*BlitAndPresentCommandBuffer, PresentationFence.get());
			PresentationFence->Wait(UINT64_MAX);

			Swapchain.Present(SwapchainImageIndex.value(), SwapchainWasRecreated);
			if (SwapchainWasRecreated)
			{
				RecreateResources();
				RecreateFramebuffers();
			}
		}

		return Metrics;
	}

	const Vulkan::RenderPass& FrameGraph::GetRenderPassObject(const String& Name) const
	{
		HERMES_ASSERT(Passes.contains(Name));
		auto& Result = Passes.at(Name).Pass;
		HERMES_ASSERT(Result);
		return *Result;
	}

	FrameGraph::FrameGraph(FrameGraphScheme InScheme)
		: Scheme(std::move(InScheme))
	{
		const auto SwapchainDimensions = Renderer::Get().GetSwapchain().GetDimensions();

		bool ContainsExternalResources = false;
		for (const auto& Resource : Scheme.ImageResources)
		{
			ContainsExternalResources |= Resource.IsExternal;

			ImageResourceContainer Container = {};
			if (!Resource.IsExternal)
			{
				auto Image = Renderer::Get().GetActiveDevice().
				                             CreateImage(Resource.Desc.Dimensions.
				                                                  GetAbsoluteDimensions(SwapchainDimensions),
				                                         TraverseResourceUsageType(Resource.Name), Resource.Desc.Format,
				                                         Resource.Desc.MipLevels);

				Container.Image = std::move(Image);
				Container.View = Container.Image->CreateDefaultImageView();
			}

			Container.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			Container.Desc = Resource.Desc;
			Container.IsExternal = Resource.IsExternal;

			ImageResources[Resource.Name] = std::move(Container);
		}

		for (const auto& Pass : Scheme.Passes)
		{
			std::vector<std::pair<VkAttachmentDescription, Vulkan::AttachmentType>> RenderPassAttachments;
			for (const auto& Attachment : Pass.second.Attachments)
			{
				auto FullAttachmentName = Pass.first + '.' + Attachment.Name;
				bool IsUsedLater = Scheme.ForwardLinks.contains(FullAttachmentName);
				VkAttachmentDescription AttachmentDesc = {};
				AttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
				AttachmentDesc.format = TraverseAttachmentDataFormat(FullAttachmentName);
				// NOTE : because the moment of layout transition after end of the render pass
				// is not synchronized we won't use this feature and would rather perform
				// layout transition ourselves using resource barriers
				AttachmentDesc.initialLayout = AttachmentDesc.finalLayout =
					PickImageLayoutForBindingMode(Attachment.Binding);
				AttachmentDesc.loadOp = Attachment.LoadOp;
				AttachmentDesc.stencilLoadOp = Attachment.StencilLoadOp;
				if (IsUsedLater)
				{
					AttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					AttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				}
				else
				{
					AttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					AttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				}

				Vulkan::AttachmentType Type;
				switch (Attachment.Binding)
				{
				case BindingMode::ColorAttachment:
					Type = Vulkan::AttachmentType::Color;
					break;
				case BindingMode::DepthStencilAttachment:
					Type = Vulkan::AttachmentType::DepthStencil;
					break;
				case BindingMode::InputAttachment:
					Type = Vulkan::AttachmentType::Input;
					break;
				default:
					HERMES_ASSERT_LOG(false, "Unknown attachment binding mode");
					break;
				}

				RenderPassAttachments.emplace_back(AttachmentDesc, Type);
			}

			const auto& GraphicsQueue = Renderer::Get().GetActiveDevice().GetQueue(VK_QUEUE_GRAPHICS_BIT);
			PassContainer NewPassContainer = {};
			NewPassContainer.Pass = Renderer::Get().GetActiveDevice().CreateRenderPass(RenderPassAttachments);
			NewPassContainer.CommandBuffer = GraphicsQueue.CreateCommandBuffer(true);
			NewPassContainer.Callback = Pass.second.Callback;

			// TODO : clean this code up
			std::vector<const Vulkan::ImageView*> FramebufferAttachments;
			Vec2ui FramebufferDimensions;
			FramebufferAttachments.reserve(Pass.second.Attachments.size());
			NewPassContainer.ClearColors.reserve(Pass.second.Attachments.size());
			NewPassContainer.AttachmentLayouts.reserve(Pass.second.Attachments.size());
			for (const auto& Attachment : Pass.second.Attachments)
			{
				String FullResourceName = TraverseResourceName(Pass.first + "." + Attachment.Name);

				String PassName, ResourceOwnName;
				SplitResourceName(FullResourceName, PassName, ResourceOwnName);

				const auto& Resource = ImageResources[ResourceOwnName];
				FramebufferAttachments.push_back(Resource.View.get());
				if (!Resource.IsExternal)
				{
					HERMES_ASSERT(FramebufferDimensions == Vec2ui{} || FramebufferDimensions == Resource.Image->
					              GetDimensions());
					FramebufferDimensions = Resource.Image->GetDimensions();
				}

				NewPassContainer.ClearColors.push_back(Attachment.ClearColor);

				NewPassContainer.Attachments.push_back(Resource.Image.get());
				NewPassContainer.Views.push_back(Resource.View.get());
				NewPassContainer.AttachmentLayouts.emplace_back(ResourceOwnName,
				                                                PickImageLayoutForBindingMode(Attachment.Binding));
			}

			if (!ContainsExternalResources)
			{
				NewPassContainer.Framebuffer = Renderer::Get().GetActiveDevice().
				                                               CreateFramebuffer(*NewPassContainer.Pass,
					                                               FramebufferAttachments,
					                                               FramebufferDimensions);
			}
			else
			{
				FramebuffersNeedsInitialization = true;
			}

			Passes[Pass.first] = std::move(NewPassContainer);
		}

		HERMES_ASSERT_LOG(Scheme.BackwardLinks.contains("$.BLIT_TO_SWAPCHAIN"),
		                  "Render graph scheme does not contain link that points to swapchain");
		auto ResourceThatBlitsToSwapchain = TraverseResourceName("$.BLIT_TO_SWAPCHAIN");
		String DummyDollarSign;
		SplitResourceName(ResourceThatBlitsToSwapchain, DummyDollarSign, BlitToSwapchainResourceOwnName);

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

	VkImageUsageFlags FrameGraph::TraverseResourceUsageType(const String& ResourceName) const
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

			if (NextAttachmentName == "$.BLIT_TO_SWAPCHAIN")
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
			case BindingMode::InputAttachment:
				Result |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
				break;
			case BindingMode::DepthStencilAttachment:
				Result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				break;
			default:
				HERMES_ASSERT(false);
				break;
			}

			if (!Scheme.ForwardLinks.contains(NextAttachmentName))
				break;

			CurrentAttachmentName = NextAttachmentName;
		}

		return Result;
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
		const auto SwapchainDimensions = Renderer::Get().GetSwapchain().GetDimensions();
		for (auto& Resource : ImageResources)
		{
			if (Resource.second.Desc.Dimensions.IsRelative() && !Resource.second.IsExternal)
			{
				Resource.second.Image = Renderer::Get().GetActiveDevice().CreateImage(
				 Resource.second.Desc.Dimensions.GetAbsoluteDimensions(SwapchainDimensions),
				 TraverseResourceUsageType(Resource.first), Resource.second.Desc.Format,
				 Resource.second.Desc.MipLevels);
				Resource.second.View = Resource.second.Image->CreateDefaultImageView();

				Resource.second.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
		}

		ResourcesWereRecreated = true;
	}

	void FrameGraph::RecreateFramebuffers()
	{
		// TODO : only recreate render targets if their images were recreated
		for (const auto& Pass : Scheme.Passes)
		{
			std::vector<const Vulkan::ImageView*> Attachments;
			Vec2ui FramebufferDimensions = {};
			Attachments.reserve(Pass.second.Attachments.size());
			Passes[Pass.first].Attachments.clear();
			Passes[Pass.first].Views.clear();
			for (const auto& Attachment : Pass.second.Attachments)
			{
				String FullResourceName = TraverseResourceName(Pass.first + "." + Attachment.Name);

				String PassName, ResourceOwnName;
				SplitResourceName(FullResourceName, PassName, ResourceOwnName);

				const auto& Resource = ImageResources[ResourceOwnName];
				Attachments.push_back(Resource.View.get());
				if (!Resource.IsExternal)
				{
					HERMES_ASSERT(FramebufferDimensions == Vec2ui{} || FramebufferDimensions == Resource.Image->
					              GetDimensions());
					FramebufferDimensions = Resource.Image->GetDimensions();
				}
				Passes[Pass.first].Attachments.push_back(Resource.Image.get());
				Passes[Pass.first].Views.push_back(Resource.View.get());
			}
			Passes[Pass.first].Framebuffer = Renderer::Get().GetActiveDevice().CreateFramebuffer(
			 *Passes[Pass.first].Pass, Attachments, FramebufferDimensions);
		}

		ResourcesWereRecreated = true;
	}
}
