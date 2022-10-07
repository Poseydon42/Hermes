#pragma once

#include <unordered_map>

#include "Core/Core.h"
#include "RenderingEngine/Material/MaterialProperty.h"

namespace Hermes
{
	/*
	 * Stores a list of exposed properties of a single shader
	 *
	 * Currently in development, the functionality is very limited.
	 * At the moment, only supports a single uniform buffer at
	 * descriptor set 1 binding 0.
	 */
	class HERMES_API ShaderReflection
	{
	public:
		ShaderReflection(const String& ShaderPath);

		const MaterialProperty* FindProperty(const String& Name) const;

		size_t GetTotalSizeForUniformBuffer() const;

	private:
		std::unordered_map<String, MaterialProperty> Properties;
		size_t SizeForUniformBuffer = 0;
	};
}
