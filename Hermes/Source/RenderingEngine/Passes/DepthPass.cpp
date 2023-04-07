#include "DepthPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Mesh.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/GeometryList.h"

namespace Hermes
{
	DepthPass::DepthPass()
	{
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		SceneUBODescriptorSet = DescriptorAllocator.Allocate(Renderer::Get().GetGlobalDataDescriptorSetLayout());

		Description.Callback = [this](const PassCallbackInfo& CallbackInfo) { PassCallback(CallbackInfo); };

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

		auto FramebufferDimensions = std::get<const Vulkan::ImageView*>(CallbackInfo.Resources.at("Depth"))->GetDimensions();
		auto ViewportDimensions = Vec2(FramebufferDimensions);

		const auto& GlobalSceneDataBuffer = Renderer::Get().GetGlobalSceneDataBuffer();
		SceneUBODescriptorSet->UpdateWithBuffer(0, 0, GlobalSceneDataBuffer, 0, static_cast<uint32>(GlobalSceneDataBuffer.GetSize()));

		for (const auto& DrawableMesh : CallbackInfo.GeometryList.GetMeshList())
		{
			auto& Material = DrawableMesh.Material;
			auto& MaterialPipeline = Material->GetBaseMaterial().GetVertexPipeline();
			const auto* Mesh = DrawableMesh.Mesh;

			if (!Mesh)
				continue;

			Material->PrepareForRender();

			CommandBuffer.BindPipeline(MaterialPipeline);
			Metrics.PipelineBindCount++;

			CommandBuffer.SetViewport({ 0.0f, 0.0f, ViewportDimensions.X, ViewportDimensions.Y, 0.0f, 1.0f });
			CommandBuffer.SetScissor({ { 0, 0 }, { FramebufferDimensions.X, FramebufferDimensions.Y } });

			CommandBuffer.BindDescriptorSet(*SceneUBODescriptorSet, MaterialPipeline, 0);
			Metrics.DescriptorSetBindCount++;
			CommandBuffer.BindVertexBuffer(Mesh->GetVertexBuffer());
			Metrics.BufferBindCount++;
			CommandBuffer.BindIndexBuffer(Mesh->GetIndexBuffer(), VK_INDEX_TYPE_UINT32);
			Metrics.BufferBindCount++;

			auto TransformationMatrix = DrawableMesh.TransformationMatrix;

			CommandBuffer.UploadPushConstants(MaterialPipeline, VK_SHADER_STAGE_VERTEX_BIT,
			                                  &TransformationMatrix, sizeof(TransformationMatrix), 0);

			for (const auto& Primitive : Mesh->GetPrimitives())
			{
				CommandBuffer.DrawIndexed(Primitive.IndexCount, 1, Primitive.IndexOffset, 0, 0);
				Metrics.DrawCallCount++;
			}
		}
	}

	const PassDesc& DepthPass::GetPassDescription() const
	{
		return Description;
	}
}
