#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Vector2.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "Vulkan/ComputePipeline.h"
#include "Vulkan/Descriptor.h"

namespace Hermes
{
	class HERMES_API LightCullingPass
	{
	public:
		LightCullingPass();

		const PassDesc& GetPassDescription() const;

		static constexpr Vec2ui ClusterSizeInPixels = { 32 };
		static constexpr uint32 NumberOfZSlices = 32;

	private:
		std::unique_ptr<Vulkan::ComputePipeline> Pipeline;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;

		PassDesc PassDescription = {};

		void PassCallback(const PassCallbackInfo& CallbackInfo);
	};
}
