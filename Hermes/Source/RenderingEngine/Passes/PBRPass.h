#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "RenderingEngine/FrameGraph/Pass.h"
#include "RenderingEngine/Scene/SceneProxies.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"
#include "RenderInterface/GenericRenderInterface/Buffer.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"

namespace Hermes
{
	class HERMES_API PBRPass
	{
		MAKE_NON_COPYABLE(PBRPass)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(PBRPass)
		ADD_DEFAULT_DESTRUCTOR(PBRPass)

	public:
		PBRPass();

		const PassDesc& GetPassDescription() const;

	private:
		std::unique_ptr<RenderInterface::Buffer> LightingDataUniformBuffer;
		std::shared_ptr<RenderInterface::DescriptorSetLayout> DescriptorLayout;
		std::unique_ptr<RenderInterface::DescriptorSet> DescriptorSet;

		bool IsPipelineCreated = false;

		// NOTE : keep in sync with shader code
		static constexpr uint32 MaxPointLightCount = 256;
		static constexpr float DefaultAmbientLightingCoefficient = 0.1f;

		struct LightingData
		{
			PointLightProxy PointLights[MaxPointLightCount];
			Vec4 CameraPosition;
			uint32 PointLightCount = 0;
			float AmbientLightingCoefficient = 0.0f;
		};

		std::unique_ptr<RenderInterface::Pipeline> Pipeline;
		std::shared_ptr<RenderInterface::Shader> VertexShader, FragmentShader;

		PassDesc Description;

		void PassCallback(
			RenderInterface::CommandBuffer& CommandBuffer,
			const RenderInterface::RenderPass& PassInstance,
			const std::vector<std::pair<const RenderInterface::Image*, const RenderInterface::ImageView*>>& Drains,
			const Scene& Scene, bool ResourcesWereRecreated);

		void RecreatePipeline(const RenderInterface::RenderPass& Pass);
	};
}
