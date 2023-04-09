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
			Instance->SetPropertyFromJSON(Name, Value, CallbackInfo.Dependencies);
		}

		return Instance;
	}

	void MaterialInstance::SetTextureProperty(const String& PropertyName, const Texture2D& Value, ColorSpace ColorSpace)
	{
		auto* Property = GetBaseMaterial().FindProperty(PropertyName);
		HERMES_ASSERT(Property);
		SetTextureProperty(*Property, Value, ColorSpace);
	}

	void MaterialInstance::SetTextureProperty(const MaterialProperty& Property, const Texture2D& Value, ColorSpace ColorSpace)
	{
		DescriptorSet->UpdateWithImageAndSampler(Property.Binding, 0, Value.GetView(ColorSpace), Renderer::GetDefaultSampler(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void MaterialInstance::SetTextureProperty(const String& PropertyName, AssetHandle TextureHandle, ColorSpace ColorSpace)
	{
		auto* Property = GetBaseMaterial().FindProperty(PropertyName);
		HERMES_ASSERT(Property);
		SetTextureProperty(*Property, TextureHandle, ColorSpace);
	}

	void MaterialInstance::SetTextureProperty(const MaterialProperty& Property, AssetHandle TextureHandle, ColorSpace ColorSpace)
	{
		auto& AssetCache = GGameLoop->GetAssetCache();

		auto MaybeTexture = AssetCache.Get<Texture2D>(TextureHandle);
		if (!MaybeTexture || !MaybeTexture.value())
		{
			HERMES_LOG_WARNING("Cannot set material instance property of material instance %s because the texture (handle 0x%zx) cannot be found", GetName().c_str(), static_cast<size_t>(TextureHandle));
			return;
		}

		const auto* Texture = MaybeTexture.value();
		HERMES_ASSERT(Texture);

		SetTextureProperty(Property, *Texture, ColorSpace);
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

		auto& Device = Renderer::GetDevice();
		auto& DescriptorAllocator = Renderer::GetDescriptorAllocator();

		DescriptorSet = DescriptorAllocator.Allocate(BaseMaterial.GetDescriptorSetLayout());

		if (HasUniformBuffer)
		{
			UniformBuffer = Device.CreateBuffer(UniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
			DescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, static_cast<uint32>(UniformBuffer->GetSize()));
			IsDirty = true;
		}		
	}

	void MaterialInstance::SetScalarPropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& JSONValue)
	{
		if (!JSONValue.Is(JSONValueType::Number))
		{
			HERMES_LOG_WARNING("Cannot set property %s of material instance %s: JSON value must have numeric type", PropertyName.data(), GetName().data());
			return;
		}

		auto Value = JSONValue.AsNumber();

		switch (Property.DataType)
		{
		case MaterialPropertyDataType::Float:
			SetNumericProperty(Property, static_cast<float>(Value));
			break;
		default:
			HERMES_ASSERT_LOG(false, "Material property %s has unknown data type %u", PropertyName.data(), static_cast<uint32>(Property.DataType));
		}
	}

	void MaterialInstance::SetVectorPropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& JSONValue)
	{
		size_t ComponentCount = Property.Width;

		HERMES_ASSERT(ComponentCount > 1 && ComponentCount <= 4);

		if (!JSONValue.Is(JSONValueType::Array))
		{
			HERMES_LOG_WARNING("Cannot set property %s of material instance %s: JSON value must have array type", PropertyName.data(), GetName().c_str());
			return;
		}

		auto Array = JSONValue.AsArray();
		if (Array.size() != ComponentCount)
		{
			HERMES_LOG_WARNING("Cannot set property %s of material instance %s: JSON array has invalid size", PropertyName.data(), GetName().c_str());
			return;
		}

		switch (Property.DataType)
		{
		case MaterialPropertyDataType::Float:
			SetFloatVectorPropertyFromJSON(PropertyName, Property, JSONValue);
			break;
		default:
			HERMES_ASSERT_LOG(false, "Material property %s has unknown data type %u", PropertyName.data(), static_cast<uint32>(Property.DataType))
		}
	}

	static ColorSpace ColorSpaceFromStringOr(StringView String, ColorSpace Fallback)
	{
		if (String == "linear")
			return ColorSpace::Linear;
		if (String == "srgb")
			return ColorSpace::SRGB;
		return Fallback;
	}

	void MaterialInstance::SetTexturePropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& JSONValue, std::span<const AssetHandle> Dependencies)
	{
		static constexpr auto DefaultColorSpace = ColorSpace::Linear; // TODO: set the default color space in material asset

		if (JSONValue.Is(JSONValueType::Number))
		{
			auto DependencyIndex = static_cast<size_t>(JSONValue.AsInteger());
			if (DependencyIndex >= Dependencies.size())
			{
				HERMES_LOG_WARNING("Cannot set property %s of material instance %s: invalid texture index", PropertyName.data(), GetName().c_str());
				return;
			}

			auto TextureHandle = Dependencies[DependencyIndex];
			SetTextureProperty(Property, TextureHandle, DefaultColorSpace);
		}
		else if (JSONValue.Is(JSONValueType::Object))
		{
			const auto& JSONTextureContainer = JSONValue.AsObject();
			if (!JSONTextureContainer.Contains("texture") || !JSONTextureContainer["texture"].Is(JSONValueType::Number))
			{
				HERMES_LOG_WARNING("Cannot set property %s of material instance %s: texture index not specified", PropertyName.data(), GetName().c_str());
				return;
			}

			auto DependencyIndex = static_cast<size_t>(JSONTextureContainer["texture"].AsInteger());
			if (DependencyIndex >= Dependencies.size())
			{
				HERMES_LOG_WARNING("Cannot set property %s of material instance %s: invalid texture index", PropertyName.data(), GetName().c_str());
				return;
			}

			auto TextureHandle = Dependencies[DependencyIndex];

			auto ColorSpace = DefaultColorSpace;
			if (JSONTextureContainer.Contains("color_space") && JSONTextureContainer["color_space"].Is(JSONValueType::String))
			{
				ColorSpace = ColorSpaceFromStringOr(JSONTextureContainer["color_space"].AsString(), DefaultColorSpace);
			}

			SetTextureProperty(Property, TextureHandle, ColorSpace);
		}
	}

	void MaterialInstance::SetFloatVectorPropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& JSONValue)
	{
		HERMES_ASSERT(JSONValue.Is(JSONValueType::Array));
		HERMES_ASSERT(JSONValue.AsArray().size() == Property.Width);

		auto ValueAsArray = JSONValue.AsArray();
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

	void MaterialInstance::SetPropertyFromJSON(const String& PropertyName, const JSONValue& Value, std::span<const AssetHandle> Dependencies)
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
		case MaterialPropertyType::Value:
			SetScalarPropertyFromJSON(PropertyName, *Property, Value);
			break;
		case MaterialPropertyType::Vector:
			SetVectorPropertyFromJSON(PropertyName, *Property, Value);
			break;
		case MaterialPropertyType::Texture:
			SetTexturePropertyFromJSON(PropertyName, *Property, Value, Dependencies);
			break;
		default:
			HERMES_LOG_WARNING("Cannot set property %s of material instance asset %s: this property type is not supported yet", PropertyName.c_str(), GetName().c_str());
			break;
		}
	}
}
