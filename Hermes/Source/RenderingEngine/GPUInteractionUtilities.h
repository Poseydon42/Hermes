#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"

namespace Hermes
{
	class HERMES_API GPUInteractionUtilities
	{
	public:
		static void UploadDataToGPUBuffer(
			const void* Data, size_t DataSize,
			size_t TargetOffset, RenderInterface::Buffer& Target);

		/**
		 * Uploads image data stored in @param Data to @param Destination via staging buffer
		 * @param Offset Offset into destination image
		 * @param Dimensions Dimensions of area that is uploaded
		 * @param BytesPerPixel Bytes per pixel in source image data
		 * @param MipLevel Mip level to upload to
		 * @param CurrentLayout Current layout of ALL mip levels of image
		 * @param LayoutToTransitionTo Layout that ALL mip levels of image will have after upload is finished
		 */
		static void UploadDataToGPUImage(
			const void* Data, Vec2ui Offset, Vec2ui Dimensions, size_t BytesPerPixel,
			uint32 MipLevel, RenderInterface::Image& Destination,
			RenderInterface::ImageLayout CurrentLayout, RenderInterface::ImageLayout LayoutToTransitionTo);

		static void GenerateMipMaps(
			RenderInterface::Image& Image,
			RenderInterface::ImageLayout CurrentLayout, RenderInterface::ImageLayout LayoutToTransitionTo);

	private:
		static RenderInterface::Buffer& EnsureStagingBuffer();

		static std::shared_ptr<RenderInterface::Buffer> StagingBuffer;
	};
}
