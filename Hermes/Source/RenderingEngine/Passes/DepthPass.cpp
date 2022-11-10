#include "DepthPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/SharedData.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/GeometryList.h"
#include "RenderingEngine/Scene/Scene.h"
#include "RenderingEngine/Scene/SceneProxies.h"

namespace Hermes
{
	DepthPass::DepthPass()
	{
		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		SceneUBO = Device.CreateBuffer(sizeof(Renderer::Get().GetSceneDataForCurrentFrame()),
		                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
		SceneUBODescriptorSet = DescriptorAllocator.Allocate(Renderer::Get().GetGlobalDataDescriptorSetLayout());

		Description.Callback.Bind<DepthPass, &DepthPass::PassCallback>(this);

		Attachment DepthAttachment = {};
		DepthAttachment.Name = "Depth";
		DepthAttachment.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		DepthAttachment.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		DepthAttachment.ClearColor.depthStencil.depth = 0.0f;
		DepthAttachment.Binding = BindingMode::DepthStencilAttachment;

		Description.Attachments = { std::move(DepthAttachment) };
	}

	void DepthPass::PassCallback(Vulkan::CommandBuffer& CommandBuffer, const Vulkan::RenderPass&,
	                             const std::vector<std::pair<const Vulkan::Image*, const Vulkan::ImageView*>>&,
	                             const Scene& Scene, const GeometryList& GeometryList, FrameMetrics& Metrics, bool)
	{
		HERMES_PROFILE_FUNC();

		// TODO: do all of this once at the start of the frame in the global renderer
		auto& Camera = Scene.GetActiveCamera();
		auto ViewProjectionMatrix = Camera.GetProjectionMatrix() * Camera.GetViewMatrix();

		const auto& SceneUBOData = Renderer::Get().GetSceneDataForCurrentFrame();
		auto* SceneDataMemory = SceneUBO->Map();
		memcpy(SceneDataMemory, &SceneUBOData, sizeof(SceneUBOData));
		SceneUBO->Unmap();

		SceneUBODescriptorSet->UpdateWithBuffer(0, 0, *SceneUBO, 0, static_cast<uint32>(SceneUBO->GetSize()));

		for (const auto& Mesh : GeometryList.GetMeshList())
		{
			auto& Material = Mesh.Material;
			auto& MaterialPipeline = Material->GetBaseMaterial().GetVertexPipeline();
			auto& MeshBuffer = Mesh.MeshData;
			
			Material->PrepareForRender();

			CommandBuffer.BindPipeline(MaterialPipeline);
			Metrics.PipelineBindCount++;
			CommandBuffer.BindDescriptorSet(*SceneUBODescriptorSet, MaterialPipeline, 0);
			Metrics.DescriptorSetBindCount++;
			CommandBuffer.BindVertexBuffer(MeshBuffer->GetVertexBuffer());
			Metrics.BufferBindCount++;
			CommandBuffer.BindIndexBuffer(MeshBuffer->GetIndexBuffer(), VK_INDEX_TYPE_UINT32);
			Metrics.BufferBindCount++;
			CommandBuffer.UploadPushConstants(MaterialPipeline, VK_SHADER_STAGE_VERTEX_BIT,
			                                  &Mesh.TransformationMatrix, sizeof(Mesh.TransformationMatrix), 0);
			const auto& DrawInformation = MeshBuffer->GetDrawInformation();
			CommandBuffer.DrawIndexed(DrawInformation.IndexCount, 1, DrawInformation.IndexOffset,
			                          static_cast<int32>(DrawInformation.VertexOffset), 0);
			Metrics.DrawCallCount++;
		}
	}

	const PassDesc& DepthPass::GetPassDescription() const
	{
		return Description;
	}
}
