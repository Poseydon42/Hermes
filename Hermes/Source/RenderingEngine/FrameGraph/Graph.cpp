#include "Graph.h"

#include <algorithm>
#include <utility>

#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Image.h"

namespace Hermes
{
	static void SplitResourceName(const String& ResourceName, String& PassName, String& OwnName)
	{
		auto DotIndex = ResourceName.find_first_of('.');
		HERMES_ASSERT(DotIndex != ResourceName.npos);
		PassName = String(ResourceName.begin(), ResourceName.begin() + DotIndex);
		OwnName = String(ResourceName.begin() + DotIndex + 1, ResourceName.end());
	}

	void FrameGraphScheme::AddPass(const String& Name, const PassDesc& Desc)
	{
		HERMES_ASSERT_LOG(Passes.count(Name) == 0, L"Trying to duplicate pass with name %s", Name.c_str());
		Passes[Name] = Desc;
	}

	void FrameGraphScheme::AddLink(const String& Source, const String& Drain)
	{
		HERMES_ASSERT_LOG(
			DrainToSourceLinkage.count(Drain) == 0,
			L"Trying to link drain %s to source %s while it is already linked with source %s",
			Drain.c_str(), Source.c_str(), DrainToSourceLinkage[Drain].c_str());

		DrainToSourceLinkage[Drain] = Source;
		SourceToDrainLinkage[Source] = Drain;
	}

	void FrameGraphScheme::AddResource(const String& Name, const ResourceDesc& Description)
	{
		Resources.emplace_back(Name, Description);
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
		return true;
	}

	void FrameGraph::Execute(const Scene& Scene)
	{
		auto& Swapchain = Renderer::Get().GetSwapchain();

		auto SwapchainImageAcquiredFence = Renderer::Get().GetActiveDevice().CreateFence();

		bool SwapchainWasRecreated = false;
		auto SwapchainImageIndex = Swapchain.AcquireImage(UINT64_MAX, *SwapchainImageAcquiredFence, SwapchainWasRecreated);
		if (SwapchainWasRecreated)
			RecreateResources();

		auto& RenderQueue = Renderer::Get().GetActiveDevice().GetQueue(RenderInterface::QueueType::Render);
		
		for (const auto& Pass : Passes)
		{
			auto& CommandBuffer = Pass.second.CommandBuffer;
			CommandBuffer->BeginRecording();

			for (const auto& Attachment : Pass.second.AttachmentLayouts)
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

				CommandBuffer->InsertImageMemoryBarrier(
					*Resource.Image, Barrier,
					RenderInterface::PipelineStage::BottomOfPipe,
					RenderInterface::PipelineStage::TopOfPipe);

				Resource.CurrentLayout = Attachment.second;
			}

			CommandBuffer->BeginRenderPass(Pass.second.Pass, Pass.second.RenderTarget, Pass.second.ClearColors);

			bool ResourcesWereRecreatedTmp = ResourcesWereRecreated; // TODO : better way to fix this maybe?
			Pass.second.Callback(*CommandBuffer, *Pass.second.Pass, Pass.second.Attachments, Scene, std::move(ResourcesWereRecreatedTmp));
			ResourcesWereRecreated = false;

			CommandBuffer->EndRenderPass();
			CommandBuffer->EndRecording();
			
			RenderQueue.SubmitCommandBuffer(CommandBuffer, {});
		}

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
			return; // Skip presentation of current frame
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
		BlitAndPresentCommandBuffer->InsertImageMemoryBarrier(
			*BlitToSwapchainResource.Image, SourceImageToTransferSourceBarrier, 
			RenderInterface::PipelineStage::BottomOfPipe, RenderInterface::PipelineStage::Transfer);
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
		PresentQueue.SubmitCommandBuffer(BlitAndPresentCommandBuffer, PresentationFence);
		PresentationFence->Wait(UINT64_MAX);

