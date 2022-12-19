#pragma once

#include <unordered_map>

#include "Core/Core.h"
#include "RenderingEngine/Material/ShaderReflection.h"
#include "Vulkan/Shader.h"

namespace Hermes
{
	class HERMES_API ShaderCache
	{
		MAKE_NON_COPYABLE(ShaderCache)

		ADD_DEFAULT_CONSTRUCTOR(ShaderCache)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(ShaderCache)
		ADD_DEFAULT_DESTRUCTOR(ShaderCache)

	public:
		const Vulkan::Shader& GetShader(const String& Name, VkShaderStageFlagBits Stage);

		const ShaderReflection& GetShaderReflection(const String& Name, VkShaderStageFlagBits Stage);

	private:
		struct ShaderContainer
		{
			std::unique_ptr<Vulkan::Shader> Shader;
			ShaderReflection Reflection;
		};

		std::unordered_map<String, ShaderContainer> LoadedShaders;

		const ShaderContainer& GetShaderContainer(const String& Name, VkShaderStageFlagBits Stage);
	};
}
