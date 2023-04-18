#include "DepthPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/FrameGraph/Resource.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Mesh.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/FrameGraph/Graph.h"
#include "RenderingEngine/Scene/Camera.h"
#include "RenderingEngine/Scene/GeometryList.h"
#include "Vulkan/CommandBuffer.h"

namespace Hermes
{
	DepthPass::DepthPass()
	{
		auto& DescriptorAllocator = Renderer::GetDescriptorAllocator();

		SceneUBODescriptorSet = DescriptorAllocator.Allocate(Renderer::GetGlobalDataDescriptorSetLayout());

		Description.Callback = [this](const PassCallbackInfo& CallbackInfo) { PassCallback(CallbackInfo); };

		Attachment DepthAttachment = {};
		DepthAttachment.Name = "Depth";
		DepthAttachment.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		DepthAttachment.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		DepthAttachment.ClearColor.depthStencil.depth = 0.0f;
		DepthAttachment.Binding = BindingMode::DepthStencilAttachment;

		Description.Attachments = { std::move(DepthAttachment) };

		Description.BufferInputs =
		{
			{ "SceneData", VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, false }
		};
	}

	void DepthPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();

		auto& CommandBuffer = CallbackInfo.CommandBuffer;

		const auto* DepthBuffer = std::get<const Vulkan::ImageView*>(CallbackInfo.Resources.at("Depth"));
		HERMES_ASSERT(DepthBuffer);
		auto FramebufferDimensions = DepthBuffer->GetDimensions();
		auto ViewportDimensions = Vec2(FramebufferDimensions);

		const auto& SceneDataBuffer = *std::get<const Vulkan::Buffer*>(CallbackInfo.Resources.at("SceneData"));
		SceneUBODescriptorSet->UpdateWithBuffer(0, 0, SceneDataBuffer, 0, static_cast<uint32>(SceneDataBuffer.GetSize()));

		for (const auto& DrawableMesh : CallbackInfo.GeometryList.GetMeshList())
		{
			auto& Material = DrawableMesh.Material;
			auto& MaterialPipeline = Material->GetBaseMaterial().GetVertexOnlyPipeline(DepthBuffer->GetFormat());
			const auto* Mesh = DrawableMesh.Mesh;

			if (!Mesh)
				continue;

			Material->PrepareForRender();

			CommandBuffer.BindPipeline(MaterialPipeline);

			CommandBuffer.SetViewport({ 0.0f, 0.0f, ViewportDimensions.X, ViewportDimensions.Y, 0.0f, 1.0f });
			CommandBuffer.SetScissor({ { 0, 0 }, { FramebufferDimensions.X, FramebufferDimensions.Y } });

			CommandBuffer.BindDescriptorSet(*SceneUBODescriptorSet, MaterialPipeline, 0);
			CommandBuffer.BindVertexBuffer(Mesh->GetVertexBuffer());
			CommandBuffer.BindIndexBuffer(Mesh->GetIndexBuffer(), VK_INDEX_TYPE_UINT32);

			auto TransformationMatrix = DrawableMesh.TransformationMatrix;

			CommandBuffer.UploadPushConstants(MaterialPipeline, VK_SHADER_STAGE_VERTEX_BIT,
			                                  &TransformationMatrix, sizeof(TransformationMatrix), 0);

			for (const auto& Primitive : Mesh->GetPrimitives())
			{
				CommandBuffer.DrawIndexed(Primitive.IndexCount, 1, Primitive.IndexOffset, 0, 0);
			}
		}
	}

	const PassDesc& DepthPass::GetPassDescription() const
	{
		return Description;
	}
}
