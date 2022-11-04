#pragma once

#include <memory>
#include <optional>

#include "Core/Core.h"
#include "Math/Math.h"
#include "Vulkan/Forward.h"
#include "Vulkan/VulkanCore.h"

namespace Hermes
{
	class HERMES_API GPUInteractionUtilities
	{
	public:
		static void UploadDataToGPUBuffer(
			const void* Data, size_t DataSize,
			size_t TargetOffset, const Vulkan::Buffer& Target);

		/**
		 * Uploads image data stored in @param Data to @param Destination via staging buffer
		 * @param Offset Offset into destination image
		 * @param Dimensions Dimensions of area that is uploaded
		 * @param BytesPerPixel Bytes per pixel in source image data
		 * @param MipLevel Mip level to upload to
		 * @param CurrentLayout Current layout of ALL mip levels of image
		 * @param LayoutToTransitionTo Layout that ALL mip levels of image will have after upload is finished
		 * @param Side Side of cubemap image to upload data to(ignored if image is not cubemap)
		 */
		static void UploadDataToGPUImage(const void* Data, Vec2ui Offset, Vec2ui Dimensions, size_t BytesPerPixel,
		                                 uint32 MipLevel, Vulkan::Image& Destination,
		                                 VkImageLayout CurrentLayout,
		                                 VkImageLayout LayoutToTransitionTo,
		                                 std::optional<Vulkan::CubemapSide> Side = {});

		static void GenerateMipMaps(const Vulkan::Image& Image,
		                            VkImageLayout CurrentLayout,
		                            VkImageLayout LayoutToTransitionTo,
		                            std::optional<Vulkan::CubemapSide> Side = {});

	private:
		static Vulkan::Buffer& EnsureStagingBuffer();

		static std::unique_ptr<Vulkan::Buffer> StagingBuffer;
	};
}
