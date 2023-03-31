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
	DEFINE_ASSET_TYPE(MaterialInstance, MaterialInstance);
	HERMES_ADD_TEXT_ASSET_LOADER(MaterialInstance, "material_instance");

	std::unique_ptr<MaterialInstance> MaterialInstance::Create(String Name, AssetHandle Handle, AssetHandle BaseMaterialHandle)
	{
		return std::unique_ptr<MaterialInstance>(new MaterialInstance(std::move(Name), Handle, BaseMaterialHandle));
	}

	std::unique_ptr<Asset> MaterialInstance::Load(const AssetLoaderCallbackInfo& CallbackInfo, const JSONObject& Data)
	{
		if (!Data.Contains("material") || !Data["material"].Is(JSONValueType::Number))
		{
			HERMES_LOG_ERROR("No base material is defined for material instance %s", CallbackInfo.Name.data());
			return nullptr;
		}

		auto BaseMaterialDependencyIndex = static_cast<size_t>(Data["material"].AsInteger());
		if (CallbackInfo.Dependencies.size() < BaseMaterialDependencyIndex)
		{
			HERMES_LOG_ERROR("Base material index %zu is invalid (while loading material instance %s)", BaseMaterialDependencyIndex, CallbackInfo.Name.data());
			return nullptr;
		}

		auto BaseMaterialHandle = CallbackInfo.Dependencies[BaseMaterialDependencyIndex];
		auto BaseMaterial = Hermes::GGameLoop->GetAssetCache().Get<Material>(BaseMaterialHandle);

		if (!BaseMaterial.has_value() || !BaseMaterial.value())
		{
			HERMES_LOG_ERROR("Base material for material instance asset %s cannot be loaded", CallbackInfo.Name.data());
			return nullptr;
		}

		auto Instance = Create(String(CallbackInfo.Name), CallbackInfo.Handle, BaseMaterialHandle);

		if (!Data.Contains("values") || !Data["values"].Is(JSONValueType::Object))
			return Instance;

		for (const auto& [Name, Value] : Data["values"].AsObject())
		{
			Instance->SetPropertyFromJSON(Name, Value);
		}

		return Instance;
	}

	void MaterialInstance::SetTextureProperty(const String& PropertyName, const Texture2D& Value, ColorSpace ColorSpace)
	{
		auto* Property = GetBaseMaterial().FindProperty(PropertyName);
		HERMES_ASSERT(Property);

		DescriptorSet->UpdateWithImageAndSampler(Property->Binding, 0, Value.GetView(ColorSpace),
		                                         Renderer::Get().GetDefaultSampler(),
		                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void MaterialInstance::SetTextureProperty(const String& PropertyName, AssetHandle TextureHandle, ColorSpace ColorSpace)
	{
		auto& AssetCache = GGameLoop->GetAssetCache();

		auto MaybeTexture = AssetCache.Get<Texture2D>(TextureHandle);
		if (!MaybeTexture || !MaybeTexture.value())
		{
			HERMES_LOG_WARNING("Cannot set material instance property %s because the texture cannot be found", PropertyName.c_str());
			return;
		}

		const auto* Texture = MaybeTexture.value();
		HERMES_ASSERT(Texture);

		SetTextureProperty(PropertyName, *Texture, ColorSpace);
		CurrentlyBoundRefCountedTextures[PropertyName] = TextureHandle;
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

	MaterialInstance::MaterialInstance(String InName, AssetHandle InHandle, AssetHandle InBaseMaterialHandle)
		: Asset(std::move(InName), AssetType::MaterialInstance, InHandle)
		, BaseMaterialHandle(InBaseMaterialHandle)
	{
		const auto& BaseMaterial = GetBaseMaterial();

		auto UniformBufferSize = BaseMaterial.GetUniformBufferSize();
		HasUniformBuffer = (UniformBufferSize > 0);

		CPUBuffer.resize(UniformBufferSize);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		DescriptorSet = DescriptorAllocator.Allocate(BaseMaterial.GetDescriptorSetLayout());

		if (HasUniformBuffer)
		{
			UniformBuffer = Device.CreateBuffer(UniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
			DescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, static_cast<uint32>(UniformBuffer->GetSize()));
			IsDirty = true;
		}		
	}

	void MaterialInstance::SetVectorPropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& Value)
	{
		size_t ComponentCount = Property.Width;

		HERMES_ASSERT(ComponentCount > 1 && ComponentCount <= 4);

		if (!Value.Is(JSONValueType::Array))
		{
			HERMES_LOG_WARNING("Cannot set property %s of material instance %s: JSON value must have array type", PropertyName.data(), GetName().c_str());
			return;
		}

		auto Array = Value.AsArray();
		if (Array.size() != ComponentCount)
		{
			HERMES_LOG_WARNING("Cannot set property %s of material instance %s: JSON array has invalid size", PropertyName.data(), GetName().c_str());
			return;
		}

		switch (Property.DataType)
		{
		case MaterialPropertyDataType::Float:
			SetFloatVectorPropertyFromJSON(PropertyName, Property, Value);
			break;
		default:
			HERMES_ASSERT_LOG(false, "Material property %s has unknown data type %u", PropertyName.data(), static_cast<uint32>(Property.DataType))
		}
	}

	void MaterialInstance::SetFloatVectorPropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& Value)
	{
		HERMES_ASSERT(Value.Is(JSONValueType::Array));
		HERMES_ASSERT(Value.AsArray().size() == Property.Width);

		auto ValueAsArray = Value.AsArray();
		for (const auto& Element : ValueAsArray)
		{
			if (Element.Is(JSONValueType::Number))
				continue;

			HERMES_LOG_WARNING("Cannot set property %s of material instance %s: array element is not a number", PropertyName.data(), GetName().data());
			return;
		}

		switch (Property.Width)
		{
		case 2:
		{
			Vec2 VectorValue = { static_cast<float>(ValueAsArray[0].AsNumber()), static_cast<float>(ValueAsArray[1].AsNumber()) };
			SetNumericProperty(Property, VectorValue);
			break;
		}
		case 3:
		{
			Vec3 VectorValue = { static_cast<float>(ValueAsArray[0].AsNumber()), static_cast<float>(ValueAsArray[1].AsNumber()), static_cast<float>(ValueAsArray[2].AsNumber()) };
			SetNumericProperty(Property, VectorValue);
			break;
		}
		case 4:
		{
			Vec4 VectorValue = { static_cast<float>(ValueAsArray[0].AsNumber()), static_cast<float>(ValueAsArray[1].AsNumber()), static_cast<float>(ValueAsArray[2].AsNumber()), static_cast<float>(ValueAsArray[3].AsNumber()) };
			SetNumericProperty(Property, VectorValue);
			break;
		}
		default:
			HERMES_ASSERT(false);
		}
	}

	void MaterialInstance::SetPropertyFromJSON(const String& PropertyName, const JSONValue& Value)
	{
		auto& BaseMaterial = GetBaseMaterial();

		const auto* Property = BaseMaterial.FindProperty(PropertyName);
		if (!Property)
		{
			HERMES_LOG_WARNING("Cannot set property %s of material instance asset %s: property does not exist", PropertyName.c_str(), GetName().c_str());
			return;
		}

		switch (Property->Type)
		{
		case MaterialPropertyType::Vector:
			SetVectorPropertyFromJSON(PropertyName, *Property, Value);
			break;
		default:
			HERMES_LOG_WARNING("Cannot set property %s of material instance asset %s: this property type is not supported yet", PropertyName.c_str(), GetName().c_str());
			break;
		}
	}
}
