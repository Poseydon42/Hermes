#pragma once

#include "Core/Core.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"

namespace Hermes
{
	class HERMES_API GPUInteractionUtilities
	{
	public:
		static void UploadDataToGPUBuffer(const void* Data, size_t DataSize, size_t TargetOffset, RenderInterface::Buffer& Target);

	private:
		static RenderInterface::Buffer& EnsureStagingBuffer();

		static std::shared_ptr<RenderInterface::Buffer> StagingBuffer;
	};
}