		Swapchain.Present(SwapchainImageIndex.value(), SwapchainWasRecreated);
		if (SwapchainWasRecreated)
			RecreateResources();
	}

	FrameGraph::FrameGraph(FrameGraphScheme InScheme)
		: Scheme(std::move(InScheme))
	{
		const auto SwapchainDimensions = Renderer::Get().GetSwapchain().GetSize();
		for (const auto& Resource : Scheme.Resources)
		{
			auto Image = Renderer::Get().GetActiveDevice().CreateImage(
				Resource.second.Dimensions.GetAbsoluteDimensions(SwapchainDimensions),
				TraverseResourceUsageType(Resource.first), Resource.second.Format,
				Resource.second.MipLevels, RenderInterface::ImageLayout::Undefined);

			ResourceContainer Container = {};
			Container.Image = std::move(Image);
			Container.CurrentLayout = RenderInterface::ImageLayout::Undefined;
			Container.Desc = Resource.second;

			Resources[Resource.first] = Container;
		}

		for (const auto& Pass : Scheme.Passes)
		{
			std::vector<RenderInterface::RenderPassAttachment> RenderPassAttachments;
			for (const auto& Drain : Pass.second.Drains)
			{
				const auto& CorrespondingSourceIterator = std::find_if(
					Pass.second.Sources.begin(), Pass.second.Sources.end(), [&](const Source& Element)
					{
						return Element.Name == Drain.Name;
					});
				RenderInterface::RenderPassAttachment AttachmentDesc = {};
				auto FullDrainName = Pass.first + L'.' + Drain.Name;
				AttachmentDesc.Format = TraverseDrainDataFormat(FullDrainName);
				// NOTE : because the moment of layout transition after end of the render pass
				// is not synchronized we won't use this feature and would rather perform
				// layout transition ourselves using resource barriers
				if (Drain.Binding == BindingMode::InputAttachment)
				{
					AttachmentDesc.LayoutAtStart = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
					AttachmentDesc.LayoutAtEnd = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
				}
				else
				{
					AttachmentDesc.LayoutAtEnd = AttachmentDesc.LayoutAtStart = Drain.Layout;
				}
				AttachmentDesc.LoadOp = Drain.LoadOp;
				AttachmentDesc.StencilLoadOp = Drain.StencilLoadOp;
				if (CorrespondingSourceIterator != Pass.second.Sources.end())
				{
					AttachmentDesc.StoreOp = RenderInterface::AttachmentStoreOp::Store;
					AttachmentDesc.StencilStoreOp = RenderInterface::AttachmentStoreOp::Store;
				}
				else
				{
					AttachmentDesc.StoreOp = RenderInterface::AttachmentStoreOp::Undefined;
					AttachmentDesc.StencilStoreOp = RenderInterface::AttachmentStoreOp::Undefined;
				}
				switch (Drain.Binding)
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
					HERMES_ASSERT_LOG(false, L"Unknown drain binding type");
					break;
				}

				RenderPassAttachments.push_back(AttachmentDesc);
			}

			for (const auto& Source : Pass.second.Sources)
			{
				if (Scheme.SourceToDrainLinkage[Pass.first + L"." + Source.Name] == L"$.BLIT_TO_SWAPCHAIN")
				{
					HERMES_ASSERT(BlitToSwapchainResourceOwnName.empty());
					String ResourceFullName = TraverseResourceName(Pass.first + L"." + Source.Name);
					String PassName;
					SplitResourceName(ResourceFullName, PassName, BlitToSwapchainResourceOwnName);
				}
			}

			const auto& RenderQueue = Renderer::Get().GetActiveDevice().GetQueue(RenderInterface::QueueType::Render);
			PassContainer NewPassContainer = {};
			NewPassContainer.Pass = Renderer::Get().GetActiveDevice().CreateRenderPass(RenderPassAttachments);
			NewPassContainer.CommandBuffer = RenderQueue.CreateCommandBuffer(true);
			NewPassContainer.Callback = Pass.second.Callback;

			std::vector<std::shared_ptr<RenderInterface::Image>> RenderTargetAttachments;
			RenderTargetAttachments.reserve(Pass.second.Drains.size());
			NewPassContainer.ClearColors.reserve(Pass.second.Drains.size());
			NewPassContainer.AttachmentLayouts.reserve(Pass.second.Drains.size());
			for (const auto& Drain : Pass.second.Drains)
			{
				String FullResourceName = TraverseResourceName(Pass.first + L"." + Drain.Name);

				String PassName, ResourceOwnName;
				SplitResourceName(FullResourceName, PassName, ResourceOwnName);

				const auto& Resource = Resources[ResourceOwnName];
				RenderTargetAttachments.push_back(Resource.Image);

				NewPassContainer.ClearColors.emplace_back();
				NewPassContainer.ClearColors.back().R = Drain.ClearColor[0];
				NewPassContainer.ClearColors.back().G = Drain.ClearColor[1];
				NewPassContainer.ClearColors.back().B = Drain.ClearColor[2];
				NewPassContainer.ClearColors.back().A = Drain.ClearColor[3];

				NewPassContainer.Attachments.push_back(Resource.Image.get());
				NewPassContainer.AttachmentLayouts.emplace_back(ResourceOwnName, Drain.Layout);
			}

			NewPassContainer.RenderTarget = Renderer::Get().GetActiveDevice().CreateRenderTarget(
				NewPassContainer.Pass, RenderTargetAttachments, RenderTargetAttachments[0]->GetSize());

			Passes[Pass.first] = NewPassContainer;
		}
	}

	String FrameGraph::TraverseResourceName(const String& FullDrainName)
	{
		String CurrentSourceName = Scheme.DrainToSourceLinkage[FullDrainName];
		while (CurrentSourceName.find_first_of('$') != 0)
		{
			// Because if we traverse back then each source of some pass must have
			// its corresponding drain with same name in the same pass thus their full
			// names would be equal
			CurrentSourceName = Scheme.DrainToSourceLinkage[CurrentSourceName];
		}
		return CurrentSourceName;
	}

	RenderInterface::ImageUsageType FrameGraph::TraverseResourceUsageType(const String& ResourceName) const
	{
		auto Result = static_cast<RenderInterface::ImageUsageType>(0);
		String CurrentSourceName = L"$." + ResourceName;
		while (true)
		{
			auto CurrentDrainName = Scheme.SourceToDrainLinkage.at(CurrentSourceName);
			String CurrentDrainRenderPassName, CurrentDrainOwnName;
			SplitResourceName(CurrentDrainName, CurrentDrainRenderPassName, CurrentDrainOwnName);

			if (CurrentDrainName == L"$.BLIT_TO_SWAPCHAIN")
			{
				Result |= RenderInterface::ImageUsageType::CopySource;
				break;
			}

			auto CurrentDrainRenderPass = Scheme.Passes.at(CurrentDrainRenderPassName);
			auto CurrentDrain = std::find_if(
				CurrentDrainRenderPass.Drains.begin(), CurrentDrainRenderPass.Drains.end(),
				[&](const Drain& Element)
				{
					return Element.Name == CurrentDrainOwnName;
				});

			switch (CurrentDrain->Binding)
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

			auto CorrespondingSourceOfCurrentRenderPass = std::find_if(
				CurrentDrainRenderPass.Sources.begin(), CurrentDrainRenderPass.Sources.end(),
				[&](const Source& Element)
				{
					return Element.Name == CurrentDrainOwnName;
				});
			if (CorrespondingSourceOfCurrentRenderPass == CurrentDrainRenderPass.Sources.end())
				break;

			// NOTE : Because own name of connected drains and sources have to match
			// and they are within same render pass thus their full names would be equal
			CurrentSourceName = CurrentDrainName;
		}

		return Result;
	}

	RenderInterface::DataFormat FrameGraph::TraverseDrainDataFormat(const String& DrainName) const
	{
		auto CurrentSource = Scheme.DrainToSourceLinkage.at(DrainName);
		while (CurrentSource[0] != L'$')
		{
			// NOTE : because drain and source name within same pass have to be the same if they are interconnected
			CurrentSource = Scheme.DrainToSourceLinkage.at(CurrentSource);
		}

		String ResourcePassName, ResourceOwnName;
		SplitResourceName(CurrentSource, ResourcePassName, ResourceOwnName);
		HERMES_ASSERT(ResourcePassName == L"$");

		const auto& Resource = Resources.at(ResourceOwnName);
		return Resource.Desc.Format;
	}

	void FrameGraph::RecreateResources()
	{
		const auto SwapchainDimensions = Renderer::Get().GetSwapchain().GetSize();
		for (auto& Resource : Resources)
		{
			if (Resource.second.Desc.Dimensions.IsRelative())
			{
				Resource.second.Image = Renderer::Get().GetActiveDevice().CreateImage(
					Resource.second.Desc.Dimensions.GetAbsoluteDimensions(SwapchainDimensions),
					TraverseResourceUsageType(Resource.first), Resource.second.Desc.Format,
					Resource.second.Desc.MipLevels, RenderInterface::ImageLayout::Undefined);

				Resource.second.CurrentLayout = RenderInterface::ImageLayout::Undefined;
			}
		}

		// TODO : only recreate render targets if their images were recreated
		for (const auto& Pass : Scheme.Passes)
		{
			std::vector<std::shared_ptr<RenderInterface::Image>> Attachments;
			Attachments.reserve(Pass.second.Drains.size());
			Passes[Pass.first].Attachments.clear();
			for (const auto& Drain : Pass.second.Drains)
			{
				String FullResourceName = TraverseResourceName(Pass.first + L"." + Drain.Name);

				String PassName, ResourceOwnName;
				SplitResourceName(FullResourceName, PassName, ResourceOwnName);

				const auto& Resource = Resources[ResourceOwnName];
				Attachments.push_back(Resource.Image);
				Passes[Pass.first].Attachments.push_back(Resource.Image.get());
			}
			Passes[Pass.first].RenderTarget = Renderer::Get().GetActiveDevice().CreateRenderTarget(
				Passes[Pass.first].Pass, Attachments, Attachments[0]->GetSize());
		}

		ResourcesWereRecreated = true;
	}
}
