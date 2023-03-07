#include "ForwardPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Material/Material.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/GeometryList.h"
#include "RenderingEngine/Scene/Scene.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Queue.h"
#include "Vulkan/RenderPass.h"

namespace Hermes
{
	std::unique_ptr<Vulkan::Image> ForwardPass::PrecomputedBRDFImage;
	std::unique_ptr<Vulkan::ImageView> ForwardPass::PrecomputedBRDFView;
	std::unique_ptr<Vulkan::Sampler> ForwardPass::PrecomputedBRDFSampler;

	ForwardPass::ForwardPass(bool ReuseDataInDepthBuffer)
	{
		auto& Device = Renderer::Get().GetActiveDevice();

		SceneUBODescriptorSet = Renderer::Get().GetDescriptorAllocator().
		                                        Allocate(Renderer::Get().GetGlobalDataDescriptorSetLayout());

		Vulkan::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDesc.MagnificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MinificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDesc.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = 16.0f; // NOTE : 65536 * 65536 textures is more than enough :)
		SamplerDesc.LODBias = 0.0f;
		EnvmapSampler = Device.CreateSampler(SamplerDesc);

		Description.Callback.Bind<ForwardPass, &ForwardPass::PassCallback>(this);

		Attachment Color = {};
		Color.Name = "Color";
		Color.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		Color.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		Color.ClearColor.color.float32[0] = 1.0f;
		Color.ClearColor.color.float32[1] = 1.0f;
		Color.ClearColor.color.float32[2] = 1.0f;
		Color.ClearColor.color.float32[3] = 1.0f;
		Color.Binding = BindingMode::ColorAttachment;

		Attachment Depth = {};
		Depth.Name = "Depth";
		Depth.LoadOp = ReuseDataInDepthBuffer ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		Depth.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		Depth.ClearColor.depthStencil.depth = 0.0f;
		Depth.Binding = BindingMode::DepthStencilAttachment;
		Description.Attachments = { Color, Depth };

		Description.BufferInputs =
		{
			{ "LightClusterList", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false },
			{ "LightIndexList", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false }
		};
	}

	const PassDesc& ForwardPass::GetPassDescription() const
	{
		return Description;
	}

	void ForwardPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();
		if (!PrecomputedBRDFSampler || !PrecomputedBRDFView || !PrecomputedBRDFSampler)
		{
			EnsurePrecomputedBRDF();
		}

		auto& CommandBuffer = CallbackInfo.CommandBuffer;
		auto& Metrics = CallbackInfo.Metrics;
		const auto& Scene = CallbackInfo.Scene;

