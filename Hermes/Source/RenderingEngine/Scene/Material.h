#pragma once

#include "Core/Core.h"
#include "RenderingEngine/Texture.h"
#include "RenderInterface/GenericRenderInterface/Descriptor.h"

namespace Hermes
{
	enum class TextureType : uint32
	{
		Albedo = 0,
		Roughness = 1,
		Metallic = 2,
		Count_
	};

	class HERMES_API Material
	{
	public:
		Material(std::vector<std::shared_ptr<Texture>> InTextures);

		const Texture& GetBasicTexture(TextureType Type) const;
		Texture& GetBasicTexture(TextureType Type);

		const RenderInterface::DescriptorSet& GetMaterialDescriptorSet() const;

		static std::shared_ptr<RenderInterface::DescriptorSetLayout> GetDescriptorSetLayout();

	private:
		std::vector<std::shared_ptr<Texture>> Textures;

		std::shared_ptr<RenderInterface::DescriptorSet> Descriptor;
		
		static constexpr uint32 MaterialDescriptorSetAllocationGranularity = 16;
		static constexpr uint32 SamplersPerMaterial = 1;
		static constexpr uint32 TexturesPerMaterial = static_cast<uint32>(TextureType::Count_);
		
		static std::shared_ptr<RenderInterface::Sampler> DefaultSampler;
		static std::shared_ptr<RenderInterface::DescriptorSetPool> MaterialDescriptorPool;
		static std::shared_ptr<RenderInterface::DescriptorSetLayout> MaterialDescriptorLayout;

		static std::shared_ptr<RenderInterface::DescriptorSet> AllocateDescriptor();
		static void CreateDescriptorSetLayout();
		static void CreateDescriptorSetPool();
		static void CreateDefaultSampler();
	};
}
