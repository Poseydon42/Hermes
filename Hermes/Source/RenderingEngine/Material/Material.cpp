#include "Material.h"

#include <cstring>

#include "AssetSystem/MeshAsset.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

namespace Hermes
{

	Material::Material()
	{
		Vec4 DefaultColor = { 1.0f, 0.0f, 1.0f, 1.0f };
		MaterialProperty ColorProperty = { L"Color", MaterialPropertyType::Vec4, 0, {} };
		memmove(&ColorProperty.Value, &DefaultColor, sizeof(DefaultColor));
		Properties.push_back(ColorProperty);

		auto& Device = Renderer::Get().GetActiveDevice();

		std::vector<RenderInterface::DescriptorBinding> PerMaterialDataBindings =
		{
			/* UBO with material data */
			{
				0,
				1,
				RenderInterface::ShaderType::FragmentShader,
				RenderInterface::DescriptorType::UniformBuffer
			}
		};
		DescriptorSetLayout = Device.CreateDescriptorSetLayout(PerMaterialDataBindings);
		DescriptorSet = Renderer::Get().GetDescriptorAllocator().Allocate(*DescriptorSetLayout);

		auto UniformBufferSize = CalculateUniformBufferSize();

		UniformBuffer = Device.CreateBuffer(UniformBufferSize,
		                                    RenderInterface::BufferUsageType::UniformBuffer |
		                                    RenderInterface::BufferUsageType::CPUAccessible);

		DescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, static_cast<uint32>(UniformBuffer->GetSize()));

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

	void Material::Update() const
	{
		if (IsDirty)
		{
			// TODO : use frame local allocator for better performance
			auto BufferSize = UniformBuffer->GetSize();
			auto* MappedMemory = static_cast<uint8*>(UniformBuffer->Map());
			HERMES_ASSERT(MappedMemory);
			for (const auto& Property : Properties)
			{
				auto PropertySize = GetMaterialPropertySize(Property.Type);
				HERMES_ASSERT(Property.Offset + PropertySize <= BufferSize);
				memcpy(MappedMemory + Property.Offset, &Property.Value, PropertySize);
			}
			UniformBuffer->Unmap();
		}
	}

	const RenderInterface::DescriptorSet& Material::GetMaterialDescriptorSet() const
	{
		return *DescriptorSet;
	}

	const RenderInterface::Pipeline& Material::GetPipeline() const
	{
		return *Pipeline;
	}

	// TODO : this is very *very* bare bones implementation, without dealing with offsets, alignment etc.
	size_t Material::CalculateUniformBufferSize() const
	{
		size_t Result = 0;

		for (const auto& Property : Properties)
		{
			Result += GetMaterialPropertySize(Property.Type);
		}

		return Result;
	}
}
