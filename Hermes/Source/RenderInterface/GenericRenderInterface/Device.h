#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Math/Vector2.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"


namespace Hermes
{
	namespace RenderInterface
	{
		struct SubpoolDescription;
		enum class DescriptorType;
		struct SamplerDescription;
		class Sampler;
		enum class ImageLayout;
		enum class DataFormat;
		enum class ImageUsageType;
		class DescriptorSetPool;
		struct DescriptorBinding;
		class DescriptorSetLayout;
		class RenderTarget;
		class Image;
		struct PipelineDescription;
		class Pipeline;
		struct RenderPassDescription;
		class RenderPass;
		class Fence;
		class Swapchain;
		class Buffer;
		
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
			virtual std::shared_ptr<Swapchain> CreateSwapchain(uint32 NumFrames) = 0;

			virtual std::shared_ptr<Queue> GetQueue(QueueType Type) = 0;

			virtual std::shared_ptr<Buffer> CreateBuffer(size_t Size, BufferUsageType Usage) = 0;

			virtual std::shared_ptr<Image> CreateImage(Vec2ui Size, ImageUsageType Usage, DataFormat Format, uint32 MipLevels, ImageLayout InitialLayout) = 0;

			virtual std::shared_ptr<Fence> CreateFence(bool InitialState = false) = 0;

			virtual std::shared_ptr<Shader> CreateShader(const String& Path, ShaderType Type) = 0;

			virtual std::shared_ptr<RenderPass> CreateRenderPass(const RenderPassDescription& Description) = 0;

			virtual std::shared_ptr<Pipeline> CreatePipeline(std::shared_ptr<RenderPass> RenderPass, const PipelineDescription& Description) = 0;

			virtual std::shared_ptr<RenderTarget> CreateRenderTarget(std::shared_ptr<RenderPass> RenderPass, const std::vector<std::shared_ptr<Image>>& Attachments, Vec2ui Size) = 0;

			virtual std::shared_ptr<DescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<DescriptorBinding>& Bindings) = 0;
			
			virtual std::shared_ptr<DescriptorSetPool> CreateDescriptorSetPool(uint32 NumberOfSets, const std::vector<SubpoolDescription>& Subpools) = 0;

			virtual std::shared_ptr<Sampler> CreateSampler(const SamplerDescription& Description) = 0;

			/**
			 * Waits until device finishes all its current and pending work and becomes idle
			 */
			virtual void WaitForIdle() = 0;
		};
	}
}
