#pragma once

#include <vector>

#include "Core/Core.h"
#include "RenderingEngine/Material/MaterialProperty.h"
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
		// FIXME: this means that a material will never be destroyed, perhaps we should try to destroy unused materials when we're close to being out of memory?
		static std::unordered_map<String, std::shared_ptr<Material>> CreatedMaterials;

		String VertexShaderName, FragmentShaderName;

		std::unique_ptr<Vulkan::DescriptorSetLayout> DescriptorSetLayout;
		std::unique_ptr<Vulkan::Pipeline> Pipeline, VertexPipeline;

		Material(String InVertexShaderPath, String InFragmentShaderPath);
	};
}
