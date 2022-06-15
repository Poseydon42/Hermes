#pragma once

#include <memory>
#include <array>
#include <unordered_map>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"
#include "RenderInterface/GenericRenderInterface/CommandBuffer.h"

namespace Hermes
{
	struct PassDesc;

	class FrameGraph;

	class HERMES_API FrameGraphScheme
	{
	public:
		void AddPass(const String& Name, const PassDesc& Desc);

		void AddLink(const String& Source, const String& Drain);

		void AddResource(const String& Name, const ResourceDesc& Description);

		std::unique_ptr<FrameGraph> Compile() const;

	private:
		friend class FrameGraph;

		std::unordered_map<String, PassDesc> Passes;
		std::vector<std::pair<String, ResourceDesc>> Resources;
		std::unordered_map<String, String> DrainToSourceLinkage;
		std::unordered_map<String, String> SourceToDrainLinkage;

		// TODO : implement this!
		bool Validate() const;
	};

	class HERMES_API FrameGraph
	{
	public:
		void Execute(const Scene& Scene);

	private:
		friend class FrameGraphScheme;

		explicit FrameGraph(FrameGraphScheme InScheme);

		String TraverseResourceName(const String& FullDrainName);

		RenderInterface::ImageUsageType TraverseResourceUsageType(const String& ResourceName) const;

		RenderInterface::DataFormat TraverseDrainDataFormat(const String& DrainName) const;

		void RecreateResources();

		FrameGraphScheme Scheme;

		struct ResourceContainer
		{
			std::shared_ptr<RenderInterface::Image> Image;
			RenderInterface::ImageLayout CurrentLayout = RenderInterface::ImageLayout::Undefined;
			ResourceDesc Desc;
		};
		std::unordered_map<String, ResourceContainer> Resources;

		struct PassContainer
		{
			std::shared_ptr<RenderInterface::RenderPass> Pass;
			std::shared_ptr<RenderInterface::RenderTarget> RenderTarget;
			std::shared_ptr<RenderInterface::CommandBuffer> CommandBuffer;
			std::vector<const RenderInterface::Image*> Attachments;
			// NOTE : pair<ResourceOwnName, LayoutAtStart>
			std::vector<std::pair<String, RenderInterface::ImageLayout>> AttachmentLayouts;
			std::vector<RenderInterface::ClearColor> ClearColors;
			PassDesc::RenderPassCallbackType Callback;
		};
		std::unordered_map<String, PassContainer> Passes;
		
		String BlitToSwapchainResourceOwnName;

		bool ResourcesWereRecreated = false;
	};
}
