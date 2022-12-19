#include "ShaderCache.h"

#include "RenderingEngine/Renderer.h"

namespace Hermes
{
	const ShaderCache::ShaderContainer& ShaderCache::GetShaderContainer(const String& Name, VkShaderStageFlagBits Stage)
	{
		auto MaybeLoadedShader = LoadedShaders.find(Name);
		if (MaybeLoadedShader != LoadedShaders.end())
		{
			HERMES_ASSERT(MaybeLoadedShader->second.Shader->GetType() == Stage);
			return MaybeLoadedShader->second;
		}

		auto& Device = Renderer::Get().GetActiveDevice();
		auto NewShader = Device.CreateShader(Name, Stage);
		auto NewShaderReflection = ShaderReflection(Name);
		
		auto Result = LoadedShaders.insert(std::make_pair(Name, ShaderContainer{ std::move(NewShader), std::move(NewShaderReflection) }));

		return Result.first->second;
	}

	const Vulkan::Shader& ShaderCache::GetShader(const String& Name, VkShaderStageFlagBits Stage)
	{
		const auto& Container = GetShaderContainer(Name, Stage);
		return *Container.Shader;
	}

	const ShaderReflection& ShaderCache::GetShaderReflection(const String& Name, VkShaderStageFlagBits Stage)
	{
		const auto& Container = GetShaderContainer(Name, Stage);
		return Container.Reflection;
	}
}
