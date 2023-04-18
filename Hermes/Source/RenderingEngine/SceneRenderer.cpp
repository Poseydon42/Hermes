#include "SceneRenderer.h"

#include "Core/Profiling.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/SharedData.h"

namespace Hermes
{
	SceneRenderer::SceneRenderer()
	{
		LightCullingPass = std::make_unique<class LightCullingPass>();
		DepthPass = std::make_unique<class DepthPass>();
		ForwardPass = std::make_unique<class ForwardPass>(true);
		PostProcessingPass = std::make_unique<class PostProcessingPass>();
		SkyboxPass = std::make_unique<class SkyboxPass>();

		FrameGraphScheme Scheme;
		Scheme.AddPass("LightCullingPass", LightCullingPass->GetPassDescription());
		Scheme.AddPass("DepthPass", DepthPass->GetPassDescription());
		Scheme.AddPass("ForwardPass", ForwardPass->GetPassDescription());
		Scheme.AddPass("PostProcessingPass", PostProcessingPass->GetPassDescription());
		Scheme.AddPass("SkyboxPass", SkyboxPass->GetPassDescription());

		ImageResourceDescription HDRColorBufferResource = {};
		HDRColorBufferResource.Dimensions = ViewportRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		HDRColorBufferResource.Format = VK_FORMAT_R16G16B16A16_SFLOAT;
		HDRColorBufferResource.MipLevels = 1;
		Scheme.AddResource("HDRColorBuffer", HDRColorBufferResource, false);

		ImageResourceDescription ColorBufferResource = {};
		ColorBufferResource.Dimensions = ViewportRelativeDimensions::CreateFromRelativeDimensions({ 1.0f });
		ColorBufferResource.Format = VK_FORMAT_B8G8R8A8_SRGB;
		ColorBufferResource.MipLevels = 1;
		Scheme.AddResource("ColorBuffer", ColorBufferResource, false);

		ImageResourceDescription DepthBufferResource = {};
		DepthBufferResource.Dimensions = ViewportRelativeDimensions::CreateFromRelativeDimensions({ 1.0f, 1.0f });
		DepthBufferResource.Format = VK_FORMAT_D32_SFLOAT;
		DepthBufferResource.MipLevels = 1;
		Scheme.AddResource("DepthBuffer", DepthBufferResource, false);

		BufferResourceDescription LightClusterListResource = {};
		// FIXME: find a way to compute this value instead of guessing
		LightClusterListResource.Size = 4 * 1024 * 1024;
		Scheme.AddResource("LightClusterList", LightClusterListResource, false);

		BufferResourceDescription LightIndexListResource = {};
		LightIndexListResource.Size = 64 * 1024 * 1024; // FIXME: see above
		Scheme.AddResource("LightIndexList", LightIndexListResource, false);

		BufferResourceDescription SceneDataResource =
		{
			.Size = sizeof(SceneData)
		};
		Scheme.AddResource("SceneData", SceneDataResource, true);

		Scheme.AddLink("$.LightClusterList", "LightCullingPass.LightClusterList");
		Scheme.AddLink("$.LightIndexList", "LightCullingPass.LightIndexList");
		Scheme.AddLink("$.SceneData", "LightCullingPass.SceneData");

		Scheme.AddLink("$.DepthBuffer", "DepthPass.Depth");
		Scheme.AddLink("$.SceneData", "DepthPass.SceneData");

		Scheme.AddLink("$.HDRColorBuffer", "ForwardPass.Color");
		Scheme.AddLink("DepthPass.Depth", "ForwardPass.Depth");
		Scheme.AddLink("LightCullingPass.LightClusterList", "ForwardPass.LightClusterList");
		Scheme.AddLink("LightCullingPass.LightIndexList", "ForwardPass.LightIndexList");
		Scheme.AddLink("$.SceneData", "ForwardPass.SceneData");

		Scheme.AddLink("ForwardPass.Color", "SkyboxPass.ColorBuffer");
		Scheme.AddLink("ForwardPass.Depth", "SkyboxPass.DepthBuffer");

		Scheme.AddLink("SkyboxPass.ColorBuffer", "PostProcessingPass.InputColor");
		Scheme.AddLink("$.ColorBuffer", "PostProcessingPass.OutputColor");

		Scheme.AddLink("PostProcessingPass.OutputColor", "$.FINAL_IMAGE");

		FrameGraph = Scheme.Compile();
		HERMES_ASSERT_LOG(FrameGraph, "Failed to compile a frame graph");

		SceneDataBuffer = Renderer::GetDevice().CreateBuffer(sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
		FrameGraph->BindExternalResource("SceneData", *SceneDataBuffer);
	}

	std::pair<const Vulkan::Image*, VkImageLayout> SceneRenderer::Render(const Scene& Scene, Vec2ui ViewportDimensions) const
	{
		HERMES_PROFILE_FUNC();

		UpdateSceneDataBuffer(Scene, ViewportDimensions);

		auto GeometryList = Scene.BakeGeometryList();
		FrameGraph->Execute(Scene, GeometryList, ViewportDimensions);

		return FrameGraph->GetFinalImage();
	}

	void SceneRenderer::UpdateSceneDataBuffer(const Scene& Scene, Vec2ui ViewportDimensions) const
	{
		HERMES_PROFILE_FUNC();

		auto& Camera = Scene.GetActiveCamera();

		auto* SceneDataForCurrentFrame = static_cast<SceneData*>(SceneDataBuffer->Map());

		auto ViewMatrix = Camera.GetViewMatrix();
		auto ProjectionMatrix = Camera.GetProjectionMatrix();

		SceneDataForCurrentFrame->ViewProjection = ProjectionMatrix * ViewMatrix;
		SceneDataForCurrentFrame->View = ViewMatrix;
		SceneDataForCurrentFrame->InverseProjection = ProjectionMatrix.Inverse();
		SceneDataForCurrentFrame->CameraLocation = { Camera.GetLocation(), 1.0f };

		SceneDataForCurrentFrame->ScreenDimensions = Vec2(ViewportDimensions);
		SceneDataForCurrentFrame->MaxPixelsPerLightCluster = static_cast<Vec2>(LightCullingPass->ClusterSizeInPixels);
		SceneDataForCurrentFrame->CameraZBounds = { Camera.GetNearZPlane(), Camera.GetFarZPlane() };
		SceneDataForCurrentFrame->NumberOfZClusters.X = LightCullingPass->NumberOfZSlices;

		size_t NextPointLightIndex = 0, NextDirectionalLightIndex = 0;
		std::function<void(const SceneNode&)> TraverseSceneHierarchy = [&](const SceneNode& Node) -> void
		{
			for (size_t ChildIndex = 0; ChildIndex < Node.GetChildrenCount(); ChildIndex++)
				TraverseSceneHierarchy(Node.GetChild(ChildIndex));

			if (Node.GetType() == SceneNodeType::PointLight)
			{
				if (NextPointLightIndex > SceneData::MaxPointLightCount)
					return;

				const auto& PointLight = static_cast<const PointLightNode&>(Node);
				auto WorldTransform = PointLight.GetWorldTransformationMatrix();
				Vec4 WorldPosition = { WorldTransform[0][3], WorldTransform[1][3], WorldTransform[2][3], 1.0f };

				SceneDataForCurrentFrame->PointLights[NextPointLightIndex].Position = WorldPosition;
				SceneDataForCurrentFrame->PointLights[NextPointLightIndex].Color = Vec4(PointLight.GetColor(), PointLight.GetIntensity());

				NextPointLightIndex++;
			}

			if (Node.GetType() == SceneNodeType::DirectionalLight)
			{
				if (NextDirectionalLightIndex > SceneData::MaxDirectionalLightCount)
					return;

				const auto& DirectionalLight = static_cast<const DirectionalLightNode&>(Node);

				auto WorldTransform = Node.GetWorldTransformationMatrix();
				auto WorldDirection = WorldTransform * Vec4(DirectionalLight.GetDirection(), 0.0f);

				SceneDataForCurrentFrame->DirectionalLights[NextDirectionalLightIndex].Direction = WorldDirection;
				SceneDataForCurrentFrame->DirectionalLights[NextDirectionalLightIndex].Color = Vec4(DirectionalLight.GetColor(), DirectionalLight.GetIntensity());

				NextDirectionalLightIndex++;
			}
		};

		TraverseSceneHierarchy(Scene.GetRootNode());

		if (NextPointLightIndex >= SceneData::MaxPointLightCount)
			HERMES_LOG_WARNING("There are more point lights in the scene than the shader can process, some of them will be ignored");
		if (NextDirectionalLightIndex >= SceneData::MaxDirectionalLightCount)
			HERMES_LOG_WARNING("There are more directional lights in the scene than the shader can process, some of them will be ignored");

		SceneDataForCurrentFrame->PointLightCount = static_cast<uint32>(NextPointLightIndex);
		SceneDataForCurrentFrame->DirectionalLightCount = static_cast<uint32>(NextDirectionalLightIndex);

		SceneDataBuffer->Unmap();
	}
}
