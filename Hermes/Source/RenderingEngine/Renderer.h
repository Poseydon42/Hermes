#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Matrix.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"

namespace Hermes
{
	class Scene;
	class IPlatformWindow;

	class HERMES_API Renderer
	{
	public:
		static Renderer& Get();

		bool Init(const RenderInterface::PhysicalDevice& GPU);

		void RunFrame(const Scene& Scene);

		RenderInterface::Device& GetActiveDevice();

	private:
		Renderer();

		std::shared_ptr<RenderInterface::Device> RenderingDevice;
		std::shared_ptr<RenderInterface::Swapchain> Swapchain;

		std::shared_ptr<RenderInterface::DescriptorSetLayout> PerFrameUBODescriptorLayout;
		std::shared_ptr<RenderInterface::DescriptorSetPool> PerFrameUBODescriptorPool;

		struct PerFrameStorage
		{
			std::shared_ptr<RenderInterface::Image> ColorBuffer, DepthBuffer;
			std::shared_ptr<RenderInterface::RenderTarget> RenderTarget;
			std::shared_ptr<RenderInterface::CommandBuffer> CommandBuffer;
			std::shared_ptr<RenderInterface::Buffer> UniformBuffer;
			std::shared_ptr<RenderInterface::DescriptorSet> PerFrameDataDescriptor;
		};

		struct PerFrameUBOData
		{
			Mat4 ViewProjection;
		};

		std::vector<PerFrameStorage> PerFrameObjects;
		std::shared_ptr<RenderInterface::RenderPass> RenderPass;
		std::shared_ptr<RenderInterface::Pipeline> Pipeline;
		std::shared_ptr<RenderInterface::Fence> RenderingFinishedFence, SwapchainImageAcquiredFence;

		std::shared_ptr<RenderInterface::Shader> VertexShader, FragmentShader;

		bool SwapchainWasRecreated;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
		static constexpr RenderInterface::DataFormat ColorAttachmentFormat = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		static constexpr RenderInterface::DataFormat DepthAttachmentFormat = RenderInterface::DataFormat::D32SignedFloat;

		static constexpr float VerticalFOV = 50.0f;
		static constexpr float NearPlane = 0.1f;
		static constexpr float FarPlane = 1000.0f;

		void RecreatePerFrameObjects();
	};
}
