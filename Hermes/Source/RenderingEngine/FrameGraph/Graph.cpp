#include "Graph.h"

#include <algorithm>
#include <utility>

#include "Core/Misc/StringUtils.h"
#include "Core/Profiling.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"

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

	static RenderInterface::ImageLayout PickImageLayoutForBindingMode(BindingMode Mode)
	{
		switch (Mode)
		{
		case BindingMode::DepthStencilAttachment:
			return RenderInterface::ImageLayout::DepthStencilAttachmentOptimal;
		case BindingMode::ColorAttachment:
			return RenderInterface::ImageLayout::ColorAttachmentOptimal;
		case BindingMode::InputAttachment:
			return RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		}
		HERMES_ASSERT(false);
		return RenderInterface::ImageLayout::Undefined;
	}

	void FrameGraphScheme::AddPass(const String& Name, const PassDesc& Desc)
	{
		HERMES_ASSERT_LOG(!Passes.contains(Name), L"Trying to duplicate pass with name %s", Name.c_str());
		Passes[Name] = Desc;
	}

	void FrameGraphScheme::AddLink(const String& From, const String& To)
	{
		HERMES_ASSERT_LOG(!BackwardLinks.contains(To),
		                  L"Trying to create link from %s to %s while link from %s to %s already exists",
		                  From.c_str(), To.c_str(), BackwardLinks[To].c_str(), To.c_str());

		BackwardLinks[To] = From;
		ForwardLinks[From] = To;
	}

	void FrameGraphScheme::AddResource(const String& Name, const ResourceDesc& Description)
	{
		Resources.emplace_back(Name, Description, false);
	}

	void FrameGraphScheme::DeclareExternalResource(const String& Name, const ResourceDesc& Description)
	{
		Resources.emplace_back(Name, Description, true);
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
				HERMES_LOG_ERROR(L"Invalid render pass name %s", Pass.first.c_str());
				return false;
			}
			// Pass name cannot have '.' character as it is used as separator between pass and attachment name
			if (Pass.first.find(L'.') != String::npos)
			{
				HERMES_LOG_ERROR(L"Invalid render pass name %s", Pass.first.c_str());
				return false;
			}

			// Check every attachment of this pass
			for (const auto& Attachment : Pass.second.Attachments)
			{
				// Attachment name cannot have '.' character as it is used as separator
				// between pass and attachment name
				if (Attachment.Name.find(L'.') != String::npos)
				{
					HERMES_LOG_ERROR(L"Invalid attachment name %s in render pas %s", Attachment.Name.c_str(),
					                 Pass.first.c_str());
					return false;
				}
			}
		}
		for (const auto& Resource : Resources)
		{
			// Resource name cannot have '.' character as it is used as separator between pass and attachment name
			if (Resource.Name.find(L'.') != String::npos)
			{
				HERMES_LOG_ERROR(L"Invalid resource name %s", Resource.Name.c_str());
				return false;
			}
		}

		// Step 2: check that all links have valid start and end pass and attachment name
		for (const auto& Link : ForwardLinks)
		{
			// There must be a dot in the link from and to names
			if (Link.first.find(L'.') == String::npos || Link.second.find(L'.') == String::npos)
			{
				HERMES_LOG_ERROR(L"Ill-formed link from '%s' to '%s'", Link.first.c_str(), Link.second.c_str());
				return false;
			}

			String FirstPassName, FirstAttachmentName, SecondPassName, SecondAttachmentName;
			SplitResourceName(Link.first, FirstPassName, FirstAttachmentName);
			SplitResourceName(Link.second, SecondPassName, SecondAttachmentName);

			// Checking the first resource name (link 'from')
			if (FirstPassName == L"$")
			{
				// If the first pass is the pass that contains external resources then check if
				// such resource exists
				if (std::ranges::find_if(Resources, [&](const auto& Element)
				{
					return Element.Name == FirstAttachmentName;
				}) == Resources.end())
				{
					HERMES_LOG_ERROR(L"Ill-formed link from '%s' to '%s': resource '%s' does not exist",
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
					HERMES_LOG_ERROR(L"Ill-formed link from '%s' to '%s': pass '%s' does not exist", Link.first.c_str(),
					                 Link.second.c_str(), FirstPassName.c_str());
					return false;
				}
				if (std::ranges::find_if(FirstPass->second.Attachments, [&](const auto& Element)
				{
					return Element.Name == FirstAttachmentName;
				}) == FirstPass->second.Attachments.end())
				{
					HERMES_LOG_ERROR(L"Ill-formed link from '%s' to '%s': pass '%s' does not contain attachmet '%s'",
					                 Link.first.c_str(), Link.second.c_str(), FirstPassName.c_str(),
					                 FirstAttachmentName.c_str());
					return false;
				}
			}

			// Checking the second resource name (link 'to')
			if (SecondPassName == L"$")
			{
				// If the second pass is the external pass (pass that contains external resources)
				// then check if the attachment name is equal to BLIT_TO_SWAPCHAIN as it is the
				// only resource that can be pointed to by some render pass
				if (SecondAttachmentName != L"BLIT_TO_SWAPCHAIN")
				{
					HERMES_LOG_ERROR(L"Ill-formed link from '%s' to '%s': only BLIT_TO_SWAPCHAIN is allowed as attachment name for the external render pass",
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
					HERMES_LOG_ERROR(L"Ill-formed link from '%s' to '%s': pass '%s' does not exist", Link.first.c_str(),
					                 Link.second.c_str(), SecondPassName.c_str());
					return false;
				}
				if (std::ranges::find_if(SecondPass->second.Attachments, [&](const auto& Element)
				{
					return Element.Name == SecondAttachmentName;
				}) == SecondPass->second.Attachments.end())
				{
					HERMES_LOG_ERROR(L"Ill-formed link from '%s' to '%s': pass '%s' does not contain attachmet '%s'",
					                 Link.first.c_str(), Link.second.c_str(), SecondPassName.c_str(),
					                 SecondAttachmentName.c_str());
					return false;
				}
			}
		}

		// TODO : add more checks (acyclic graphs, more than one component etc.)

		return true;
	}

	void FrameGraph::BindExternalResource(const String& Name, std::shared_ptr<RenderInterface::Image> Image,
	                                      std::shared_ptr<RenderInterface::ImageView> View,
	                                      RenderInterface::ImageLayout CurrentLayout)
	{
		auto& Resource = Resources.at(Name);
		HERMES_ASSERT_LOG(Resource.IsExternal, L"Trying to rebind non-external frame graph resource");

		Resource.Image  = std::move(Image);
		Resource.View = std::move(View);
		Resource.CurrentLayout = CurrentLayout;

		// TODO : recreate render targets that were using this resource instead of all
		// TODO : recreate render targets in Execute() on flag so that we do not recreate them multiple times
		//        in the same frame if multiple resources were changed
		RecreateRenderTargets();
	}

	FrameMetrics FrameGraph::Execute(const Scene& Scene, const GeometryList& GeometryList)
	{
		HERMES_PROFILE_FUNC();
		FrameMetrics Metrics = {};

		if (RenderTargetsNeedsInitialization)
		{
			RecreateRenderTargets();
			RenderTargetsNeedsInitialization = false;
		}

		auto& Swapchain = Renderer::Get().GetSwapchain();

		auto SwapchainImageAcquiredFence = Renderer::Get().GetActiveDevice().CreateFence();

		bool SwapchainWasRecreated = false;
		auto SwapchainImageIndex = Swapchain.AcquireImage(UINT64_MAX, *SwapchainImageAcquiredFence, SwapchainWasRecreated);
		if (SwapchainWasRecreated)
		{
			RecreateResources();
			RecreateRenderTargets();
		}

		auto& RenderQueue = Renderer::Get().GetActiveDevice().GetQueue(RenderInterface::QueueType::Render);
		
		for (const auto& PassName : PassExecutionOrder)
		{
			HERMES_PROFILE_SCOPE("Hermes::FrameGraph::Execute per pass loop");
			const auto& Pass = Passes[PassName];

			auto& CommandBuffer = Pass.CommandBuffer;
			CommandBuffer->BeginRecording();

			for (const auto& Attachment : Pass.AttachmentLayouts)
			{
				auto& Resource = Resources[Attachment.first];

				RenderInterface::ImageMemoryBarrier Barrier = {};
				Barrier.BaseMipLevel = 0;
				Barrier.MipLevelCount = Resource.Image->GetMipLevelsCount();
				Barrier.OldLayout = Resource.CurrentLayout;
				Barrier.NewLayout = Attachment.second;

				// TODO : this is a general solution that covers all required synchronization cases, but we should rather
				// detect or require from user which types of operations would be performed and implement proper barrier
				Barrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::MemoryWrite;
				Barrier.OperationsThatCanStartAfter = RenderInterface::AccessType::MemoryRead | RenderInterface::AccessType::MemoryWrite;

				// TODO: see above
				auto SourceStages = RenderInterface::PipelineStage::ColorAttachmentOutput |
					RenderInterface::PipelineStage::EarlyFragmentTests |
					RenderInterface::PipelineStage::LateFragmentTests;

				CommandBuffer->InsertImageMemoryBarrier(*Resource.Image, Barrier, SourceStages,
				                                        RenderInterface::PipelineStage::TopOfPipe);

				Resource.CurrentLayout = Attachment.second;
			}

			CommandBuffer->BeginRenderPass(*Pass.Pass, *Pass.RenderTarget, Pass.ClearColors);

			std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>> Attachments(Pass.Attachments.size());
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

			RenderQueue.SubmitCommandBuffer(*CommandBuffer, {});
		}

		{
			HERMES_PROFILE_SCOPE("Hermes::FrameGraph::Execute presentation");
			auto& PresentQueue = Renderer::Get().GetActiveDevice().GetQueue(RenderInterface::QueueType::Presentation);
			auto BlitAndPresentCommandBuffer = PresentQueue.CreateCommandBuffer(true);

			auto& BlitToSwapchainResource = Resources[BlitToSwapchainResourceOwnName];

			SwapchainImageAcquiredFence->Wait(UINT64_MAX);
			if (!SwapchainImageIndex.has_value())
			{
				HERMES_LOG_ERROR(L"Swapchain did not return valid image index");
				// Waiting for previously submitted rendering command buffers to finish
				// execution on rendering queue
				// TODO : any more efficient way to do this?
				RenderQueue.WaitForIdle();
				return Metrics; // Skip presentation of current frame
			}
			const auto SwapchainImage = Swapchain.GetImage(SwapchainImageIndex.value());

			BlitAndPresentCommandBuffer->BeginRecording();
			
			RenderInterface::ImageMemoryBarrier SourceImageToTransferSourceBarrier = {};
			SourceImageToTransferSourceBarrier.BaseMipLevel = 0;
			SourceImageToTransferSourceBarrier.MipLevelCount = 1;
			SourceImageToTransferSourceBarrier.OldLayout = BlitToSwapchainResource.CurrentLayout;
			SourceImageToTransferSourceBarrier.NewLayout = RenderInterface::ImageLayout::TransferSourceOptimal;
			SourceImageToTransferSourceBarrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::MemoryWrite;
			SourceImageToTransferSourceBarrier.OperationsThatCanStartAfter = RenderInterface::AccessType::TransferRead;
			BlitAndPresentCommandBuffer->InsertImageMemoryBarrier(*BlitToSwapchainResource.Image,
			                                                      SourceImageToTransferSourceBarrier,
			                                                      RenderInterface::PipelineStage::ColorAttachmentOutput
			                                                      | RenderInterface::PipelineStage::EarlyFragmentTests |
			                                                      RenderInterface::PipelineStage::LateFragmentTests,
			                                                      RenderInterface::PipelineStage::Transfer);
			BlitToSwapchainResource.CurrentLayout = RenderInterface::ImageLayout::TransferSourceOptimal;
			
			RenderInterface::ImageMemoryBarrier SwapchainImageToTransferDestinationBarrier = {};
			SwapchainImageToTransferDestinationBarrier.BaseMipLevel = 0;
			SwapchainImageToTransferDestinationBarrier.MipLevelCount = 1;
			SwapchainImageToTransferDestinationBarrier.OldLayout = RenderInterface::ImageLayout::Undefined;
			SwapchainImageToTransferDestinationBarrier.NewLayout = RenderInterface::ImageLayout::TransferDestinationOptimal;
			SwapchainImageToTransferDestinationBarrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::MemoryRead;
			SwapchainImageToTransferDestinationBarrier.OperationsThatCanStartAfter = RenderInterface::AccessType::TransferWrite;
			BlitAndPresentCommandBuffer->InsertImageMemoryBarrier(
				*SwapchainImage, SwapchainImageToTransferDestinationBarrier,
				RenderInterface::PipelineStage::BottomOfPipe, RenderInterface::PipelineStage::Transfer);

			RenderInterface::ImageBlitRegion BlitRegion = {};
			BlitRegion.SourceRegion.MipLevel = 0;
			BlitRegion.SourceRegion.AspectMask = RenderInterface::ImageAspect::Color;
			BlitRegion.SourceRegion.RectMin = { 0, 0 };
			BlitRegion.SourceRegion.RectMax = BlitToSwapchainResource.Image->GetSize();
			BlitRegion.DestinationRegion.MipLevel = 0;
			BlitRegion.DestinationRegion.AspectMask = RenderInterface::ImageAspect::Color;
			BlitRegion.DestinationRegion.RectMin = { 0, 0 };
			BlitRegion.DestinationRegion.RectMax = SwapchainImage->GetSize();

			BlitAndPresentCommandBuffer->BlitImage(
				*BlitToSwapchainResource.Image, RenderInterface::ImageLayout::TransferSourceOptimal,
				*SwapchainImage, RenderInterface::ImageLayout::TransferDestinationOptimal,
				{ BlitRegion }, RenderInterface::FilteringMode::Linear);

			RenderInterface::ImageMemoryBarrier SwapchainImageToReadyForPresentationBarrier = {};
			SwapchainImageToReadyForPresentationBarrier.BaseMipLevel = 0;
			SwapchainImageToReadyForPresentationBarrier.MipLevelCount = 1;
			SwapchainImageToReadyForPresentationBarrier.OldLayout = RenderInterface::ImageLayout::TransferDestinationOptimal;
			SwapchainImageToReadyForPresentationBarrier.NewLayout = RenderInterface::ImageLayout::ReadyForPresentation;
			SwapchainImageToReadyForPresentationBarrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::TransferWrite;
			SwapchainImageToReadyForPresentationBarrier.OperationsThatCanStartAfter = RenderInterface::AccessType::MemoryRead;
			BlitAndPresentCommandBuffer->InsertImageMemoryBarrier(
				*SwapchainImage, SwapchainImageToReadyForPresentationBarrier,
				RenderInterface::PipelineStage::Transfer, RenderInterface::PipelineStage::TopOfPipe);

			BlitAndPresentCommandBuffer->EndRecording();
			auto PresentationFence = Renderer::Get().GetActiveDevice().CreateFence();
			PresentQueue.SubmitCommandBuffer(*BlitAndPresentCommandBuffer, PresentationFence.get());
			PresentationFence->Wait(UINT64_MAX);

			Swapchain.Present(SwapchainImageIndex.value(), SwapchainWasRecreated);
			if (SwapchainWasRecreated)
			{
				RecreateResources();
				RecreateRenderTargets();
			}
		}

		return Metrics;
	}

	const RenderInterface::RenderPass& FrameGraph::GetRenderPassObject(const String& Name) const
	{
		HERMES_ASSERT(Passes.contains(Name));
		auto& Result = Passes.at(Name).Pass;
		HERMES_ASSERT(Result);
		return *Result;
	}

	FrameGraph::FrameGraph(FrameGraphScheme InScheme)
		: Scheme(std::move(InScheme))
	{
		const auto SwapchainDimensions = Renderer::Get().GetSwapchain().GetSize();

		bool ContainsExternalResources = false;
		for (const auto& Resource : Scheme.Resources)
		{
			ContainsExternalResources |= Resource.IsExternal;

			ResourceContainer Container = {};
			if (!Resource.IsExternal)
			{
				auto Image = Renderer::Get().GetActiveDevice().CreateImage(
					Resource.Desc.Dimensions.GetAbsoluteDimensions(SwapchainDimensions),
					TraverseResourceUsageType(Resource.Name), Resource.Desc.Format,
					Resource.Desc.MipLevels, RenderInterface::ImageLayout::Undefined);
				
				Container.Image = std::move(Image);
				Container.View = Container.Image->CreateDefaultImageView();
			}
			
			Container.CurrentLayout = RenderInterface::ImageLayout::Undefined;
			Container.Desc = Resource.Desc;
			Container.IsExternal = Resource.IsExternal;

			Resources[Resource.Name] = std::move(Container);
		}

		for (const auto& Pass : Scheme.Passes)
		{
			std::vector<RenderInterface::RenderPassAttachment> RenderPassAttachments;
			for (const auto& Attachment : Pass.second.Attachments)
			{
				auto FullAttachmentName = Pass.first + L'.' + Attachment.Name;
				bool IsUsedLater = Scheme.ForwardLinks.contains(FullAttachmentName);
				RenderInterface::RenderPassAttachment AttachmentDesc = {};
				AttachmentDesc.Format = TraverseAttachmentDataFormat(FullAttachmentName);
				// NOTE : because the moment of layout transition after end of the render pass
				// is not synchronized we won't use this feature and would rather perform
				// layout transition ourselves using resource barriers
				AttachmentDesc.LayoutAtStart = AttachmentDesc.LayoutAtEnd =
					PickImageLayoutForBindingMode(Attachment.Binding);
				AttachmentDesc.LoadOp = Attachment.LoadOp;
				AttachmentDesc.StencilLoadOp = Attachment.StencilLoadOp;
				if (IsUsedLater)
				{
					AttachmentDesc.StoreOp = RenderInterface::AttachmentStoreOp::Store;
					AttachmentDesc.StencilStoreOp = RenderInterface::AttachmentStoreOp::Store;
				}
				else
				{
					AttachmentDesc.StoreOp = RenderInterface::AttachmentStoreOp::Undefined;
					AttachmentDesc.StencilStoreOp = RenderInterface::AttachmentStoreOp::Undefined;
				}
				switch (Attachment.Binding)
				{
				case BindingMode::ColorAttachment:
					AttachmentDesc.Type = RenderInterface::AttachmentType::Color;
					break;
				case BindingMode::DepthStencilAttachment:
					AttachmentDesc.Type = RenderInterface::AttachmentType::DepthStencil;
					break;
				case BindingMode::InputAttachment:
					AttachmentDesc.Type = RenderInterface::AttachmentType::Input;
					break;
				default:
					HERMES_ASSERT_LOG(false, L"Unknown attachment binding mode");
					break;
				}

				RenderPassAttachments.push_back(AttachmentDesc);
			}

			const auto& RenderQueue = Renderer::Get().GetActiveDevice().GetQueue(RenderInterface::QueueType::Render);
			PassContainer NewPassContainer = {};
			NewPassContainer.Pass = Renderer::Get().GetActiveDevice().CreateRenderPass(RenderPassAttachments);
			NewPassContainer.CommandBuffer = RenderQueue.CreateCommandBuffer(true);
			NewPassContainer.Callback = Pass.second.Callback;

			// TODO : clean this code up
			std::vector<const RenderInterface::ImageView*> RenderTargetAttachments;
			Vec2ui RenderTargetDimensions;
			RenderTargetAttachments.reserve(Pass.second.Attachments.size());
			NewPassContainer.ClearColors.reserve(Pass.second.Attachments.size());
			NewPassContainer.AttachmentLayouts.reserve(Pass.second.Attachments.size());
			for (const auto& Attachment : Pass.second.Attachments)
			{
				String FullResourceName = TraverseResourceName(Pass.first + L"." + Attachment.Name);

				String PassName, ResourceOwnName;
				SplitResourceName(FullResourceName, PassName, ResourceOwnName);

				const auto& Resource = Resources[ResourceOwnName];
				RenderTargetAttachments.push_back(Resource.View.get());
				if (!Resource.IsExternal)
				{
					HERMES_ASSERT(RenderTargetDimensions == Vec2ui{} || RenderTargetDimensions == Resource.Image->GetSize());
					RenderTargetDimensions = Resource.Image->GetSize();
				}

				NewPassContainer.ClearColors.push_back(Attachment.ClearColor);

				NewPassContainer.Attachments.push_back(Resource.Image.get());
				NewPassContainer.Views.push_back(Resource.View.get());
				NewPassContainer.AttachmentLayouts.emplace_back(ResourceOwnName, PickImageLayoutForBindingMode(Attachment.Binding));
			}

			if (!ContainsExternalResources)
			{
				NewPassContainer.RenderTarget = Renderer::Get().GetActiveDevice().CreateRenderTarget(
					*NewPassContainer.Pass, RenderTargetAttachments, RenderTargetDimensions);
			}
			else
			{
				RenderTargetsNeedsInitialization = true;
			}

			Passes[Pass.first] = std::move(NewPassContainer);
		}

		HERMES_ASSERT_LOG(Scheme.BackwardLinks.contains(L"$.BLIT_TO_SWAPCHAIN"),
		                  L"Render graph scheme does not contain link that points to swapchain");
		auto ResourceThatBlitsToSwapchain = TraverseResourceName(L"$.BLIT_TO_SWAPCHAIN");
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
			if (Link->first.starts_with(L"$.") || Link->second.starts_with(L"$."))
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
					if (Link.first.starts_with(CurrentPass->first + L"."))
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
						if (Link->second.starts_with(CurrentPass->first + L"."))
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

	RenderInterface::ImageUsageType FrameGraph::TraverseResourceUsageType(const String& ResourceName) const
	{
		auto Result = static_cast<RenderInterface::ImageUsageType>(0);
		String CurrentAttachmentName = L"$." + ResourceName;
		while (true)
		{
			if (!Scheme.ForwardLinks.contains(CurrentAttachmentName))
				break;
			auto NextAttachmentName = Scheme.ForwardLinks.at(CurrentAttachmentName);
			String CurrentAttachmentRenderPassName, CurrentAttachmentOwnName;
			SplitResourceName(NextAttachmentName, CurrentAttachmentRenderPassName, CurrentAttachmentOwnName);

			if (NextAttachmentName == L"$.BLIT_TO_SWAPCHAIN")
			{
				Result |= RenderInterface::ImageUsageType::CopySource;
				break;
			}

			auto CurrentAttachmentRenderPass = Scheme.Passes.at(CurrentAttachmentRenderPassName);
			auto CurrentAttachment = std::ranges::find_if(CurrentAttachmentRenderPass.Attachments, [&](const Attachment& Element)
			{
				return Element.Name == CurrentAttachmentOwnName;
			});

			switch (CurrentAttachment->Binding)
			{
			case BindingMode::ColorAttachment:
				Result |= RenderInterface::ImageUsageType::ColorAttachment;
				break;
			case BindingMode::InputAttachment:
				Result |= RenderInterface::ImageUsageType::InputAttachment;
				break;
			case BindingMode::DepthStencilAttachment:
				Result |= RenderInterface::ImageUsageType::DepthStencilAttachment;
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

	RenderInterface::DataFormat FrameGraph::TraverseAttachmentDataFormat(const String& AttachmentName) const
	{
		auto CurrentAttachment = Scheme.BackwardLinks.at(AttachmentName);
		while (CurrentAttachment[0] != L'$')
		{
			CurrentAttachment = Scheme.BackwardLinks.at(CurrentAttachment);
		}

		String ResourcePassName, ResourceOwnName;
		SplitResourceName(CurrentAttachment, ResourcePassName, ResourceOwnName);
		HERMES_ASSERT(ResourcePassName == L"$");

		const auto& Resource = Resources.at(ResourceOwnName);
		return Resource.Desc.Format;
	}

	void FrameGraph::RecreateResources()
	{
		const auto SwapchainDimensions = Renderer::Get().GetSwapchain().GetSize();
		for (auto& Resource : Resources)
		{
			if (Resource.second.Desc.Dimensions.IsRelative() && !Resource.second.IsExternal)
			{
				Resource.second.Image = Renderer::Get().GetActiveDevice().CreateImage(
					Resource.second.Desc.Dimensions.GetAbsoluteDimensions(SwapchainDimensions),
					TraverseResourceUsageType(Resource.first), Resource.second.Desc.Format,
					Resource.second.Desc.MipLevels, RenderInterface::ImageLayout::Undefined);
				Resource.second.View = Resource.second.Image->CreateDefaultImageView();

				Resource.second.CurrentLayout = RenderInterface::ImageLayout::Undefined;
			}
		}

		ResourcesWereRecreated = true;
	}

	void FrameGraph::RecreateRenderTargets()
	{
		// TODO : only recreate render targets if their images were recreated
		for (const auto& Pass : Scheme.Passes)
		{
			std::vector<const RenderInterface::ImageView*> Attachments;
			Vec2ui RenderTargetDimensions = {};
			Attachments.reserve(Pass.second.Attachments.size());
			Passes[Pass.first].Attachments.clear();
			Passes[Pass.first].Views.clear();
			for (const auto& Attachment : Pass.second.Attachments)
			{
				String FullResourceName = TraverseResourceName(Pass.first + L"." + Attachment.Name);

				String PassName, ResourceOwnName;
				SplitResourceName(FullResourceName, PassName, ResourceOwnName);

				const auto& Resource = Resources[ResourceOwnName];
				Attachments.push_back(Resource.View.get());
				if (!Resource.IsExternal)
				{
					HERMES_ASSERT(RenderTargetDimensions == Vec2ui{} || RenderTargetDimensions == Resource.Image->GetSize());
					RenderTargetDimensions = Resource.Image->GetSize();
				}
				Passes[Pass.first].Attachments.push_back(Resource.Image.get());
				Passes[Pass.first].Views.push_back(Resource.View.get());
			}
			Passes[Pass.first].RenderTarget = Renderer::Get().GetActiveDevice().CreateRenderTarget(
				*Passes[Pass.first].Pass, Attachments, RenderTargetDimensions);
		}

		ResourcesWereRecreated = true;
	}
}
