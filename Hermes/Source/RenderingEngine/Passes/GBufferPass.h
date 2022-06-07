#pragma once

#include <memory>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderingEngine/Scene/SceneProxies.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class Scene;
	struct PassDesc;

	class HERMES_API GBufferPass
	{
	public:
		GBufferPass(std::shared_ptr<RenderInterface::Device> InDevice);

		const PassDesc& GetPassDescription() const;

	private:
		std::shared_ptr<RenderInterface::Device> Device;

		std::shared_ptr<RenderInterface::Buffer> SceneDataUniformBuffer, LightingDataUniformBuffer;
		std::shared_ptr<RenderInterface::DescriptorSetLayout> PerFrameUBODescriptorLayout;
		std::shared_ptr<RenderInterface::DescriptorSet> PerFrameDataDescriptor;

		bool IsPipelineCreated = false;

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
			uint32 PointLightCount = 0;
			float AmbientLightingCoefficient = 0.0f;
		};

		std::shared_ptr<RenderInterface::Pipeline> Pipeline;
		std::shared_ptr<RenderInterface::Shader> VertexShader, FragmentShader;

		PassDesc Descriptor;

		void PassCallback(
			RenderInterface::CommandBuffer& CommandBuffer,
			const RenderInterface::RenderPass& PassInstance,
			const std::vector<const RenderInterface::Image*>& Drains,
			const Scene& Scene, bool ResourcesWereRecreated);

		void RecreatePipeline(const RenderInterface::RenderPass& Pass);
	};
}
