#include "Scene.h"

#include "AssetSystem/AssetLoader.h"
#include "AssetSystem/ImageAsset.h"
#include "Logging/Logger.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderInterface/GenericRenderInterface/Device.h"
#include "RenderInterface/GenericRenderInterface/Fence.h"
#include "RenderInterface/GenericRenderInterface/Queue.h"

namespace Hermes
{
	static std::unique_ptr<CubemapTexture> ComputeIrradianceCubemap(const CubemapTexture& Source)
	{
		const Vec2ui Dimensions = { 16 };

		static constexpr std::array CubemapSides =
		{
			RenderInterface::CubemapSide::PositiveX, RenderInterface::CubemapSide::NegativeX,
			RenderInterface::CubemapSide::PositiveY, RenderInterface::CubemapSide::NegativeY,
			RenderInterface::CubemapSide::PositiveZ, RenderInterface::CubemapSide::NegativeZ
		};

		struct PushConstants
		{
			Mat4 MVP;
		};

		std::unique_ptr<CubemapTexture> Result = CubemapTexture::CreateEmpty(Dimensions,
		                                                                     RenderInterface::DataFormat::R32G32B32A32SignedFloat,
		                                                                     RenderInterface::ImageUsageType::ColorAttachment
		                                                                     | RenderInterface::ImageUsageType::Sampled,
		                                                                     1);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& Queue = Device.GetQueue(RenderInterface::QueueType::Render);

		auto CommandBuffer = Queue.CreateCommandBuffer(true);

		auto VertexShader = Device.CreateShader(L"Shaders/Bin/render_uniform_cube.glsl.spv",
		                                        RenderInterface::ShaderType::VertexShader);
		auto FragmentShader = Device.CreateShader(L"Shaders/Bin/irradiance_convolution.glsl.spv",
		                                          RenderInterface::ShaderType::FragmentShader);

		RenderInterface::SamplerDescription SamplerDesc = {};
		SamplerDesc.AddressingModeU = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		SamplerDesc.MinificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.MagnificationFilteringMode = RenderInterface::FilteringMode::Linear;
		SamplerDesc.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		SamplerDesc.MipMode = RenderInterface::MipmappingMode::Linear;
		SamplerDesc.MinMipLevel = 0.0f;
		SamplerDesc.MaxMipLevel = static_cast<float>(Source.GetMipLevelsCount());
		SamplerDesc.MipBias = 0.0f;
		auto Sampler = Device.CreateSampler(SamplerDesc);

		RenderInterface::DescriptorBinding SourceTextureBinding = {};
		SourceTextureBinding.Index = 0;
		SourceTextureBinding.DescriptorCount = 1;
		SourceTextureBinding.Shader = RenderInterface::ShaderType::FragmentShader;
		SourceTextureBinding.Type = RenderInterface::DescriptorType::CombinedSampler;
		auto DescriptorLayout = Device.CreateDescriptorSetLayout({ SourceTextureBinding });
		auto DescriptorSet = Renderer::Get().GetDescriptorAllocator().Allocate(*DescriptorLayout);
		DescriptorSet->UpdateWithImageAndSampler(0, 0, Source.GetDefaultView(), *Sampler,
		                                         RenderInterface::ImageLayout::ShaderReadOnlyOptimal);

		RenderInterface::RenderPassAttachment OutputAttachmentDesc = {};
		OutputAttachmentDesc.Type = RenderInterface::AttachmentType::Color;
		OutputAttachmentDesc.LoadOp = RenderInterface::AttachmentLoadOp::Clear;
		OutputAttachmentDesc.StoreOp = RenderInterface::AttachmentStoreOp::Store;
		OutputAttachmentDesc.StencilLoadOp = RenderInterface::AttachmentLoadOp::Undefined;
		OutputAttachmentDesc.StencilStoreOp = RenderInterface::AttachmentStoreOp::Undefined;
		OutputAttachmentDesc.Format = Result->GetDataFormat();
		OutputAttachmentDesc.LayoutAtStart = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		OutputAttachmentDesc.LayoutAtEnd = RenderInterface::ImageLayout::ShaderReadOnlyOptimal;
		auto RenderPass = Device.CreateRenderPass({ OutputAttachmentDesc });

		RenderInterface::PipelineDescription PipelineDesc = {};
		PipelineDesc.PushConstants = { { RenderInterface::ShaderType::VertexShader, 0, sizeof(PushConstants) } };
		PipelineDesc.ShaderStages = { VertexShader.get(), FragmentShader.get() };
		PipelineDesc.DescriptorLayouts = { DescriptorLayout.get() };
		PipelineDesc.InputAssembler.Topology = RenderInterface::TopologyType::TriangleList;
		PipelineDesc.Viewport.Origin = { 0, 0 };
		PipelineDesc.Viewport.Dimensions = Dimensions;
		PipelineDesc.Rasterizer.Direction = RenderInterface::FaceDirection::CounterClockwise;
		PipelineDesc.Rasterizer.Cull = RenderInterface::CullMode::Back;
		PipelineDesc.Rasterizer.Fill = RenderInterface::FillMode::Fill;
		PipelineDesc.DepthStencilStage.IsDepthTestEnabled = false;
		PipelineDesc.DepthStencilStage.IsDepthWriteEnabled = false;
		auto Pipeline = Device.CreatePipeline(*RenderPass, PipelineDesc);

		// NOTE : we must not destroy the image view that is bound to the render target, so we need to store
		//        an owning pointer to it as well
		std::vector<std::pair<std::unique_ptr<RenderInterface::ImageView>, std::unique_ptr<
			                      RenderInterface::RenderTarget>>> RenderTargets;
		RenderInterface::ImageViewDescription ViewDesc = {};
		ViewDesc.BaseMipLevel = 0;
		ViewDesc.MipLevelCount = 1;
		ViewDesc.Aspects = RenderInterface::ImageAspect::Color;
		for (auto CubemapSide : CubemapSides)
		{
			auto View = Result->GetRawImage().CreateCubemapImageView(ViewDesc, CubemapSide);
			auto RenderTarget = Device.CreateRenderTarget(*RenderPass, { View.get() }, Dimensions);
			RenderTargets.emplace_back(std::move(View), std::move(RenderTarget));
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
		RenderInterface::ImageMemoryBarrier Barrier = {};
		Barrier.OldLayout = RenderInterface::ImageLayout::Undefined;
		Barrier.NewLayout = RenderInterface::ImageLayout::ColorAttachmentOptimal;
		Barrier.OperationsThatHaveToEndBefore = RenderInterface::AccessType::None;
		Barrier.OperationsThatCanStartAfter = RenderInterface::AccessType::None;
		Barrier.BaseMipLevel = 0;
		Barrier.MipLevelCount = Result->GetMipLevelsCount();
		Barrier.Side = RenderInterface::CubemapSide::All;
		CommandBuffer->InsertImageMemoryBarrier(Result->GetRawImage(), Barrier,
		                                        RenderInterface::PipelineStage::BottomOfPipe,
		                                        RenderInterface::PipelineStage::TopOfPipe);
		CommandBuffer->BindPipeline(*Pipeline);
		CommandBuffer->BindDescriptorSet(*DescriptorSet, *Pipeline, 0);

		for (size_t SideIndex = 0; SideIndex < CubemapSides.size(); SideIndex++)
		{
			auto Side = CubemapSides[SideIndex];

			PushConstants PushConstantsData = {};
			auto Model = Mat4::Identity();
			Mat4 View;
			switch (Side)
			{
			case RenderInterface::CubemapSide::PositiveX:
				View = Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
				break;
			case RenderInterface::CubemapSide::NegativeX:
				View = Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
				break;
			case RenderInterface::CubemapSide::PositiveY:
				View = Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
				break;
			case RenderInterface::CubemapSide::NegativeY:
				View = Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
				break;
			case RenderInterface::CubemapSide::PositiveZ:
				View = Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });
				break;
			case RenderInterface::CubemapSide::NegativeZ:
				View = Mat4::LookAt(Vec3 { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });
				break;
			default:
				HERMES_ASSERT(false);
				break;
			}
			auto Projection = Mat4::Perspective(Math::HalfPi, 1.0f, 0.1f, 100.0f);
			PushConstantsData.MVP = Projection * View * Model;

			CommandBuffer->BeginRenderPass(*RenderPass, *RenderTargets[SideIndex].second, { { 0.0f, 0.0f, 0.0f, 1.0f } });
			CommandBuffer->UploadPushConstants(*Pipeline, RenderInterface::ShaderType::VertexShader, &PushConstantsData,
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

	Scene::Scene()
	{
		auto LoadCubemap = [](const String& Name)
		{
			auto RawReflectionEnvmapAsset = Asset::As<ImageAsset>(AssetLoader::Load(Name));
			HERMES_ASSERT_LOG(RawReflectionEnvmapAsset, L"Failed to load cubemap %s.", Name.c_str());
			auto RawReflectionEnvmapTexture = Texture::CreateFromAsset(*RawReflectionEnvmapAsset, false, false);
			return CubemapTexture::CreateFromEquirectangularTexture(*RawReflectionEnvmapTexture,
			                                                        RenderInterface::DataFormat::R16G16B16A16SignedFloat,
			                                                        false);
		};

		ReflectionEnvmap = LoadCubemap(L"Textures/envmap");
		IrradianceEnvmap = ComputeIrradianceCubemap(*ReflectionEnvmap);
	}

	void Scene::AddMesh(MeshProxy Proxy)
	{
		Meshes.push_back(std::move(Proxy));
	}

	const std::vector<PointLightProxy>& Scene::GetPointLights() const
	{
		return PointLights;
	}

	const CubemapTexture& Scene::GetReflectionEnvmap() const
	{
		return *ReflectionEnvmap;
	}

	const CubemapTexture& Scene::GetIrradianceEnvmap() const
	{
		return *IrradianceEnvmap;
	}

	const std::vector<MeshProxy>& Scene::GetMeshes() const
	{
		return Meshes;
	}

	void Scene::AddPointLight(PointLightProxy Proxy)
	{
		PointLights.push_back(Proxy);
	}

	void Scene::ChangeActiveCamera(std::shared_ptr<Camera> NewCamera)
	{
		ActiveCamera = std::move(NewCamera);
	}

	Camera& Scene::GetActiveCamera() const
	{
		return *ActiveCamera;
	}
}
