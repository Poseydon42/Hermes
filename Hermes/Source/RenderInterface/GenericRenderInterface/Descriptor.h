#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ShaderType;

		struct DescriptorBinding
		{
			uint32 Index;
			uint32 DescriptorCount;
			ShaderType Shader;
			// TODO : descriptor types(for now we assume that it is uniform buffer, but we'd need to add samplers/images in future)
		};

		class HERMES_API DescriptorSetLayout
		{
			MAKE_NON_COPYABLE(DescriptorSetLayout)
			ADD_DEFAULT_CONSTRUCTOR(DescriptorSetLayout)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(DescriptorSetLayout)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(DescriptorSetLayout)
		};

		class HERMES_API DescriptorSetPool
		{
			MAKE_NON_COPYABLE(DescriptorSetPool)
			ADD_DEFAULT_CONSTRUCTOR(DescriptorSetPool)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(DescriptorSetPool)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(DescriptorSetPool)

		public:
			virtual uint32 GetSize() const = 0;
		};
	}
}
