#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Math/Vector2.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"
#include "RenderInterface/GenericRenderInterface/Resource.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"


namespace Hermes
{
	namespace RenderInterface
	{
		struct PipelineDescription;
		class Pipeline;
		struct RenderPassDescription;
		class RenderPass;
		class Fence;
		class Swapchain;
		class Resource;
		
		/**
		 * Represents a 'logical device', basically an interface to all GPU functionality
		 */
		class HERMES_API Device
		{
			MAKE_NON_COPYABLE(Device)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(Device)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Device)
			ADD_DEFAULT_CONSTRUCTOR(Device)

		public:
			virtual std::shared_ptr<Swapchain> CreateSwapchain(Vec2i Size, uint32 NumFrames) = 0;

			virtual std::shared_ptr<Queue> GetQueue(QueueType Type) = 0;

			virtual std::shared_ptr<Resource> CreateBuffer(size_t Size, ResourceUsageType Usage) = 0;

			virtual std::shared_ptr<Fence> CreateFence(bool InitialState = false) = 0;

			virtual std::shared_ptr<Shader> CreateShader(const String& Path, ShaderType Type) = 0;

			virtual std::shared_ptr<RenderPass> CreateRenderPass(const RenderPassDescription& Description) = 0;

			virtual std::shared_ptr<Pipeline> CreatePipeline(std::shared_ptr<RenderPass>& RenderPass, const PipelineDescription& Description) = 0;

			/**
			 * Waits until device finishes all its current and pending work and becomes idle
			 */
			virtual void WaitForIdle() = 0;
		};
	}
}
