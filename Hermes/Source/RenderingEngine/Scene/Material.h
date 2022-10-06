#pragma once

#include <vector>

#include "Core/Core.h"
#include "Math/Math.h"
#include "RenderingEngine/Texture.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class HERMES_API Material
	{
	public:
		Material();

		const RenderInterface::DescriptorSet& GetMaterialDescriptorSet() const;

		const RenderInterface::Pipeline& GetPipeline() const;

	private:
		struct MaterialData
		{
			Vec4 Color;
		};

		std::unique_ptr<RenderInterface::DescriptorSetLayout> DescriptorSetLayout;
		std::unique_ptr<RenderInterface::DescriptorSet> DescriptorSet;
		std::unique_ptr<RenderInterface::Pipeline> Pipeline;
		std::unique_ptr<RenderInterface::Buffer> UniformBuffer;
	};
}
