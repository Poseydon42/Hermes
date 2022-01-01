#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
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

		~Material();
		ADD_DEFAULT_COPY_CONSTRUCTOR(Material)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Material)

		const Texture& GetBasicTexture(TextureType Type) const;
		Texture& GetBasicTexture(TextureType Type);

		const RenderInterface::DescriptorSet& GetMaterialDescriptorSet() const;

		static std::shared_ptr<RenderInterface::DescriptorSetLayout> GetDescriptorSetLayout();

		static void SetDefaultAnisotropyLevel(float NewLevel);

	private:
		std::vector<std::shared_ptr<Texture>> Textures;

		std::shared_ptr<RenderInterface::DescriptorSet> Descriptor;
		std::shared_ptr<RenderInterface::Sampler> Sampler;
		
		static constexpr uint32 MaterialDescriptorSetAllocationGranularity = 16;
		static constexpr uint32 SamplersPerMaterial = 1;
		static constexpr uint32 TexturesPerMaterial = static_cast<uint32>(TextureType::Count_);

		static float DefaultAnisotropyLevel;
		static std::shared_ptr<RenderInterface::DescriptorSetPool> MaterialDescriptorPool;
		static std::shared_ptr<RenderInterface::DescriptorSetLayout> MaterialDescriptorLayout;
		static std::vector<Material*> Instances;
		
		void UpdateSampler();

		static std::shared_ptr<RenderInterface::DescriptorSet> AllocateDescriptor();
		static void CreateDescriptorSetLayout();
		static void CreateDescriptorSetPool();
	};
}
