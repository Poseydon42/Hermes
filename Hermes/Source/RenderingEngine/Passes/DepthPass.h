#pragma once

#include <memory>

#include "Core/Core.h"
#include "RenderingEngine/FrameGraph/Pass.h"

namespace Hermes
{
	class HERMES_API DepthPass
	{
	public:
		DepthPass();

		const PassDesc& GetPassDescription() const;
	private:
		std::unique_ptr<Vulkan::Buffer> SceneUBO;
		std::unique_ptr<Vulkan::DescriptorSet> SceneUBODescriptorSet;

		PassDesc Description;

		void PassCallback(const PassCallbackInfo& CallbackInfo);
	};
}
