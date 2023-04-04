#include "LightCullingPass.h"

#include "Core/Profiling.h"
#include "RenderingEngine/Renderer.h"

namespace Hermes
{
	LightCullingPass::LightCullingPass()
	{
		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		auto DescriptorSetLayout = Device.CreateDescriptorSetLayout(
			{
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr },
				{ 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr }
			});
		DescriptorSet = DescriptorAllocator.Allocate(*DescriptorSetLayout);

		auto Shader = Device.CreateShader("/Shaders/Bin/light_culling.glsl.spv", VK_SHADER_STAGE_COMPUTE_BIT);
		Pipeline = Device.CreateComputePipeline({ DescriptorSetLayout.get() }, *Shader);

		PassDescription.Type = PassType::Compute;
		PassDescription.BufferInputs =
		{
			{ "LightClusterList", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false },
			{ "LightIndexList", VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, false }
		};
		PassDescription.Callback = [this](const PassCallbackInfo& CallbackInfo) { PassCallback(CallbackInfo); };
	}

	const PassDesc& LightCullingPass::GetPassDescription() const
	{
		return PassDescription;
	}

	void LightCullingPass::PassCallback(const PassCallbackInfo& CallbackInfo)
	{
		HERMES_PROFILE_FUNC();

		const auto& GlobalSceneDataBuffer = Renderer::Get().GetGlobalSceneDataBuffer();
		const auto& LightClusterListBuffer = *std::get<const Vulkan::Buffer*>(CallbackInfo.Resources.at("LightClusterList"));
		const auto& LightIndexListBuffer = *std::get<const Vulkan::Buffer*>(CallbackInfo.Resources.at("LightIndexList"));

		DescriptorSet->UpdateWithBuffer(0, 0, GlobalSceneDataBuffer, 0, static_cast<uint32>(GlobalSceneDataBuffer.GetSize()));
		DescriptorSet->UpdateWithBuffer(1, 0, LightClusterListBuffer, 0, static_cast<uint32>(LightClusterListBuffer.GetSize()));
		DescriptorSet->UpdateWithBuffer(2, 0, LightIndexListBuffer, 0, static_cast<uint32>(LightIndexListBuffer.GetSize()));

		auto SwapchainDimensions = Renderer::Get().GetSwapchain().GetDimensions();
		auto NumOfClustersXY = (SwapchainDimensions + ClusterSizeInPixels - 1) / ClusterSizeInPixels;

		auto& CommandBuffer = CallbackInfo.CommandBuffer;
		CommandBuffer.BindPipeline(*Pipeline);
		CommandBuffer.BindDescriptorSet(*DescriptorSet, *Pipeline, 0);
		CommandBuffer.Dispatch(NumOfClustersXY.X, NumOfClustersXY.Y, NumberOfZSlices);

		auto& Metrics = CallbackInfo.Metrics;
		Metrics.PipelineBindCount++;
		Metrics.DescriptorSetBindCount++;
	}
}
