#pragma once

#include <memory>
#include <unordered_map>

#include "Core/Core.h"
#include "Math/Rect2D.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Image.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Framebuffer.h"

namespace Hermes
{
	class FrameGraph;
	class GeometryList;

	class HERMES_API FrameGraphScheme
	{
	public:
		void AddPass(const String& Name, const PassDesc& Desc);

		void AddLink(const String& From, const String& To);

		void AddResource(const String& Name, const ImageResourceDescription& Description);

		void AddResource(const String& Name, const BufferResourceDescription& Description);

		void DeclareExternalResource(const String& Name, const ImageResourceDescription& Description);

		std::unique_ptr<FrameGraph> Compile() const;

	private:
		friend class FrameGraph;

		std::unordered_map<String, PassDesc> Passes;

		struct ImageResourceContainer
		{
			String Name;
			ImageResourceDescription Desc;

			bool IsExternal = false;
		};
		std::vector<ImageResourceContainer> ImageResources;

		struct BufferResourceContainer
		{
			String Name;
			BufferResourceDescription Desc;
		};
		std::vector<BufferResourceContainer> BufferResources;

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
		void BindExternalResource(const String& Name, std::shared_ptr<Vulkan::Image> Image,
		                          std::shared_ptr<Vulkan::ImageView> View,
		                          VkImageLayout CurrentLayout);

		FrameMetrics Execute(const Scene& Scene, const GeometryList& GeometryList, Rect2Dui Viewport);

		const Vulkan::RenderPass& GetRenderPassObject(const String& Name) const;

	private:
		friend class FrameGraphScheme;

		explicit FrameGraph(FrameGraphScheme InScheme);

		String TraverseResourceName(const String& FullAttachmentName);

		VkImageUsageFlags TraverseImageResourceUsageType(const String& ResourceName) const;

		VkBufferUsageFlags TraverseBufferResourceUsageType(const String& ResourceName) const;

		bool TraverseCheckIfBufferIsMappable(const String& ResourceName) const;

		VkFormat TraverseAttachmentDataFormat(const String& AttachmentName) const;

		void RecreateResources();
		void RecreateFramebuffers();

		FrameGraphScheme Scheme;

		struct ImageResourceContainer
		{
			std::shared_ptr<Vulkan::Image> Image;
			std::shared_ptr<Vulkan::ImageView> View;
			VkImageLayout CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			ImageResourceDescription Desc;

			bool IsExternal = false;
		};
		std::unordered_map<String, ImageResourceContainer> ImageResources;

		struct BufferResourceContainer
		{
			std::unique_ptr<Vulkan::Buffer> Buffer;
			BufferResourceDescription Desc;
		};
		std::unordered_map<String, BufferResourceContainer> BufferResources;

		struct PassContainer
		{
			std::unique_ptr<Vulkan::RenderPass> Pass;
			std::unique_ptr<Vulkan::Framebuffer> Framebuffer;
			std::unique_ptr<Vulkan::CommandBuffer> CommandBuffer;

			std::unordered_map<String, PassResourceVariant> ResourceMap;

			// NOTE : pair<ResourceOwnName, LayoutAtStart>
			std::vector<std::pair<String, VkImageLayout>> AttachmentLayouts;
			std::vector<String> InputBufferResourceNames;

			std::vector<VkClearValue> ClearColors;
			PassDesc::PassCallbackType Callback;
		};
		std::unordered_map<String, PassContainer> Passes;

		std::vector<String> PassExecutionOrder;
		
		String BlitToSwapchainResourceOwnName;

		bool ResourcesWereRecreated = false;
		bool FramebuffersNeedsInitialization = false;

		Rect2Dui CurrentViewport = {};
	};
}
