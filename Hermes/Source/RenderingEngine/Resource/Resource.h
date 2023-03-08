#pragma once

#include "Core/Core.h"

namespace Hermes
{
	enum class ResourceType
	{
		Texture2D,
		TextureCube,

		Mesh
	};

	class HERMES_API Resource
	{
	public:
		virtual ~Resource() = default;

		Resource(String InName, ResourceType InType);

		ResourceType GetType() const;

		StringView GetName() const;

	private:
		String Name;
		ResourceType Type;
	};
}