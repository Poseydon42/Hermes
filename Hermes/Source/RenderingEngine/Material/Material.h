#pragma once

#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetLoader.h"
#include "Core/Core.h"
#include "RenderingEngine/Material/MaterialProperty.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Pipeline.h"

namespace Hermes
{
	class JSONObject;
	class MaterialInstance;

	/*
	 * A generic definition of a material - collection of shaders, a set of properties that can be changed
	 * by the user and the graphics pipeline object that this material uses.
	 *
	 * A material cannot be applied directly to an object. It serves as a template based on which material
	 * instances can be created.
	 */
	class HERMES_API Material : public Asset
	{
	public:
		static std::unique_ptr<Material> Create(String Name, AssetHandle Handle, String VertexShaderPath, String FragmentShaderPath);

		static std::unique_ptr<Asset> Load(const AssetLoaderCallbackInfo& CallbackInfo, const JSONObject& Data);

		const MaterialProperty* FindProperty(const String& PropertyName) const;

		const Vulkan::DescriptorSetLayout& GetDescriptorSetLayout() const;

		const Vulkan::Pipeline& GetFullPipeline(VkFormat ColorAttachmentFormat, VkFormat DepthAttachmentFormat) const;

		/*
		 * Returns a pipeline with vertex shader only that can be used for things like depth pass etc.
		 */
		const Vulkan::Pipeline& GetVertexOnlyPipeline(VkFormat DepthAttachmentFormat) const;

		size_t GetUniformBufferSize() const;

	private:
		String VertexShaderName, FragmentShaderName;

		std::unique_ptr<Vulkan::DescriptorSetLayout> DescriptorSetLayout;

		mutable std::unique_ptr<Vulkan::Pipeline> FullPipeline;
		mutable std::unique_ptr<Vulkan::Pipeline> VertexOnlyPipeline;

		mutable VkFormat CachedPipelineColorAttachmentFormat = VK_FORMAT_UNDEFINED;
		mutable VkFormat CachedPipelineDepthAttachmentFormat = VK_FORMAT_UNDEFINED;


		Material(String InName, AssetHandle InHandle, String InVertexShaderPath, String InFragmentShaderPath);

		void CreatePipelines(VkFormat ColorAttachmentFormat, VkFormat DepthAttachmentFormat) const;
	};
}
