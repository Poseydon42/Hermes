#include "DepthPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/GeometryList.h"
#include "RenderingEngine/Scene/SceneProxies.h"

namespace Hermes
{
	DepthPass::DepthPass()
	{
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

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

	void DepthPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();

		auto& CommandBuffer = CallbackInfo.CommandBuffer;
		auto& Metrics = CallbackInfo.Metrics;

		const auto& GlobalSceneDataBuffer = Renderer::Get().GetGlobalSceneDataBuffer();
		SceneUBODescriptorSet->UpdateWithBuffer(0, 0, GlobalSceneDataBuffer, 0, static_cast<uint32>(GlobalSceneDataBuffer.GetSize()));

		for (const auto& Mesh : CallbackInfo.GeometryList.GetMeshList())
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
