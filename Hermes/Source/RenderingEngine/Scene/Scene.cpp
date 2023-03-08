#include "Scene.h"

#include <functional>

#include "ApplicationCore/GameLoop.h"
#include "AssetSystem/ImageAsset.h"
#include "Core/Profiling.h"
#include "Math/Frustum.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "Vulkan/CommandBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/Queue.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Sampler.h"

namespace Hermes
{
	static Mat4 LookAtMatrixForCubemapSide(Vulkan::CubemapSide Side)
	{
		switch (Side)
		{
		case Vulkan::CubemapSide::PositiveX:
			return Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		case Vulkan::CubemapSide::NegativeX:
			return Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
		case Vulkan::CubemapSide::PositiveY:
			return Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f });
		case Vulkan::CubemapSide::NegativeY:
			return Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
		case Vulkan::CubemapSide::PositiveZ:
			return Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
		case Vulkan::CubemapSide::NegativeZ:
			return Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
		default:
			HERMES_ASSERT(false)
		}
	}

	static std::unique_ptr<TextureCubeResource> ComputeIrradianceCubemap(const TextureCubeResource& Source)
	{
		constexpr Vec2ui Dimensions = { 16 };

		constexpr std::array CubemapSides =
		{
			Vulkan::CubemapSide::PositiveX, Vulkan::CubemapSide::NegativeX,
			Vulkan::CubemapSide::PositiveY, Vulkan::CubemapSide::NegativeY,
			Vulkan::CubemapSide::PositiveZ, Vulkan::CubemapSide::NegativeZ
		};

		struct PushConstants
		{
			Mat4 MVP;
		};

		std::unique_ptr<TextureCubeResource> Result = TextureCubeResource::CreateEmpty("IrradianceEnvmap", Dimensions, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& Queue = Device.GetQueue(VK_QUEUE_GRAPHICS_BIT);

		auto CommandBuffer = Queue.CreateCommandBuffer(true);

		auto VertexShader = Device.CreateShader("/Shaders/Bin/render_uniform_cube.glsl.spv",
		                                        VK_SHADER_STAGE_VERTEX_BIT);
		auto FragmentShader = Device.CreateShader("/Shaders/Bin/irradiance_convolution.glsl.spv",
		                                          VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDesc.MinificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MagnificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDesc.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = static_cast<float>(Source.GetMipLevelsCount());
		SamplerDesc.LODBias = 0.0f;
		auto Sampler = Device.CreateSampler(SamplerDesc);

		VkDescriptorSetLayoutBinding SourceTextureBinding = {};
		SourceTextureBinding.binding = 0;
		SourceTextureBinding.descriptorCount = 1;
		SourceTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		SourceTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		auto DescriptorLayout = Device.CreateDescriptorSetLayout({ SourceTextureBinding });
		auto DescriptorSet = Renderer::Get().GetDescriptorAllocator().Allocate(*DescriptorLayout);
		DescriptorSet->UpdateWithImageAndSampler(0, 0, Source.GetView(), *Sampler,
		                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkAttachmentDescription OutputAttachmentDesc = {};
		OutputAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		OutputAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		OutputAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		OutputAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		OutputAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		OutputAttachmentDesc.format = Result->GetDataFormat();
		OutputAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		OutputAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		auto RenderPass = Device.CreateRenderPass({
			                                          std::make_pair(OutputAttachmentDesc,
			                                                         Vulkan::AttachmentType::Color)
		                                          });

		Vulkan::PipelineDescription PipelineDesc = {};
		PipelineDesc.PushConstants = { { VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants) } };
		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDesc.DescriptorSetLayouts = { DescriptorLayout.get() };
		PipelineDesc.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		PipelineDesc.Viewport.x = 0;
		PipelineDesc.Viewport.y = 0;
		PipelineDesc.Viewport.width = static_cast<float>(Dimensions.X);
		PipelineDesc.Viewport.height = static_cast<float>(Dimensions.Y);
		PipelineDesc.Viewport.minDepth = 0.0f;
		PipelineDesc.Viewport.maxDepth = 1.0f;
		PipelineDesc.Scissor.offset = { 0, 0 };
		PipelineDesc.Scissor.extent = { Dimensions.X, Dimensions.Y };
		PipelineDesc.FaceDirection = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		PipelineDesc.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineDesc.PolygonMode = VK_POLYGON_MODE_FILL;
		PipelineDesc.IsDepthTestEnabled = false;
		PipelineDesc.IsDepthWriteEnabled = false;
		auto Pipeline = Device.CreatePipeline(*RenderPass, PipelineDesc);

		// NOTE : we must not destroy the image view that is bound to the render target, so we need to store
		//        an owning pointer to it as well
		std::vector<std::pair<std::unique_ptr<Vulkan::ImageView>, std::unique_ptr<
			                      Vulkan::Framebuffer>>> Framebuffers;
		for (auto CubemapSide : CubemapSides)
		{
			VkImageSubresourceRange ViewSubresource = {};
			ViewSubresource.aspectMask = Result->GetRawImage().GetFullAspectMask();
			ViewSubresource.baseArrayLayer = CubemapSideToArrayLayer(CubemapSide);
			ViewSubresource.layerCount = 1;
			ViewSubresource.baseMipLevel = 0;
			ViewSubresource.levelCount = 1;
			auto View = Result->GetRawImage().CreateImageView(ViewSubresource);
			auto Framebuffer = Device.CreateFramebuffer(*RenderPass, { View.get() }, Dimensions);
			Framebuffers.emplace_back(std::move(View), std::move(Framebuffer));
		}

		/*
		 * Steps to take:
		 *   1) Transfer layout of the resulting cubemap from undefined (which it has after it was created) to
		 *      color attachment optimal
		 *	 2) Bind pipeline and descriptor set
		 *	 3) Repeat 6 times:
		 *	    a) Begin render pass
		 *	    b) Upload push constants with the MVP matrix for given cubemap face
		 *		c) Issue draw call
		 *		d) End render pass
		 *	 4) Transfer layout to the cubemap from color attachment optimal to shader readonly optimal (to be used
		 *	    as a texture). This is done by the implicit render pass attachment layout transition, so we don't
		 *	    need to insert any more barriers
		 */
		CommandBuffer->BeginRecording();
		VkImageMemoryBarrier Barrier = {};
		Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		Barrier.image = Result->GetRawImage().GetImage();
		Barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Barrier.srcAccessMask = VK_ACCESS_NONE;
		Barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		Barrier.subresourceRange = Result->GetRawImage().GetFullSubresourceRange();
		CommandBuffer->InsertImageMemoryBarrier(Barrier, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CommandBuffer->BindPipeline(*Pipeline);
		CommandBuffer->BindDescriptorSet(*DescriptorSet, *Pipeline, 0);

		for (size_t SideIndex = 0; SideIndex < CubemapSides.size(); SideIndex++)
		{
			auto Side = CubemapSides[SideIndex];

			PushConstants PushConstantsData = {};
			auto Model = Mat4::Identity();
			auto View = LookAtMatrixForCubemapSide(Side);
			auto Projection = Mat4::Perspective(Math::HalfPi, 1.0f, 0.1f, 100.0f);
			PushConstantsData.MVP = Projection * View * Model;

			VkClearValue ClearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
			CommandBuffer->BeginRenderPass(*RenderPass, *Framebuffers[SideIndex].second, { &ClearValue, 1 });
			CommandBuffer->UploadPushConstants(*Pipeline, VK_SHADER_STAGE_VERTEX_BIT, &PushConstantsData,
			                                   sizeof(PushConstantsData), 0);
			// Draw 36 vertices for 6 cube faces that are defined in the vertex shader
			CommandBuffer->Draw(36, 1, 0, 0);

			CommandBuffer->EndRenderPass();
		}

		CommandBuffer->EndRecording();

		auto Fence = Device.CreateFence();
		Queue.SubmitCommandBuffer(*CommandBuffer, Fence.get());
		Fence->Wait(UINT64_MAX);

		return Result;
	}

	static std::unique_ptr<TextureCubeResource> ComputeSpecularEnvmap(const TextureCubeResource& Source)
	{
		constexpr uint32 MipLevelCount = 5;
		constexpr Vec2ui Dimensions { 256 };
		constexpr std::array CubemapSides =
		{
			Vulkan::CubemapSide::PositiveX, Vulkan::CubemapSide::NegativeX,
			Vulkan::CubemapSide::PositiveY, Vulkan::CubemapSide::NegativeY,
			Vulkan::CubemapSide::PositiveZ, Vulkan::CubemapSide::NegativeZ
		};

		struct VertexShaderPushConstants
		{
			Mat4 MVP;
		};

		struct FragmentShaderPushConstants
		{
			float RoughnessLevel;
		};

		auto Result = TextureCubeResource::CreateEmpty("SpecularEnvmap", Dimensions, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, MipLevelCount);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& Queue = Device.GetQueue(VK_QUEUE_GRAPHICS_BIT);

		auto CommandBuffer = Queue.CreateCommandBuffer(true);

		auto VertexShader = Device.CreateShader("/Shaders/Bin/render_uniform_cube.glsl.spv",
		                                        VK_SHADER_STAGE_VERTEX_BIT);
		auto FragmentShader = Device.CreateShader("/Shaders/Bin/specular_prefilter.glsl.spv",
		                                          VK_SHADER_STAGE_FRAGMENT_BIT);

		Vulkan::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerDesc.MinificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.MagnificationFilter = VK_FILTER_LINEAR;
		SamplerDesc.CoordinateSystem = Vulkan::CoordinateSystem::Normalized;
		SamplerDesc.MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = static_cast<float>(Source.GetMipLevelsCount());
		SamplerDesc.LODBias = 0.0f;
		auto Sampler = Device.CreateSampler(SamplerDesc);

		VkDescriptorSetLayoutBinding SourceTextureBinding = {};
		SourceTextureBinding.binding = 0;
		SourceTextureBinding.descriptorCount = 1;
		SourceTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		SourceTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		auto DescriptorLayout = Device.CreateDescriptorSetLayout({ SourceTextureBinding });
		auto DescriptorSet = Renderer::Get().GetDescriptorAllocator().Allocate(*DescriptorLayout);
		DescriptorSet->UpdateWithImageAndSampler(0, 0, Source.GetView(), *Sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkAttachmentDescription OutputAttachmentDesc = {};
		OutputAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		OutputAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		OutputAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		OutputAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		OutputAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		OutputAttachmentDesc.format = Result->GetDataFormat();
		OutputAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		OutputAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		auto RenderPass = Device.CreateRenderPass({
			                                          std::make_pair(OutputAttachmentDesc,
			                                                         Vulkan::AttachmentType::Color)
		                                          });

		Vulkan::PipelineDescription PipelineDesc = {};
		VkPushConstantRange VertexShaderPushConstantsRange = {};
		VertexShaderPushConstantsRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		VertexShaderPushConstantsRange.offset = 0;
		VertexShaderPushConstantsRange.size = sizeof(VertexShaderPushConstants);
		VkPushConstantRange FragmentShaderPushConstantsRange = {};
		FragmentShaderPushConstantsRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragmentShaderPushConstantsRange.offset = sizeof(VertexShaderPushConstants);
		FragmentShaderPushConstantsRange.size = sizeof(FragmentShaderPushConstants);
		PipelineDesc.PushConstants = { VertexShaderPushConstantsRange, FragmentShaderPushConstantsRange };
		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDesc.DescriptorSetLayouts = { DescriptorLayout.get() };
		PipelineDesc.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		PipelineDesc.Viewport.x = 0;
		PipelineDesc.Viewport.y = 0;
		PipelineDesc.Scissor.offset = { 0, 0 };
		PipelineDesc.FaceDirection = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		PipelineDesc.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineDesc.PolygonMode = VK_POLYGON_MODE_FILL;
		PipelineDesc.IsDepthTestEnabled = false;
		PipelineDesc.IsDepthWriteEnabled = false;

		std::vector<std::unique_ptr<Vulkan::Pipeline>> Pipelines;
		auto CurrentMipDimensions = Dimensions;
		for (uint32 MipLevel = 0; MipLevel < MipLevelCount; MipLevel++)
		{
			PipelineDesc.Viewport.width = static_cast<float>(CurrentMipDimensions.X);
			PipelineDesc.Viewport.height = static_cast<float>(CurrentMipDimensions.Y);
			PipelineDesc.Scissor.extent = { CurrentMipDimensions.X, CurrentMipDimensions.Y };
			auto Pipeline = Device.CreatePipeline(*RenderPass, PipelineDesc);
			Pipelines.push_back(std::move(Pipeline));
			CurrentMipDimensions /= 2;
		}

		// NOTE : we must not destroy the image view that is bound to the render target, so we need to store
		//        an owning pointer to it as well
		std::vector<std::pair<std::unique_ptr<Vulkan::ImageView>, std::unique_ptr<
			                      Vulkan::Framebuffer>>> Framebuffers;
		VkImageSubresourceRange Subresource = {};
		Subresource.levelCount = 1;
		Subresource.layerCount = 1;
		Subresource.aspectMask = Result->GetRawImage().GetFullAspectMask();
		auto MipLevelDimensions = Dimensions;
		for (uint32 MipLevel = 0; MipLevel < MipLevelCount; MipLevel++)
		{
			Subresource.baseMipLevel = MipLevel;
			for (auto CubemapSide : CubemapSides)
			{
				Subresource.baseArrayLayer = CubemapSideToArrayLayer(CubemapSide);
				auto View = Result->GetRawImage().CreateImageView(Subresource);
				auto Framebuffer = Device.CreateFramebuffer(*RenderPass, { View.get() }, MipLevelDimensions);
				Framebuffers.emplace_back(std::move(View), std::move(Framebuffer));
			}
			MipLevelDimensions /= 2;
		}

		/*
		 * Steps to take:
		 *   1) Transfer layout of the resulting cubemap from undefined (which it has after it was created) to
		 *      color attachment optimal
		 *	 2) Bind pipeline and descriptor set
		 *	 3) Repeat MipLevelCount * 6 times:
		 *	    a) Begin render pass
		 *	    b) Upload push constants with the MVP matrix for given cubemap face and the roughness level
		 *		c) Issue draw call
		 *		d) End render pass
		 *	 4) Transfer layout to the cubemap from color attachment optimal to shader readonly optimal (to be used
		 *	    as a texture). This is done by the implicit render pass attachment layout transition, so we don't
		 *	    need to insert any more barriers
		 */
		CommandBuffer->BeginRecording();
		VkImageMemoryBarrier Barrier = {};
		Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		Barrier.image = Result->GetRawImage().GetImage();
		Barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Barrier.srcAccessMask = VK_ACCESS_NONE;
		Barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		Barrier.subresourceRange = Result->GetRawImage().GetFullSubresourceRange();
		CommandBuffer->InsertImageMemoryBarrier(Barrier, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		for (uint32 MipLevel = 0; MipLevel < MipLevelCount; MipLevel++)
		{
			float Roughness = static_cast<float>(MipLevel) / static_cast<float>(MipLevelCount - 1);

			const auto& Pipeline = *Pipelines[MipLevel];
			CommandBuffer->BindPipeline(Pipeline);
			CommandBuffer->BindDescriptorSet(*DescriptorSet, Pipeline, 0);
			for (size_t SideIndex = 0; SideIndex < CubemapSides.size(); SideIndex++)
			{
				auto Side = CubemapSides[SideIndex];

				VertexShaderPushConstants VertexPushConstants = {};
				auto Model = Mat4::Identity();
				auto View = LookAtMatrixForCubemapSide(Side);
				auto Projection = Mat4::Perspective(Math::HalfPi, 1.0f, 0.1f, 100.0f);
				VertexPushConstants.MVP = Projection * View * Model;

				FragmentShaderPushConstants FragmentPushConstants = {};
				FragmentPushConstants.RoughnessLevel = Roughness;

				size_t FramebufferIndex = MipLevel * CubemapSides.size() + SideIndex;
				VkClearValue ClearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
				CommandBuffer->BeginRenderPass(*RenderPass, *Framebuffers[FramebufferIndex].second,
				                               { &ClearValue, 1 });
				CommandBuffer->UploadPushConstants(Pipeline, VK_SHADER_STAGE_VERTEX_BIT,
				                                   &VertexPushConstants, sizeof(VertexPushConstants), 0);
				CommandBuffer->UploadPushConstants(Pipeline, VK_SHADER_STAGE_FRAGMENT_BIT,
				                                   &FragmentPushConstants, sizeof(FragmentPushConstants),
				                                   sizeof(VertexPushConstants));
				// Draw 36 vertices for 6 cube faces that are defined in the vertex shader
				CommandBuffer->Draw(36, 1, 0, 0);

				CommandBuffer->EndRenderPass();
			}
		}

		CommandBuffer->EndRecording();

		auto Fence = Device.CreateFence();
		Queue.SubmitCommandBuffer(*CommandBuffer, Fence.get());
		Fence->Wait(UINT64_MAX);

		return Result;
	}

	Scene::Scene()
	{
		static constexpr auto EnvmapName = "/Textures/envmap";

		auto& AssetCache = GGameLoop->GetAssetCache();
		auto RawReflectionEnvmapAsset = AssetCache.Get<ImageAsset>(EnvmapName);
		HERMES_ASSERT_LOG(RawReflectionEnvmapAsset && RawReflectionEnvmapAsset.value()->GetResource() && RawReflectionEnvmapAsset.value()->GetResource()->GetType() == ResourceType::Texture2D, "Cannot load envmap %s", EnvmapName);

		const auto* RawReflectionEnvmap = static_cast<const Texture2DResource*>(RawReflectionEnvmapAsset.value()->GetResource());
		ReflectionEnvmap = TextureCubeResource::CreateFromEquirectangularTexture("ReflectionEnvmap", *RawReflectionEnvmap, VK_FORMAT_R16G16B16A16_SFLOAT, true);
		IrradianceEnvmap = ComputeIrradianceCubemap(*ReflectionEnvmap);
		SpecularEnvmap = ComputeSpecularEnvmap(*ReflectionEnvmap);
	}

	SceneNode& Scene::GetRootNode()
	{
		return RootNode;
	}

	const SceneNode& Scene::GetRootNode() const
	{
		return RootNode;
	}

	void Scene::Reset()
	{
		HERMES_PROFILE_FUNC();

		while (RootNode.GetChildrenCount() > 0)
			RootNode.RemoveChild(0);
		RootNode.SetLocalTransform({});
		ActiveCamera = nullptr;
	}
	
	const TextureCubeResource& Scene::GetReflectionEnvmap() const
	{
		return *ReflectionEnvmap;
	}

	const TextureCubeResource& Scene::GetIrradianceEnvmap() const
	{
		return *IrradianceEnvmap;
	}

	const TextureCubeResource& Scene::GetSpecularEnvmap() const
	{
		return *SpecularEnvmap;
	}

	void Scene::ChangeActiveCamera(std::shared_ptr<Camera> NewCamera)
	{
		ActiveCamera = std::move(NewCamera);
	}

	Camera& Scene::GetActiveCamera() const
	{
		return *ActiveCamera;
	}

	GeometryList Scene::BakeGeometryList() const
	{
		HERMES_PROFILE_FUNC();
		std::vector<DrawableMesh> CulledMeshes;

		auto Frustum = GetActiveCamera().GetFrustum();

		std::function<void(const SceneNode&)> MeshTraversal = [&](const SceneNode& CurrentNode) -> void
		{
			for (size_t ChildIndex = 0; ChildIndex < CurrentNode.GetChildrenCount(); ChildIndex++)
				MeshTraversal(CurrentNode.GetChild(ChildIndex));

			if (CurrentNode.GetType() != SceneNodeType::Mesh)
				return;

			const auto& Mesh = static_cast<const MeshNode&>(CurrentNode);
			auto TransformationMatrix = CurrentNode.GetWorldTransformationMatrix();

			if (!Frustum.IsInside(Mesh.GetBoundingVolume(), TransformationMatrix))
				return;

			CulledMeshes.emplace_back(TransformationMatrix , &Mesh.GetMesh(), &Mesh.GetMaterialInstance());
		};
		MeshTraversal(RootNode);

		return GeometryList(std::move(CulledMeshes));
	}
}
