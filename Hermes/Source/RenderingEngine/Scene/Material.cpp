#include "Material.h"

#include "RenderingEngine/Renderer.h"
#include "RenderInterface/GenericRenderInterface/Sampler.h"
#include "RenderInterface/GenericRenderInterface/Shader.h"

namespace Hermes
{
	float                                                 Material::DefaultAnisotropyLevel;
	std::vector<Material*>                                Material::Instances;
	std::shared_ptr<RenderInterface::DescriptorSetPool>   Material::MaterialDescriptorPool;
	std::shared_ptr<RenderInterface::DescriptorSetLayout> Material::MaterialDescriptorLayout;

	Material::Material(std::vector<std::shared_ptr<Texture>> InTextures)
		: Textures(std::move(InTextures))
		, Descriptor(AllocateDescriptor())
	{
		HERMES_ASSERT(Textures.size() == static_cast<uint32>(TextureType::Count_));
		static_assert(SamplersPerMaterial == 1);

		Instances.push_back(this);

		Vec2ui TextureDimensions = Textures[0]->GetDimensions();
		for (size_t Index = 1; Index < Textures.size(); Index++)
		{
			if (TextureDimensions.X != Textures[Index]->GetDimensions().X ||
				TextureDimensions.Y != Textures[Index]->GetDimensions().Y)
			{
				HERMES_ASSERT_LOG(false, L"Found two textures with different sizes while trying to create material instance.");
			}
		}

		UpdateSampler();

		for (uint32 ImageIndex = 0; ImageIndex < static_cast<uint32>(TextureType::Count_); ImageIndex++)
		{
			if (Textures[ImageIndex]->IsReady())
			{
				Descriptor->UpdateWithImage(ImageIndex + 1, 0, Textures[ImageIndex]->GetRawImage(), RenderInterface::ImageLayout::ShaderReadOnlyOptimal);
			}
			else
			{
				HERMES_LOG_WARNING(L"Trying to create material with invalid texture handle %u", static_cast<uint32>(ImageIndex));
			}
		}
	}

	Material::~Material()
	{
		auto InstanceIterator = std::find(Instances.begin(), Instances.end(), this);
		HERMES_ASSERT_LOG(InstanceIterator != Instances.end(), L"Found material instance that is not contained in Material::Instances list");
		Instances.erase(InstanceIterator);
	}

	const Texture& Material::GetBasicTexture(TextureType Type) const
	{
		return *Textures[static_cast<size_t>(Type)];
	}

	Texture&  Material::GetBasicTexture(TextureType Type)
	{
		return *Textures[static_cast<size_t>(Type)];
	}

	const RenderInterface::DescriptorSet& Material::GetMaterialDescriptorSet() const
	{
		return *Descriptor;
	}

	std::shared_ptr<RenderInterface::DescriptorSetLayout> Material::GetDescriptorSetLayout()
	{
		if (!MaterialDescriptorLayout)
			CreateDescriptorSetLayout();
		return MaterialDescriptorLayout;
	}

	void Material::SetDefaultAnisotropyLevel(float NewLevel)
	{
		if (Math::Abs(DefaultAnisotropyLevel - NewLevel) > 0.000001f)
		{
			DefaultAnisotropyLevel = NewLevel;
			for (const auto& Instance : Instances)
			{
				Instance->UpdateSampler();
			}
		}
	}

	std::shared_ptr<RenderInterface::DescriptorSet> Material::AllocateDescriptor()
	{
		if (!MaterialDescriptorLayout)
		{
			CreateDescriptorSetLayout();
		}
		
		if (!MaterialDescriptorPool)
		{
			CreateDescriptorSetPool();
		}

		// TODO : currently this code crashes inside Vulkan rendering abstraction if
		// we fail to allocate descriptor set due to descriptor set pool being full.
		// Instead, we should be able to track whether descriptor pools get full and in
		// such case create more of them and successfully allocate further descriptor sets
		return MaterialDescriptorPool->CreateDescriptorSet(MaterialDescriptorLayout);
	}

	void Material::CreateDescriptorSetLayout()
	{
		auto& RenderingDevice = Renderer::Get().GetActiveDevice();

		std::vector<RenderInterface::DescriptorBinding> PerMaterialDataBindings =
		{
			/* Albedo sampler */
			{
				/*Index*/ 0,
				/*DescriptorCount*/ 1,
				/*Shader*/ RenderInterface::ShaderType::FragmentShader,
				/*Type*/ RenderInterface::DescriptorType::Sampler,
			},

			/* Albedo texture */
			{
				/*Index*/ 1,
				/*DescriptorCount*/ 1,
				/*Shader*/ RenderInterface::ShaderType::FragmentShader,
				/*Type*/ RenderInterface::DescriptorType::SampledImage,
			},

			/* Metallic texture */
			{
				/*Index*/ 2,
				/*DescriptorCount*/ 1,
				/*Shader*/ RenderInterface::ShaderType::FragmentShader,
				/*Type*/ RenderInterface::DescriptorType::SampledImage,
			},

			/* Roughness texture */
			{
				/*Index*/ 3,
				/*DescriptorCount*/ 1,
				/*Shader*/ RenderInterface::ShaderType::FragmentShader,
				/*Type*/ RenderInterface::DescriptorType::SampledImage,
			}
		};
		MaterialDescriptorLayout = RenderingDevice.CreateDescriptorSetLayout(PerMaterialDataBindings);
	}

	void Material::CreateDescriptorSetPool()
	{
		auto& RenderingDevice = Renderer::Get().GetActiveDevice();

		std::vector<RenderInterface::SubpoolDescription> MaterialDescriptorSubpools =
		{
			{
				/*Type*/  RenderInterface::DescriptorType::Sampler,
				/*Count*/ MaterialDescriptorSetAllocationGranularity * SamplersPerMaterial
			},
			{
				/*Type*/  RenderInterface::DescriptorType::SampledImage,
				/*Count*/ MaterialDescriptorSetAllocationGranularity * TexturesPerMaterial
			}
		};
		MaterialDescriptorPool = RenderingDevice.CreateDescriptorSetPool(MaterialDescriptorSetAllocationGranularity, MaterialDescriptorSubpools);
	}

	void Material::UpdateSampler()
	{
		auto& RenderingDevice = Renderer::Get().GetActiveDevice();

		RenderInterface::SamplerDescription Description;
		Description.CoordinateSystem = RenderInterface::CoordinateSystem::Normalized;
		Description.AddressingModeU = RenderInterface::AddressingMode::Repeat;
		Description.AddressingModeV = RenderInterface::AddressingMode::Repeat;
		Description.MinificationFilteringMode = RenderInterface::FilteringMode::Linear;
		Description.MagnificationFilteringMode = RenderInterface::FilteringMode::Linear;
		Description.AnisotropyLevel = DefaultAnisotropyLevel > 0.0f ? DefaultAnisotropyLevel : std::optional<float>{};
		Description.MinMipLevel = 0.0f;
		Description.MaxMipLevel = static_cast<float>(Textures[0]->GetMipLevelsCount());
		Description.MipBias = 0.0f;
		Description.MipMode = RenderInterface::MipmappingMode::Linear;
		Sampler = RenderingDevice.CreateSampler(Description);

		Descriptor->UpdateWithSampler(0, 0, *Sampler);
	}
}
