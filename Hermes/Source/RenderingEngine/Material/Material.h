#pragma once

#include <vector>

#include "Core/Core.h"
#include "RenderingEngine/Material/MaterialProperty.h"
#include "RenderingEngine/Texture.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class MaterialInstance;

	/*
	 * A generic definition of a material - collection of shaders, a set of properties that can be changed
	 * by the user and the graphics pipeline object that this material uses.
	 *
	 * A material cannot be applied directly to an object. It serves as a template based on which material
	 * instances can be created.
	 */
	class HERMES_API Material : public std::enable_shared_from_this<Material>
	{
	public:
		static std::shared_ptr<Material> Create();

		std::unique_ptr<MaterialInstance> CreateInstance() const;

		const std::vector<MaterialProperty>& GetProperties() const;

		const MaterialProperty* FindProperty(const String& Name) const;

		const RenderInterface::DescriptorSetLayout& GetDescriptorSetLayout() const;

		const RenderInterface::Pipeline& GetPipeline() const;

	private:
		std::vector<MaterialProperty> Properties;
		size_t UniformBufferSize;

		std::unique_ptr<RenderInterface::DescriptorSetLayout> DescriptorSetLayout;
		std::unique_ptr<RenderInterface::Pipeline> Pipeline;

		Material();

		size_t CalculateUniformBufferSize() const;
	};
}
