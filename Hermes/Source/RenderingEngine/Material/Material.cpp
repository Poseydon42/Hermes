#include "Material.h"

#include "AssetSystem/MeshAsset.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"
#include "RenderingEngine/Material/MaterialInstance.h"

namespace Hermes
{

	Material::Material()
		: Reflection(L"Shaders/Bin/solid_color_frag.glsl.spv")
	{

		auto& Device = Renderer::Get().GetActiveDevice();

		std::vector<RenderInterface::DescriptorBinding> PerMaterialDataBindings;

		// Binding 0 is always material uniform buffer that stores numeric properties
		RenderInterface::DescriptorBinding UBOBinding = {};
		UBOBinding.Index = 0;
		UBOBinding.DescriptorCount = 1;
		UBOBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		UBOBinding.Type = RenderInterface::DescriptorType::UniformBuffer;
		PerMaterialDataBindings.push_back(UBOBinding);

		// Iterate over texture properties and add their corresponding bindings to the list
		for (const auto& Property : Reflection.GetProperties())
		{
			if (Property.second.Type != MaterialPropertyType::Texture)
				continue;

			RenderInterface::DescriptorBinding Binding = {};
			Binding.Index = Property.second.Binding;
			Binding.DescriptorCount = 1;
			Binding.Shader = RenderInterface::ShaderType::FragmentShader;
			Binding.Type = RenderInterface::DescriptorType::CombinedSampler;
			PerMaterialDataBindings.push_back(Binding);
		}

		DescriptorSetLayout = Device.CreateDescriptorSetLayout(PerMaterialDataBindings);

		auto VertexShader = Device.CreateShader(L"Shaders/Bin/solid_color_vert.glsl.spv",
		                                        RenderInterface::ShaderType::VertexShader);
		auto FragmentShader = Device.CreateShader(L"Shaders/Bin/solid_color_frag.glsl.spv",
		                                          RenderInterface::ShaderType::FragmentShader);RenderInterface::PipelineDescription PipelineDesc = {};
		PipelineDesc.PushConstants.push_back({ RenderInterface::ShaderType::VertexShader, 0, sizeof(GlobalDrawcallData) });
		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDesc.DescriptorLayouts = {
			&Renderer::Get().GetGlobalDataDescriptorSetLayout(), DescriptorSetLayout.get()
		};

		RenderInterface::VertexBinding VertexInput = {};
		VertexInput.Index = 0;
		VertexInput.Stride = sizeof(Vertex);
		VertexInput.IsPerInstance = false;
		PipelineDesc.VertexInput.VertexBindings.push_back(VertexInput);

		RenderInterface::VertexAttribute PositionAttribute = {}, TextureCoordinatesAttribute = {}, NormalAttribute = {}, TangentAttribute = {};
		PositionAttribute.BindingIndex = 0;
		PositionAttribute.Location = 0;
		PositionAttribute.Offset = offsetof(Vertex, Position);
		PositionAttribute.Format = RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(PositionAttribute);

		TextureCoordinatesAttribute.BindingIndex = 0;
		TextureCoordinatesAttribute.Location = 1;
		TextureCoordinatesAttribute.Offset = offsetof(Vertex, TextureCoordinates);
		TextureCoordinatesAttribute.Format = RenderInterface::DataFormat::R32G32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(TextureCoordinatesAttribute);

		NormalAttribute.BindingIndex = 0;
		NormalAttribute.Location = 2;
		NormalAttribute.Offset = offsetof(Vertex, Normal);
		NormalAttribute.Format = RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(NormalAttribute);

		TangentAttribute.BindingIndex = 0;
		TangentAttribute.Location = 3;
		TangentAttribute.Offset = offsetof(Vertex, Tangent);
		TangentAttribute.Format = RenderInterface::DataFormat::R32G32B32SignedFloat;
		PipelineDesc.VertexInput.VertexAttributes.push_back(TangentAttribute);

		PipelineDesc.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;

		PipelineDesc.Viewport.Origin = { 0 };
		// TODO : recreate pipeline on resize
		PipelineDesc.Viewport.Dimensions = Renderer::Get().GetSwapchain().GetSize();

		PipelineDesc.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDesc.Rasterizer.Direction = RenderInterface::FaceDirection::Clockwise;
		PipelineDesc.Rasterizer.Fill = RenderInterface::FillMode::Fill;

		PipelineDesc.DepthStencilStage.ComparisonMode = RenderInterface::ComparisonOperator::Greater;
		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = true;

		Pipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Renderer::Get().GetGraphicsRenderPassObject(),
		                                                            PipelineDesc);
	}

	std::shared_ptr<Material> Material::Create()
	{
		return std::shared_ptr<Material>(new Material());
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

	const RenderInterface::DescriptorSetLayout& Material::GetDescriptorSetLayout() const
	{
		return *DescriptorSetLayout;
	}

	const RenderInterface::Pipeline& Material::GetPipeline() const
	{
		return *Pipeline;
	}
}
