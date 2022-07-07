#pragma once

#include <memory>

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"
#include "Core/Misc/DefaultConstructors.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	namespace RenderInterface
	{
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

			/*
			 * For DescriptorType::Sampler
			 */
			virtual void UpdateWithSampler(uint32 BindingIndex, uint32 ArrayIndex, const Sampler& Sampler) = 0;

			// TODO : this should take image view rather than plain image
			/*
			 * For DescriptorType::SampledImage
			 */
			virtual void UpdateWithImage(uint32 BindingIndex, uint32 ArrayIndex, const ImageView& Image, ImageLayout LayoutAtTimeOfAccess) = 0;

			/*
			 * For DescriptorType::CombinedSampler
			 */
			virtual void UpdateWithImageAndSampler(uint32 BindingIndex, uint32 ArrayIndex, const ImageView& Image, const Sampler& Sampler, ImageLayout LayoutAtTimeOfAccess) = 0;
		};

		// TODO : other types
		enum class DescriptorType
		{
			UniformBuffer,
			Sampler,
			SampledImage,
			CombinedSampler,
			InputAttachment
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

		struct SubpoolDescription
		{
			DescriptorType Type;
			uint32 Count;
		};

		class HERMES_API DescriptorSetPool
		{
			MAKE_NON_COPYABLE(DescriptorSetPool)
			ADD_DEFAULT_CONSTRUCTOR(DescriptorSetPool)
			ADD_DEFAULT_VIRTUAL_DESTRUCTOR(DescriptorSetPool)
			ADD_DEFAULT_MOVE_CONSTRUCTOR(DescriptorSetPool)

		public:
			virtual std::unique_ptr<DescriptorSet> CreateDescriptorSet(const DescriptorSetLayout& Layout) = 0;

			virtual uint32 GetNumberOfSets() const = 0;
		};
	}
}
