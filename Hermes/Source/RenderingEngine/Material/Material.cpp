#include "Material.h"

#include "AssetSystem/AssetHeaders.h"
#include "AssetSystem/AssetLoader.h"
#include "JSON/JSONObject.h"
#include "JSON/JSONValue.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "Vulkan/Device.h"
#include "Vulkan/Swapchain.h"
#include "RenderingEngine/Material/MaterialInstance.h"

namespace Hermes
{
	DEFINE_ASSET_TYPE(Material, Material);
	HERMES_ADD_TEXT_ASSET_LOADER(Material, "material");

	Material::Material(String InName, AssetHandle InHandle, String InVertexShaderPath, String InFragmentShaderPath)
		: Asset(std::move(InName), AssetType::Material, InHandle)
		, VertexShaderName(std::move(InVertexShaderPath))
		, FragmentShaderName(std::move(InFragmentShaderPath))
	{
		auto& Device = Renderer::Get().GetActiveDevice();

		std::vector<VkDescriptorSetLayoutBinding> PerMaterialDataBindings;

		auto& ShaderCache = Renderer::Get().GetShaderCache();

		const auto& Reflection = ShaderCache.GetShaderReflection(FragmentShaderName, VK_SHADER_STAGE_FRAGMENT_BIT);
		if (Reflection.RequiresUniformBuffer())
		{
			// Binding 0 is always material uniform buffer that stores numeric properties
			VkDescriptorSetLayoutBinding UBOBinding = {};
			UBOBinding.binding = 0;
			UBOBinding.descriptorCount = 1;
			UBOBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			UBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			PerMaterialDataBindings.push_back(UBOBinding);
		}

		// Iterate over texture properties and add their corresponding bindings to the list
		for (const auto& Property : Reflection.GetProperties())
		{
			if (Property.second.Type != MaterialPropertyType::Texture)
				continue;

			VkDescriptorSetLayoutBinding Binding = {};
			Binding.binding = Property.second.Binding;
			Binding.descriptorCount = 1;
			Binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			Binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			PerMaterialDataBindings.push_back(Binding);
		}

		DescriptorSetLayout = Device.CreateDescriptorSetLayout(PerMaterialDataBindings);

		const auto& VertexShader = ShaderCache.GetShader(VertexShaderName, VK_SHADER_STAGE_VERTEX_BIT);
		const auto& FragmentShader = ShaderCache.GetShader(FragmentShaderName, VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::PipelineDescription PipelineDesc = {};
		PipelineDesc.PushConstants.push_back({ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GlobalDrawcallData) });
		PipelineDesc.ShaderStages = { &VertexShader, &FragmentShader };
		PipelineDesc.DescriptorSetLayouts = {
			&Renderer::Get().GetGlobalDataDescriptorSetLayout(), DescriptorSetLayout.get()
		};

		VkVertexInputBindingDescription VertexInput = {};
		VertexInput.binding = 0;
		VertexInput.stride = sizeof(Vertex);
		VertexInput.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		PipelineDesc.VertexInputBindings.push_back(VertexInput);

		VkVertexInputAttributeDescription PositionAttribute = {}, TextureCoordinatesAttribute = {}, NormalAttribute = {}, TangentAttribute = {};
		PositionAttribute.binding = 0;
		PositionAttribute.location = 0;
		PositionAttribute.offset = offsetof(Vertex, Position);
		PositionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		PipelineDesc.VertexInputAttributes.push_back(PositionAttribute);

		TextureCoordinatesAttribute.binding = 0;
		TextureCoordinatesAttribute.location = 1;
		TextureCoordinatesAttribute.offset = offsetof(Vertex, TextureCoordinates);
		TextureCoordinatesAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		PipelineDesc.VertexInputAttributes.push_back(TextureCoordinatesAttribute);

		NormalAttribute.binding = 0;
		NormalAttribute.location = 2;
		NormalAttribute.offset = offsetof(Vertex, Normal);
		NormalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		PipelineDesc.VertexInputAttributes.push_back(NormalAttribute);

		TangentAttribute.binding = 0;
		TangentAttribute.location = 3;
		TangentAttribute.offset = offsetof(Vertex, Tangent);
		TangentAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		PipelineDesc.VertexInputAttributes.push_back(TangentAttribute);

		PipelineDesc.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		PipelineDesc.Viewport.x = 0;
		PipelineDesc.Viewport.y = 0;
		PipelineDesc.Viewport.minDepth = 0.0f;
		PipelineDesc.Viewport.maxDepth = 1.0f;
		// TODO : recreate pipeline on resize
		PipelineDesc.Viewport.width = static_cast<float>(Renderer::Get().GetSwapchain().GetDimensions().X);
		PipelineDesc.Viewport.height = static_cast<float>(Renderer::Get().GetSwapchain().GetDimensions().Y);
		PipelineDesc.Scissor.offset = { 0, 0 };
		PipelineDesc.Scissor.extent = {
			Renderer::Get().GetSwapchain().GetDimensions().X, Renderer::Get().GetSwapchain().GetDimensions().Y
		};

		PipelineDesc.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineDesc.FaceDirection = VK_FRONT_FACE_CLOCKWISE;
		PipelineDesc.PolygonMode = VK_POLYGON_MODE_FILL;

		PipelineDesc.DepthCompareOperator = VK_COMPARE_OP_GREATER_OR_EQUAL;
		PipelineDesc.IsDepthTestEnabled = true;
		PipelineDesc.IsDepthWriteEnabled = true;

		Pipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Renderer::Get().GetGraphicsRenderPassObject(),
		                                                            PipelineDesc);
		PipelineDesc.ShaderStages = { &VertexShader };
		// NOTE: vertex shaders don't have any user-defined material properties for now
		PipelineDesc.DescriptorSetLayouts = { &Renderer::Get().GetGlobalDataDescriptorSetLayout() };
		VertexPipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Renderer::Get().GetVertexRenderPassObject(), PipelineDesc);
	}

	std::unique_ptr<Material> Material::Create(String Name, AssetHandle Handle, String VertexShaderPath, String FragmentShaderPath)
	{
		return std::unique_ptr<Material>(new Material(std::move(Name), Handle, std::move(VertexShaderPath), std::move(FragmentShaderPath)));
	}

	std::unique_ptr<Asset> Material::Load(String Name, AssetHandle Handle, const JSONObject& Data)
	{
		if (!Data.Contains("shaders") || !Data["shaders"].Is(JSONValueType::Object))
		{
			HERMES_LOG_ERROR("Material asset %s does not specify any shaders", Name.data());
			return nullptr;
		}

		const auto& JSONShaders = Data["shaders"].AsObject();

		String VertexShader, FragmentShader;
		for (const auto& JSONShader : JSONShaders)
		{
			if (!JSONShader.second.Is(JSONValueType::String))
				continue;

			const auto& Type = JSONShader.first;
			auto Path = JSONShader.second.AsString();

			if (Type == "vertex")
				VertexShader = String(Path);
			if (Type == "fragment")
				FragmentShader = String(Path);
		}

		return Create(std::move(Name), Handle, std::move(VertexShader), std::move(FragmentShader));
	}

	std::unique_ptr<MaterialInstance> Material::CreateInstance() const
	{
		auto& ShaderCache = Renderer::Get().GetShaderCache();
		const auto& Reflection = ShaderCache.GetShaderReflection(FragmentShaderName, VK_SHADER_STAGE_FRAGMENT_BIT);

		return std::unique_ptr<MaterialInstance>(new MaterialInstance(GetSelfHandle(), Reflection.GetTotalSizeForUniformBuffer()));
	}

	const MaterialProperty* Material::FindProperty(const String& PropertyName) const
	{
		auto& ShaderCache = Renderer::Get().GetShaderCache();
		const auto& Reflection = ShaderCache.GetShaderReflection(FragmentShaderName, VK_SHADER_STAGE_FRAGMENT_BIT);

		return Reflection.FindProperty(PropertyName);
	}

	const Vulkan::DescriptorSetLayout& Material::GetDescriptorSetLayout() const
	{
		return *DescriptorSetLayout;
	}

	const Vulkan::Pipeline& Material::GetPipeline() const
	{
		return *Pipeline;
	}

	const Vulkan::Pipeline& Material::GetVertexPipeline() const
	{
		return *VertexPipeline;
	}
}
