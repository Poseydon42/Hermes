#pragma once

#include <memory>
#include <unordered_map>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Image.h"

namespace Hermes
{
	class FrameGraph;
	class GeometryList;

	class HERMES_API FrameGraphScheme
	{
	public:
		void AddPass(const String& Name, const PassDesc& Desc);

		void AddLink(const String& From, const String& To);

		void AddResource(const String& Name, const ImageResourceDescription& Description, bool IsExternal);

		void AddResource(const String& Name, const BufferResourceDescription& Description, bool IsExternal);

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
			bool IsExternal = false;
		};
		std::vector<BufferResourceContainer> BufferResources;

		// NOTE : ForwardLinks holds links between outputs of first render pass and inputs of the second
		//        BackwardLinks is just a reversed version of it because STL does not have bidirectional map
		std::unordered_map<String, String> ForwardLinks;
		std::unordered_map<String, String> BackwardLinks;

		bool Validate() const;
	};

	class HERMES_API FrameGraph
	{
		MAKE_NON_COPYABLE(FrameGraph)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(FrameGraph)
		ADD_DEFAULT_DESTRUCTOR(FrameGraph)
	public:
		void BindExternalResource(const String& Name, const Vulkan::Image& Image, const Vulkan::ImageView& View, VkImageLayout CurrentLayout);

		void BindExternalResource(const String& Name, const Vulkan::Buffer& Buffer);

		void Execute(const Scene& Scene, const GeometryList& GeometryList, Vec2ui ViewportDimensions);

		/*
		 * Returns the image containing the result of rendering together with the layout it is currently in.
		 * The user must return the image to the same layout before the next call to Execute().
		 */
		std::pair<const Vulkan::Image*, VkImageLayout> GetFinalImage() const;

	private:
		friend class FrameGraphScheme;

		explicit FrameGraph(FrameGraphScheme InScheme);

		String TraverseResourceName(const String& FullAttachmentName);

		VkImageUsageFlags TraverseImageResourceUsageType(const String& ResourceName) const;

		VkBufferUsageFlags TraverseBufferResourceUsageType(const String& ResourceName) const;

		bool TraverseCheckIfBufferIsMappable(const String& ResourceName) const;

		VkFormat TraverseAttachmentDataFormat(const String& AttachmentName) const;

		void RecreateResources();

		FrameGraphScheme Scheme;

		struct ImageResourceContainer
		{
			std::unique_ptr<Vulkan::Image> Image;
			const Vulkan::Image* ExternalImage = nullptr;

			std::unique_ptr<Vulkan::ImageView> View;
			const Vulkan::ImageView* ExternalView = nullptr;

			VkImageLayout CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			ImageResourceDescription Desc;
			bool IsExternal = false;

			const Vulkan::Image& GetImage() const;
			const Vulkan::ImageView& GetView() const;
		};
		std::unordered_map<String, ImageResourceContainer> ImageResources;

		struct BufferResourceContainer
		{
			std::unique_ptr<Vulkan::Buffer> Buffer;
			const Vulkan::Buffer* ExternalBuffer = nullptr;

			BufferResourceDescription Desc;
			bool IsExternal = false;

			const Vulkan::Buffer& GetBuffer() const;
		};
		std::unordered_map<String, BufferResourceContainer> BufferResources;

		struct PassContainer
		{
			// Pair of the name of the resource (not attachment) and the corresponding attachment info
			std::vector<std::pair<String, VkRenderingAttachmentInfo>> ColorAttachments;
			std::optional<std::pair<String, VkRenderingAttachmentInfo>> DepthAttachment;

			// Pair of resource name and its layout at the start of render pass
			std::vector<std::pair<String, VkImageLayout>> ImageResourceLayouts;

			std::vector<std::pair<String, String>> ImageAttachmentResourceNames;
			std::vector<std::pair<String, String>> BufferInputResourceNames;

			std::vector<VkClearValue> ClearColors;
			PassDesc::PassCallbackType Callback;
		};
		std::unordered_map<String, PassContainer> Passes;

		std::vector<String> PassExecutionOrder;
		
		String FinalImageResourceName;

		Vec2ui CurrentViewportDimensions = {};
	};
}
