#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Matrix.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Scene/SceneProxies.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"
#include "RenderInterface/GenericRenderInterface/PhysicalDevice.h"
#include "RenderInterface/GenericRenderInterface/CommonTypes.h"

namespace Hermes
{
	class DescriptorAllocator;
	class Scene;
	class IPlatformWindow;

	struct GraphicsSettings
	{
		float AnisotropyLevel = 1.0;
	};

	class HERMES_API Renderer
	{
	public:
		static Renderer& Get();

		bool Init(const RenderInterface::PhysicalDevice& GPU);

		void UpdateGraphicsSettings(GraphicsSettings NewSettings);

		void RunFrame(const Scene& Scene);

		RenderInterface::Device& GetActiveDevice();

		RenderInterface::Swapchain& GetSwapchain();

		DescriptorAllocator& GetDescriptorAllocator();

	private:
		RenderInterface::DeviceProperties GPUProperties;
		GraphicsSettings CurrentSettings;

		std::shared_ptr<RenderInterface::Device> RenderingDevice;
		std::shared_ptr<RenderInterface::Swapchain> Swapchain;

		std::shared_ptr<RenderInterface::DescriptorSetLayout> PerFrameUBODescriptorLayout;
		std::shared_ptr<DescriptorAllocator> DescriptorAllocator;

		std::shared_ptr<RenderInterface::Buffer> SceneDataUniformBuffer, LightingDataUniformBuffer;
		std::shared_ptr<RenderInterface::DescriptorSet> PerFrameDataDescriptor;

		struct PerFrameSceneUBO
		{
			Mat4 ViewProjection;
		};

		// NOTE : keep in sync with shader code
		static constexpr uint32 MaxPointLightCount = 256;
		static constexpr float DefaultAmbientLightingCoefficient = 0.1f;

		struct PerFrameLightingUBO
		{
			PointLightProxy PointLights[MaxPointLightCount];
			Vec4 CameraPosition;
			uint32 PointLightCount;
			float AmbientLightingCoefficient;
		};

		std::shared_ptr<RenderInterface::Pipeline> Pipeline;

		std::shared_ptr<RenderInterface::Shader> VertexShader, FragmentShader;

		std::unique_ptr<class FrameGraph> FrameGraph;

		static constexpr uint32 NumberOfBackBuffers = 3; // TODO : let user modify
		static constexpr RenderInterface::DataFormat ColorAttachmentFormat = RenderInterface::DataFormat::B8G8R8A8UnsignedNormalized;
		static constexpr RenderInterface::DataFormat DepthAttachmentFormat = RenderInterface::DataFormat::D32SignedFloat;

		void GraphicsPassCallback(RenderInterface::CommandBuffer& CommandBuffer, const Scene& Scene, bool ResourcesWereRecreated);

		void RecreatePipeline();

		void DumpGPUProperties() const;
	};
}
