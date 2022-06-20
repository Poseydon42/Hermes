#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Math/Vector2.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"


namespace Hermes
{
	namespace RenderInterface
	{		
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
			virtual std::unique_ptr<Swapchain> CreateSwapchain(uint32 NumFrames) const = 0;

			virtual const Queue& GetQueue(QueueType Type) const = 0;

			virtual std::unique_ptr<Buffer> CreateBuffer(size_t Size, BufferUsageType Usage) const = 0;

			virtual std::unique_ptr<Image> CreateImage(
				Vec2ui Size, ImageUsageType Usage, DataFormat Format,
				uint32 MipLevels, ImageLayout InitialLayout) const = 0;

			virtual std::unique_ptr<Image> CreateCubemap(
				Vec2ui Size, ImageUsageType Usage, DataFormat Format,
				uint32 MipLevels, ImageLayout InitialLayout) const = 0;

			virtual std::unique_ptr<Fence> CreateFence(bool InitialState = false) const = 0;

			virtual std::unique_ptr<Shader> CreateShader(const String& Path, ShaderType Type) const = 0;

			virtual std::unique_ptr<RenderPass> CreateRenderPass(const std::vector<RenderPassAttachment>& Attachments) const = 0;

			virtual std::unique_ptr<Pipeline> CreatePipeline(
				const RenderPass& RenderPass, const PipelineDescription& Description) const = 0;

			virtual std::unique_ptr<RenderTarget> CreateRenderTarget(
				std::shared_ptr<RenderPass> RenderPass, const std::vector<std::shared_ptr<Image>>& Attachments, 
				Vec2ui Size) const = 0;

			virtual std::unique_ptr<DescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<DescriptorBinding>& Bindings) const = 0;
			
			virtual std::unique_ptr<DescriptorSetPool> CreateDescriptorSetPool(uint32 NumberOfSets, const std::vector<SubpoolDescription>& Subpools, bool SupportIndividualDeallocations = false) const = 0;

			virtual std::unique_ptr<Sampler> CreateSampler(const SamplerDescription& Description) const = 0;

			/**
			 * Waits until device finishes all its current and pending work and becomes idle
			 */
			virtual void WaitForIdle() const = 0;
		};
	}
}
