#include "SkyboxPass.h"

#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetLoader.h"
#include "Math/Common.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Texture.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Image.h"
#include "RenderInterface/GenericRenderInterface/Pipeline.h"
#include "RenderInterface/GenericRenderInterface/RenderPass.h"
#include "RenderInterface/GenericRenderInterface/Sampler.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

namespace Hermes
{
	SkyboxPass::SkyboxPass(std::shared_ptr<RenderInterface::Device> InDevice)
		: Device(std::move(InDevice))
	{
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		RenderInterface::DescriptorBinding UniformBufferBinding = {};
		UniformBufferBinding.Index = 0;
		UniformBufferBinding.DescriptorCount = 1;
		UniformBufferBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		UniformBufferBinding.Type = RenderInterface::DescriptorType::UniformBuffer;
		RenderInterface::DescriptorBinding SkyboxTextureBinding = {};
		SkyboxTextureBinding.Index = 1;
		SkyboxTextureBinding.DescriptorCount = 1;
		SkyboxTextureBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		SkyboxTextureBinding.Type = RenderInterface::DescriptorType::CombinedSampler;
		DataDescriptorLayout = Device->CreateDescriptorSetLayout({
			UniformBufferBinding, SkyboxTextureBinding
		});

		DataDescriptorSet = DescriptorAllocator.Allocate(DataDescriptorLayout);

		VertexShader = Device->CreateShader(L"Shaders/Bin/skybox_vert.glsl.spv",
		                                    RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device->CreateShader(L"Shaders/Bin/skybox_frag.glsl.spv",
		                                      RenderInterface::ShaderType::FragmentShader);

		EnvmapAsset = Asset::As<ImageAsset>(AssetLoader::Load(L"Textures/default_envmap_reflection"));
		HERMES_ASSERT_LOG(EnvmapAsset, L"Failed to load reflection envmap.");

		UniformBuffer = Device->CreateBuffer(sizeof(SkyboxPassData),
		                                     RenderInterface::BufferUsageType::UniformBuffer |
		                                     RenderInterface::BufferUsageType::CPUAccessible);
		EnvmapTexture = Texture::CreateFromAsset(*EnvmapAsset, false);

		RenderInterface::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressingModeU = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.AnisotropyLevel = Renderer::Get().GetGraphicsSettings().AnisotropyLevel;
		SamplerDesc.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDesc.MagnificationFilteringMode = RenderInterface::FilteringMode::Nearest;
		EnvmapSampler = Device->CreateSampler(SamplerDesc);

		DataDescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, sizeof(SkyboxPassData));
		DataDescriptorSet->UpdateWithImageAndSampler(1, 0, EnvmapTexture->GetRawImage(), *EnvmapSampler,
		                                             RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		Description.Callback.Bind<SkyboxPass, &SkyboxPass::PassCallback>(this);

		Description.Sources.push_back({ L"ColorBuffer" });

		Description.Drains.resize(2);
		Description.Drains[0].Name = L"ColorBuffer";
		Description.Drains[0].Binding = BindingMode::ColorAttachment;
		Description.Drains[0].Layout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		Description.Drains[0].LoadOp = RenderInterface::AttachmentLoadOp::Load;
		Description.Drains[0].StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;

		Description.Drains[1].Name = L"DepthBuffer";
		Description.Drains[1].Binding = BindingMode::DepthStencilAttachment;
		Description.Drains[1].Layout = RenderInterface::ImageLayout::DepthStencilReadOnlyOptimal;
		Description.Drains[1].LoadOp = RenderInterface::AttachmentLoadOp::Load;
		Description.Drains[1].StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
	}

	const PassDesc& SkyboxPass::GetPassDescription() const
	{
		return Description;
	}

	void SkyboxPass::PassCallback(RenderInterface::CommandBuffer& CommandBuffer,
	                              const RenderInterface::RenderPass& PassInstance,
	                              const std::vector<const RenderInterface::Image*>& Drains,
	                              const Scene& Scene, bool ResourcesWereRecreated)
	{
		if (ResourcesWereRecreated || !IsPipelineCreated)
		{
			RecreatePipeline(PassInstance);
			IsPipelineCreated = true;
		}

		SkyboxPassData Data;
		Data.ViewMatrix = Scene.GetActiveCamera().GetViewMatrix();
		Data.ViewportDimensions = Vec2(Renderer::Get().GetSwapchain().GetSize());
		Data.HalfVerticalFOV = Math::Radians(Scene.GetActiveCamera().GetVerticalFOV() / 2.0f);
		Data.AspectRatio = static_cast<float>(Drains[0]->GetSize().X) / static_cast<float>(Drains[0]->GetSize().Y);
		auto* Memory = UniformBuffer->Map();
		memcpy(Memory, &Data, sizeof(Data));
		UniformBuffer->Unmap();

		CommandBuffer.BindPipeline(*Pipeline);
		CommandBuffer.BindDescriptorSet(*DataDescriptorSet, *Pipeline, 0);
		// Drawing 6 vertices without bound vertex buffer because their coordinates
		// are hardcoded in shader code
		CommandBuffer.Draw(6, 1, 0, 0);
	}

	void SkyboxPass::RecreatePipeline(const RenderInterface::RenderPass& Pass)
	{
		RenderInterface::PipelineDescription PipelineDescription = {};

		PipelineDescription.ShaderStages = { VertexShader, FragmentShader };

		PipelineDescription.DescriptorLayouts = { DataDescriptorLayout };

		PipelineDescription.VertexInput.VertexAttributes = {};
		PipelineDescription.VertexInput.VertexBindings = {};

		PipelineDescription.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;

		PipelineDescription.Viewport.Origin = { 0, 0 };
		PipelineDescription.Viewport.Dimensions = Renderer::Get().GetSwapchain().GetSize();

		PipelineDescription.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDescription.DepthStencilStage.ComparisonMode = RenderInterface::ComparisonOperator::Equal;
		PipelineDescription.DepthStencilStage.IsDepthWriteEnabled = false;

		PipelineDescription.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDescription.Rasterizer.Direction = RenderInterface::FaceDirection::Clockwise;
		PipelineDescription.Rasterizer.Fill = RenderInterface::FillMode::Fill;

		Pipeline = Device->CreatePipeline(Pass, PipelineDescription);
	}
}
