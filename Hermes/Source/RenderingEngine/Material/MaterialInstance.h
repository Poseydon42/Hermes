#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Material/Material.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Descriptor.h"

namespace Hermes
{
	enum class ColorSpace;
	class JSONValue;
	class JSONObject;
	class Material;
	class Texture2D;

	/*
	 * An instantiation of a material that stores the values of every material property.
	 *
	 * Can be applied to an object in the scene to change its visual appearance.
	 */
	class HERMES_API MaterialInstance : public Asset
	{
	public:
		static std::unique_ptr<MaterialInstance> Create(String Name, AssetHandle Handle, AssetHandle BaseMaterialHandle);

		static std::unique_ptr<Asset> Load(const AssetLoaderCallbackInfo& CallbackInfo, const JSONObject& Data);

		template<typename ValueType>
		void SetNumericProperty(const String& Name, const ValueType& Value, size_t ArrayIndex = 0);

		template<typename ValueType>
		void SetNumericProperty(const MaterialProperty& Property, const ValueType& Value, size_t ArrayIndex = 0);
		
		void SetTextureProperty(const String& PropertyName, const Texture2D& Value, ColorSpace ColorSpace);
		
		void SetTextureProperty(const String& PropertyName, AssetHandle TextureHandle, ColorSpace ColorSpace);

		void PrepareForRender() const;

		const Material& GetBaseMaterial() const;

		const Vulkan::DescriptorSet& GetMaterialDescriptorSet() const;

	private:
		mutable bool IsDirty = false;
		bool HasUniformBuffer = false;

		AssetHandle BaseMaterialHandle;
		std::unordered_map<String, AssetHandle> CurrentlyBoundRefCountedTextures;
		std::vector<uint8> CPUBuffer;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;
		std::unique_ptr<Vulkan::Buffer> UniformBuffer;

		MaterialInstance(String InName, AssetHandle InHandle, AssetHandle InBaseMaterialHandle);

		/*
		 * Loading from JSON routines
		 */
		void SetVectorPropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& Value);
		
		void SetFloatVectorPropertyFromJSON(StringView PropertyName, const MaterialProperty& Property, const JSONValue& Value);
		
		void SetPropertyFromJSON(const String& PropertyName, const JSONValue& Value);
	};

	// TODO : add type checking
	template<typename ValueType>
	void MaterialInstance::SetNumericProperty(const String& Name, const ValueType& Value, size_t ArrayIndex)
	{
		auto* Property = GetBaseMaterial().FindProperty(Name);
		HERMES_ASSERT_LOG(Property, "Unknown material property '%s'", Name.c_str());

		SetNumericProperty(*Property, Value, ArrayIndex);
	}

	template<typename ValueType>
	void MaterialInstance::SetNumericProperty(const MaterialProperty& Property, const ValueType& Value, size_t ArrayIndex)
	{
		auto SizeOfSingleElement = Property.Size / Property.ArrayLength;
		HERMES_ASSERT(sizeof(ValueType) <= SizeOfSingleElement);
		HERMES_ASSERT(ArrayIndex < Property.ArrayLength);
		memcpy(CPUBuffer.data() + Property.Offset + ArrayIndex * SizeOfSingleElement, &Value, sizeof(ValueType));

		IsDirty = true;
	}
}
