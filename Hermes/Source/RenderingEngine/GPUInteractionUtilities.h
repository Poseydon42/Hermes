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

		static void UploadDataToGPUImage(
			const void* Data, Vec2ui Offset, Vec2ui Dimensions, size_t BytesPerPixel,
			uint32 MipLevel, RenderInterface::Image& Destination,
			RenderInterface::ImageLayout CurrentLayout, RenderInterface::ImageLayout LayoutToTransitionTo);

	private:
		static RenderInterface::Buffer& EnsureStagingBuffer();

		static std::shared_ptr<RenderInterface::Buffer> StagingBuffer;
	};
}
