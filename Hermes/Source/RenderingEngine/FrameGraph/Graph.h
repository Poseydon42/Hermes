#pragma once

#include <memory>
#include <unordered_map>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderInterface/GenericRenderInterface/RenderTarget.h"

namespace Hermes
{
	class FrameGraph;
	class GeometryList;

	class HERMES_API FrameGraphScheme
	{
	public:
		void AddPass(const String& Name, const PassDesc& Desc);

		void AddLink(const String& From, const String& To);

		void AddResource(const String& Name, const ResourceDesc& Description);

		void DeclareExternalResource(const String& Name, const ResourceDesc& Description);

		std::unique_ptr<FrameGraph> Compile() const;

	private:
		friend class FrameGraph;

		std::unordered_map<String, PassDesc> Passes;

		struct ResourceContainer
		{
			String Name;
			ResourceDesc Desc;

			bool IsExternal = false;
		};
		std::vector<ResourceContainer> Resources;

		// NOTE : ForwardLinks holds links between outputs of first render pass and inputs of the second
		//        BackwardLinks is just a reversed version of it because STL does not have bidirectional map
		std::unordered_map<String, String> ForwardLinks;
		std::unordered_map<String, String> BackwardLinks;

		bool Validate() const;
	};

	struct FrameMetrics
	{
		uint32 DrawCallCount = 0;
		uint32 PipelineBindCount = 0;
		uint32 BufferBindCount = 0;
		uint32 DescriptorSetBindCount = 0;
	};

	class HERMES_API FrameGraph
	{
		MAKE_NON_COPYABLE(FrameGraph)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(FrameGraph)
		ADD_DEFAULT_DESTRUCTOR(FrameGraph)
	public:
		void BindExternalResource(const String& Name, std::shared_ptr<RenderInterface::Image> Image,
		                          std::shared_ptr<RenderInterface::ImageView> View,
		                          RenderInterface::ImageLayout CurrentLayout);

		FrameMetrics Execute(const Scene& Scene, const GeometryList& GeometryList);

		const RenderInterface::RenderPass& GetRenderPassObject(const String& Name) const;

	private:
		friend class FrameGraphScheme;

		explicit FrameGraph(FrameGraphScheme InScheme);

		String TraverseResourceName(const String& FullAttachmentName);

		RenderInterface::ImageUsageType TraverseResourceUsageType(const String& ResourceName) const;

		RenderInterface::DataFormat TraverseAttachmentDataFormat(const String& AttachmentName) const;

		void RecreateResources();
		void RecreateRenderTargets();

		FrameGraphScheme Scheme;

		struct ResourceContainer
		{
			std::shared_ptr<RenderInterface::Image> Image;
			std::shared_ptr<RenderInterface::ImageView> View;
			RenderInterface::ImageLayout CurrentLayout = RenderInterface::ImageLayout::Undefined;
			ResourceDesc Desc;

			bool IsExternal = false;
		};
		std::unordered_map<String, ResourceContainer> Resources;

		struct PassContainer
		{
			std::unique_ptr<RenderInterface::RenderPass> Pass;
			std::unique_ptr<RenderInterface::RenderTarget> RenderTarget;
			std::unique_ptr<RenderInterface::CommandBuffer> CommandBuffer;
			std::vector<const RenderInterface::Image*> Attachments;
			std::vector<const RenderInterface::ImageView*> Views;
			// NOTE : pair<ResourceOwnName, LayoutAtStart>
			std::vector<std::pair<String, RenderInterface::ImageLayout>> AttachmentLayouts;
			std::vector<RenderInterface::ClearColor> ClearColors;
			PassDesc::RenderPassCallbackType Callback;
		};
		std::unordered_map<String, PassContainer> Passes;

		std::vector<String> PassExecutionOrder;
		
		String BlitToSwapchainResourceOwnName;

		bool ResourcesWereRecreated = false;
		bool RenderTargetsNeedsInitialization = false;
	};
}