		const auto& GlobalSceneDataBuffer = Renderer::Get().GetGlobalSceneDataBuffer();
		const auto& LightClusterListBuffer = *std::get<const Vulkan::Buffer*>(CallbackInfo.Resources.at("LightClusterList"));
		const auto& LightIndexListBuffer = *std::get<const Vulkan::Buffer*>(CallbackInfo.Resources.at("LightIndexList"));
		SceneUBODescriptorSet->UpdateWithBuffer(0, 0, GlobalSceneDataBuffer, 0, static_cast<uint32>(GlobalSceneDataBuffer.GetSize()));
		SceneUBODescriptorSet->UpdateWithImageAndSampler(1, 0, Scene.GetIrradianceEnvmap().GetView(ColorSpace::Linear),
		                                                 *EnvmapSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		SceneUBODescriptorSet->UpdateWithImageAndSampler(2, 0, Scene.GetSpecularEnvmap().GetView(ColorSpace::Linear),
		                                                 *EnvmapSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		SceneUBODescriptorSet->UpdateWithImageAndSampler(3, 0, *PrecomputedBRDFView, *PrecomputedBRDFSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		SceneUBODescriptorSet->UpdateWithBuffer(4, 0, LightClusterListBuffer, 0, static_cast<uint32>(LightClusterListBuffer.GetSize()));
		SceneUBODescriptorSet->UpdateWithBuffer(5, 0, LightIndexListBuffer, 0, static_cast<uint32>(LightIndexListBuffer.GetSize()));

		for (const auto& Mesh : CallbackInfo.GeometryList.GetMeshList())
		{
			auto& Material = Mesh.Material;
			auto& MaterialPipeline = Material->GetBaseMaterial().GetPipeline();
			auto& MeshBuffer = Mesh.MeshBuffer;

			// TODO : move it into the renderer?
			Material->PrepareForRender();

			CommandBuffer.BindPipeline(MaterialPipeline);
			Metrics.PipelineBindCount++;
			CommandBuffer.BindDescriptorSet(*SceneUBODescriptorSet, MaterialPipeline, 0);
			Metrics.DescriptorSetBindCount++;
			CommandBuffer.BindDescriptorSet(Material->GetMaterialDescriptorSet(), MaterialPipeline, 1);
			Metrics.DescriptorSetBindCount++;
			CommandBuffer.BindVertexBuffer(MeshBuffer->GetVertexBuffer());
			Metrics.BufferBindCount++;
			CommandBuffer.BindIndexBuffer(MeshBuffer->GetIndexBuffer(), VK_INDEX_TYPE_UINT32);
			Metrics.BufferBindCount++;

			auto TransformationMatrix = Mesh.TransformationMatrix;

			CommandBuffer.UploadPushConstants(MaterialPipeline, VK_SHADER_STAGE_VERTEX_BIT,
			                                  &TransformationMatrix, sizeof(TransformationMatrix), 0);

			for (const auto& Primitive : MeshBuffer->GetPrimitives())
			{
				CommandBuffer.DrawIndexed(Primitive.IndexCount, 1, Primitive.IndexOffset, 0, 0);
				Metrics.DrawCallCount++;
			}
		}
	}

	void ForwardPass::EnsurePrecomputedBRDF()
	{
		static constexpr Vec2ui Dimensions { 512 };
		static constexpr VkFormat Format = VK_FORMAT_R16G16_SFLOAT;

		// First, destroy the previous sampler and BRDF image
		PrecomputedBRDFImage.reset();
		PrecomputedBRDFView.reset();
		PrecomputedBRDFSampler.reset();

		// Compute the BRDF
		auto& Device = Renderer::Get().GetActiveDevice();

		PrecomputedBRDFImage = Device.CreateImage(Dimensions,
		                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		                                          Format, 1);
		PrecomputedBRDFView = PrecomputedBRDFImage->CreateDefaultImageView();

		VkAttachmentDescription OutputAttachment = {};
		OutputAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		OutputAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		OutputAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		OutputAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		OutputAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		OutputAttachment.format = PrecomputedBRDFImage->GetDataFormat();
		OutputAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		OutputAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		auto RenderPass = Device.CreateRenderPass({ { OutputAttachment, Vulkan::AttachmentType::Color } });

		auto VertexShader = Device.CreateShader("/Shaders/Bin/fs_vert.glsl.spv", VK_SHADER_STAGE_VERTEX_BIT);
		auto FragmentShader = Device.CreateShader("/Shaders/Bin/precompute_brdf.glsl.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::PipelineDescription PipelineDesc = {};
		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDesc.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		PipelineDesc.Viewport.x = 0;
		PipelineDesc.Viewport.y = 0;
		PipelineDesc.Viewport.width = static_cast<float>(PrecomputedBRDFImage->GetDimensions().X);
		PipelineDesc.Viewport.height = static_cast<float>(PrecomputedBRDFImage->GetDimensions().Y);
		PipelineDesc.Viewport.minDepth = 0.0f;
		PipelineDesc.Viewport.maxDepth = 1.0f;
		PipelineDesc.Scissor.offset = { 0, 0 };
		PipelineDesc.Scissor.extent = {
			PrecomputedBRDFImage->GetDimensions().X, PrecomputedBRDFImage->GetDimensions().Y
		};
		PipelineDesc.PolygonMode = VK_POLYGON_MODE_FILL;
		PipelineDesc.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineDesc.FaceDirection = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		PipelineDesc.IsDepthTestEnabled = false;
		PipelineDesc.IsDepthWriteEnabled = false;
		auto Pipeline = Device.CreatePipeline(*RenderPass, PipelineDesc);

		auto Framebuffer = Device.CreateFramebuffer(*RenderPass, { PrecomputedBRDFView.get() },
		                                            PrecomputedBRDFImage->GetDimensions());

		auto& Queue = Device.GetQueue(VK_QUEUE_GRAPHICS_BIT);
		auto CommandBuffer = Queue.CreateCommandBuffer(true);
		auto Fence = Device.CreateFence();

		/*
		 * NOTE : steps to take:
		 *   1) Transition the precomputed BRDF image to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		 *	 2) Begin render pass
		 *	 3) Bind pipeline
		 *	 4) Issue draw call for 2 triangles (6 vertices)
		 *	 5) End render pass/recording
		 */
		CommandBuffer->BeginRecording();
		VkImageMemoryBarrier Barrier = {};
		Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		Barrier.srcAccessMask = VK_ACCESS_NONE;
		Barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		Barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Barrier.image = PrecomputedBRDFImage->GetImage();
		Barrier.subresourceRange = PrecomputedBRDFImage->GetFullSubresourceRange();
		CommandBuffer->InsertImageMemoryBarrier(Barrier, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		VkClearValue ClearValue = {};
		ClearValue.color.float32[0] = 0.0f;
		ClearValue.color.float32[1] = 0.0f;
		ClearValue.color.float32[2] = 0.0f;
		ClearValue.color.float32[3] = 1.0f;
		CommandBuffer->BeginRenderPass(*RenderPass, *Framebuffer, { &ClearValue, 1 });
		CommandBuffer->BindPipeline(*Pipeline);
		CommandBuffer->Draw(6, 1, 0, 0);
		CommandBuffer->EndRenderPass();
		CommandBuffer->EndRecording();

		Queue.SubmitCommandBuffer(*CommandBuffer, Fence.get());

		Vulkan::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDesc.MinificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MagnificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDesc.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = 1.0f;
		SamplerDesc.LODBias = 0.0f;
		PrecomputedBRDFSampler = Device.CreateSampler(SamplerDesc);

		Fence->Wait(UINT64_MAX);
	}
}
