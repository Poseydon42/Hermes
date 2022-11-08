#include "Material.h"

#include "AssetSystem/MeshAsset.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Device.h"
#include "Vulkan/Swapchain.h"
#include "RenderingEngine/Material/MaterialInstance.h"

namespace Hermes
{

	Material::Material(const String& VertexShaderPath, const String& FragmentShaderPath)
		: Reflection(FragmentShaderPath)
	{

		auto& Device = Renderer::Get().GetActiveDevice();

		std::vector<VkDescriptorSetLayoutBinding> PerMaterialDataBindings;

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

		auto VertexShader = Device.CreateShader(VertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT);
		auto FragmentShader = Device.CreateShader(FragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::PipelineDescription PipelineDesc = {};
		PipelineDesc.PushConstants.push_back({ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GlobalDrawcallData) });
		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
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
		PipelineDesc.ShaderStages = { VertexShader.get() };
		// NOTE: vertex shaders don't have any user-defined material properties for now
		PipelineDesc.DescriptorSetLayouts = { &Renderer::Get().GetGlobalDataDescriptorSetLayout() };
		VertexPipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Renderer::Get().GetVertexRenderPassObject(), PipelineDesc);
	}

	std::shared_ptr<Material> Material::Create(const String& VertexShaderPath, const String& FragmentShaderPath)
	{
		return std::shared_ptr<Material>(new Material(VertexShaderPath, FragmentShaderPath));
	}

	std::unique_ptr<MaterialInstance> Material::CreateInstance() const
	{
		return std::unique_ptr<MaterialInstance>(new MaterialInstance(shared_from_this(),
		                                                              Reflection.GetTotalSizeForUniformBuffer()));
	}

	const MaterialProperty* Material::FindProperty(const String& Name) const
	{
		return Reflection.FindProperty(Name);
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
