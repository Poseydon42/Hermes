#pragma once

#include <vector>

#include "Core/Core.h"
#include "RenderingEngine/Material/MaterialProperty.h"
#include "RenderingEngine/Material/ShaderReflection.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Pipeline.h"

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
		static std::shared_ptr<Material> Create(const String& VertexShaderPath, const String& FragmentShaderPath);

		std::unique_ptr<MaterialInstance> CreateInstance() const;

		const MaterialProperty* FindProperty(const String& Name) const;

		const Vulkan::DescriptorSetLayout& GetDescriptorSetLayout() const;

		const Vulkan::Pipeline& GetPipeline() const;

		/*
		 * Returns a pipeline with vertex shader only that can be used for things like depth pass etc.
		 */
		const Vulkan::Pipeline& GetVertexPipeline() const;

	private:
		ShaderReflection Reflection;

		std::unique_ptr<Vulkan::DescriptorSetLayout> DescriptorSetLayout;
		std::unique_ptr<Vulkan::Pipeline> Pipeline, VertexPipeline;

		Material(const String& VertexShaderPath, const String& FragmentShaderPath);
	};
}
