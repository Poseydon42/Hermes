#include "MaterialInstance.h"

#include "ApplicationCore/GameLoop.h"
#include "AssetSystem/AssetCache.h"
#include "JSON/JSONParser.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Texture.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	void MaterialInstance::SetTextureProperty(const String& Name, const Texture2D& Value, ColorSpace ColorSpace)
	{
		auto* Property = GetBaseMaterial().FindProperty(Name);
		HERMES_ASSERT(Property);

		DescriptorSet->UpdateWithImageAndSampler(Property->Binding, 0, Value.GetView(ColorSpace),
		                                         Renderer::Get().GetDefaultSampler(),
		                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void MaterialInstance::SetTextureProperty(const String& Name, AssetHandle TextureHandle, ColorSpace ColorSpace)
	{
		auto& AssetCache = GGameLoop->GetAssetCache();

		auto MaybeTexture = AssetCache.Get<Texture2D>(TextureHandle);
		if (!MaybeTexture || !MaybeTexture.value())
		{
			HERMES_LOG_WARNING("Cannot set material instance property %s because the texture cannot be found", Name.c_str());
			return;
		}

		const auto* Texture = MaybeTexture.value();
		HERMES_ASSERT(Texture);

		SetTextureProperty(Name, *Texture, ColorSpace);
		CurrentlyBoundRefCountedTextures[Name] = TextureHandle;
	}

	void MaterialInstance::PrepareForRender() const
	{
		if (IsDirty && HasUniformBuffer)
		{
			// TODO : use frame local allocator for better performance
			auto BufferSize = UniformBuffer->GetSize();
			auto* MappedMemory = UniformBuffer->Map();
			HERMES_ASSERT(MappedMemory);
			memcpy(MappedMemory, CPUBuffer.data(), BufferSize);
			UniformBuffer->Unmap();

			IsDirty = false;
		}
	}

	const Material& MaterialInstance::GetBaseMaterial() const
	{
		auto& AssetCache = GGameLoop->GetAssetCache();

		auto BaseMaterial = AssetCache.Get<Material>(BaseMaterialHandle);
		HERMES_ASSERT(BaseMaterial.has_value() && BaseMaterial.value());

		return *BaseMaterial.value();
	}

	const Vulkan::DescriptorSet& MaterialInstance::GetMaterialDescriptorSet() const
	{
		return *DescriptorSet;
	}

	MaterialInstance::MaterialInstance(AssetHandle InBaseMaterialHandle, size_t UniformBufferSize)
		: HasUniformBuffer(UniformBufferSize > 0)
		, BaseMaterialHandle(InBaseMaterialHandle)
	{
		CPUBuffer.resize(UniformBufferSize);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		DescriptorSet = DescriptorAllocator.Allocate(GetBaseMaterial().GetDescriptorSetLayout());

		if (HasUniformBuffer)
		{
			UniformBuffer = Device.CreateBuffer(UniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
			DescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, static_cast<uint32>(UniformBuffer->GetSize()));
			IsDirty = true;
		}		
	}
}
