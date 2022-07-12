#include "SkyboxPass.h"

#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetLoader.h"
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
#include "RenderInterface/GenericRenderInterface/Shader.h"
#include "RenderInterface/GenericRenderInterface/Swapchain.h"

namespace Hermes
{
	SkyboxPass::SkyboxPass(std::shared_ptr<RenderInterface::Device> InDevice)
		: Device(std::move(InDevice))
	{
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		RenderInterface::DescriptorBinding CubemapTextureBinding = {};
		CubemapTextureBinding.Index = 0;
		CubemapTextureBinding.DescriptorCount = 1;
		CubemapTextureBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		CubemapTextureBinding.Type = RenderInterface::DescriptorType::CombinedSampler;
		DataDescriptorLayout = Device->CreateDescriptorSetLayout({ CubemapTextureBinding });

		DataDescriptorSet = DescriptorAllocator.Allocate(*DataDescriptorLayout);

		VertexShader = Device->CreateShader(L"Shaders/Bin/skybox_vert.glsl.spv",
		                                    RenderInterface::ShaderType::VertexShader);
		FragmentShader = Device->CreateShader(L"Shaders/Bin/skybox_frag.glsl.spv",
		                                      RenderInterface::ShaderType::FragmentShader);

		RenderInterface::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressingModeU = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		// TODO : recreate when graphics settings change
		SamplerDesc.AnisotropyLevel = Renderer::Get().GetGraphicsSettings().AnisotropyLevel;
		SamplerDesc.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDesc.MagnificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.MinificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.MipMode = RenderInterface::MipmappingMode::Linear;
		EnvmapSampler = Device->CreateSampler(SamplerDesc);

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
	                              const std::vector<std::pair<
		                              const RenderInterface::Image*, const RenderInterface::ImageView*>>&,
	                              const Scene& Scene, bool ResourcesWereRecreated)
	{
		if (ResourcesWereRecreated || !IsPipelineCreated)
		{
			RecreatePipeline(PassInstance);
			IsPipelineCreated = true;
		}

		const auto& Camera = Scene.GetActiveCamera();
		auto FullViewMatrix = Camera.GetViewMatrix();
		auto ViewMatrixWithoutTranslation = Mat3(FullViewMatrix);
		auto ViewMatrix = Mat4(ViewMatrixWithoutTranslation);
		ViewMatrix[3][3] = 1.0f;
		auto ProjectionMatrix = Camera.GetProjectionMatrix();

		auto ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		const auto& ReflectionEnvmap = Scene.GetReflectionEnvmap();
		DataDescriptorSet->UpdateWithImageAndSampler(0, 0, ReflectionEnvmap.GetDefaultView(), *EnvmapSampler,
		                                             RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		CommandBuffer.BindPipeline(*Pipeline);
		CommandBuffer.BindDescriptorSet(*DataDescriptorSet, *Pipeline, 0);
		CommandBuffer.UploadPushConstants(*Pipeline, RenderInterface::ShaderType::VertexShader, &ViewProjectionMatrix,
		                                  sizeof(ViewProjectionMatrix), 0);
		// Drawing 36 vertices without bound vertex buffer because their coordinates
		// are hardcoded in shader code
		CommandBuffer.Draw(36, 1, 0, 0);
	}

	void SkyboxPass::RecreatePipeline(const RenderInterface::RenderPass& Pass)
	{
		RenderInterface::PipelineDescription PipelineDescription = {};

		PipelineDescription.PushConstants = { { RenderInterface::ShaderType::VertexShader, 0, sizeof(Mat4) } };

		PipelineDescription.ShaderStages = { VertexShader.get(), FragmentShader.get() };

		PipelineDescription.DescriptorLayouts = { DataDescriptorLayout.get() };

		PipelineDescription.VertexInput.VertexAttributes = {};
		PipelineDescription.VertexInput.VertexBindings = {};

		PipelineDescription.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;

		PipelineDescription.Viewport.Origin = { 0, 0 };
		PipelineDescription.Viewport.Dimensions = Renderer::Get().GetSwapchain().GetSize();

		PipelineDescription.DepthStencilStage.IsDepthTestEnabled = true;
		PipelineDescription.DepthStencilStage.ComparisonMode = RenderInterface::ComparisonOperator::LessOrEqual;
		PipelineDescription.DepthStencilStage.IsDepthWriteEnabled = false;

		PipelineDescription.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDescription.Rasterizer.Direction = RenderInterface::FaceDirection::CounterClockwise;
		PipelineDescription.Rasterizer.Fill = RenderInterface::FillMode::Fill;

		Pipeline = Device->CreatePipeline(Pass, PipelineDescription);
	}
}
