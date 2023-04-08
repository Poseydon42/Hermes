#include "SkyboxPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderingEngine/Texture.h"
#include "Vulkan/Descriptor.h"
#include "Vulkan/Device.h"
#include "Vulkan/Image.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Sampler.h"
#include "Vulkan/Swapchain.h"

namespace Hermes
{
	SkyboxPass::SkyboxPass()
	{
		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		VkDescriptorSetLayoutBinding CubemapTextureBinding = {};
		CubemapTextureBinding.binding = 0;
		CubemapTextureBinding.descriptorCount = 1;
		CubemapTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		CubemapTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DataDescriptorLayout = Device.CreateDescriptorSetLayout({ CubemapTextureBinding });

		DataDescriptorSet = DescriptorAllocator.Allocate(*DataDescriptorLayout);

		Vulkan::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDesc.AnisotropyLevel = 0.0f;
		SamplerDesc.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDesc.MinificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MagnificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		EnvmapSampler = Device.CreateSampler(SamplerDesc);

		Description.Callback = [this](const PassCallbackInfo& CallbackInfo) { PassCallback(CallbackInfo); };

		Description.Attachments.resize(2);
		Description.Attachments[0].Name = "ColorBuffer";
		Description.Attachments[0].Binding = BindingMode::ColorAttachment;
		Description.Attachments[0].LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		Description.Attachments[0].StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

		Description.Attachments[1].Name = "DepthBuffer";
		Description.Attachments[1].Binding = BindingMode::DepthStencilAttachment;
		Description.Attachments[1].LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		Description.Attachments[1].StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}

	const PassDesc& SkyboxPass::GetPassDescription() const
	{
		return Description;
	}

	void SkyboxPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();
		if (!Pipeline)
			CreatePipeline(*CallbackInfo.RenderPass);

		auto FramebufferDimensions = std::get<const Vulkan::ImageView*>(CallbackInfo.Resources.at("ColorBuffer"))->GetDimensions();
		auto ViewportDimensions = Vec2(FramebufferDimensions);

		auto& CommandBuffer = CallbackInfo.CommandBuffer;
		auto& Metrics = CallbackInfo.Metrics;
		const auto& Scene = CallbackInfo.Scene;

		const auto& Camera = Scene.GetActiveCamera();
		auto FullViewMatrix = Camera.GetViewMatrix();
		auto ViewMatrixWithoutTranslation = Mat3(FullViewMatrix);
		auto ViewMatrix = Mat4(ViewMatrixWithoutTranslation);
		ViewMatrix[3][3] = 1.0f;
		auto ProjectionMatrix = Camera.GetProjectionMatrix();

		auto ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		const auto& ReflectionEnvmap = Scene.GetReflectionEnvmap();
		DataDescriptorSet->UpdateWithImageAndSampler(0, 0, ReflectionEnvmap.GetView(), *EnvmapSampler,
		                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		CommandBuffer.BindPipeline(*Pipeline);
		Metrics.PipelineBindCount++;

		CommandBuffer.SetViewport({ 0.0f, 0.0f, ViewportDimensions.X, ViewportDimensions.Y, 0.0f, 1.0f });
		CommandBuffer.SetScissor({ { 0, 0 }, { FramebufferDimensions.X, FramebufferDimensions.Y } });

		CommandBuffer.BindDescriptorSet(*DataDescriptorSet, *Pipeline, 0);
		Metrics.DescriptorSetBindCount++;
		CommandBuffer.UploadPushConstants(*Pipeline, VK_SHADER_STAGE_VERTEX_BIT, &ViewProjectionMatrix,
		                                  sizeof(ViewProjectionMatrix), 0);
		// Drawing 36 vertices without bound vertex buffer because their coordinates
		// are hardcoded in shader code
		CommandBuffer.Draw(36, 1, 0, 0);
		Metrics.DrawCallCount++;
	}

	void SkyboxPass::CreatePipeline(const Vulkan::RenderPass& Pass)
	{
		auto& ShaderCache = Renderer::Get().GetShaderCache();

		auto& VertexShader = ShaderCache.GetShader("/Shaders/Bin/skybox_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT);
		auto& FragmentShader = ShaderCache.GetShader("/Shaders/Bin/skybox_frag.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::PipelineDescription PipelineDescription = {};

		PipelineDescription.PushConstants = { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4) } };
		PipelineDescription.ShaderStages = { &VertexShader, &FragmentShader };
		PipelineDescription.DescriptorSetLayouts = { DataDescriptorLayout.get() };

		PipelineDescription.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		PipelineDescription.IsDepthTestEnabled = true;
		PipelineDescription.DepthCompareOperator = VK_COMPARE_OP_GREATER_OR_EQUAL;
		PipelineDescription.IsDepthWriteEnabled = false;

		PipelineDescription.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineDescription.FaceDirection = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		PipelineDescription.PolygonMode = VK_POLYGON_MODE_FILL;

		PipelineDescription.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		Pipeline = Renderer::Get().GetActiveDevice().CreatePipeline(Pass, PipelineDescription);
	}
}
