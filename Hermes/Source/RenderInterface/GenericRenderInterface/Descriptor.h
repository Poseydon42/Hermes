#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"

namespace Hermes
{
	namespace RenderInterface
	{
		enum class ShaderType;
		class Buffer;

		class HERMES_API DescriptorSet
		{
			MAKE_NON_COPYABLE(DescriptorSet)
			ADD_DEFAULT_CONSTRUCTOR(DescriptorSet)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(DescriptorSet)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(DescriptorSet)

		public:
			/**
			 * For DescriptorType::UniformBuffer
			 */
			virtual void UpdateWithBuffer(uint32 BindingIndex, uint32 ArrayIndex, const Buffer& Buffer, uint32 Offset, uint32 Size) = 0;
		};

		// TODO : other types
		enum class DescriptorType
		{
			UniformBuffer,
			Sampler,
			SampledImage
		};

		struct DescriptorBinding
		{
			uint32 Index;
			uint32 DescriptorCount;
			ShaderType Shader;
			DescriptorType Type;
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
			virtual std::shared_ptr<DescriptorSet> CreateDescriptorSet(std::shared_ptr<DescriptorSetLayout> Layout) = 0;

			virtual uint32 GetSize() const = 0;
		};
	}
}
