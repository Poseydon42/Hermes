#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "AssetSystem/AssetCache.h"
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
	class Texture2DResource;

	/*
	 * An instantiation of a material that stores the values of every material property.
	 *
	 * Can be applied to an object in the scene to change its visual appearance.
	 */
	class HERMES_API MaterialInstance
	{
	public:
		static std::optional<std::unique_ptr<MaterialInstance>> CreateFromJSON(StringView JSON);

		template<typename ValueType>
		void SetNumericProperty(const String& Name, const ValueType& Value, size_t ArrayIndex = 0);

		/*
		 * Sets a texture property using a direct reference to a Texture object.
		 *
		 * Reference counting and ensuring the lifetime of the texture is a responsibility of the user in this case.
		 */
		void SetTextureProperty(const String& Name, const Texture2DResource& Value, ColorSpace ColorSpace);

		/*
		 * Sets a texture property using a texture name. The texture will be acquired from a global TextureCache object.
		 *
		 * Reference counting is implemented in the texture cache object and valid lifetime is guaranteed.
		 */
		void SetTextureProperty(const String& Name, AssetHandle TextureHandle, ColorSpace ColorSpace);

		void PrepareForRender() const;

		const Material& GetBaseMaterial() const;

		const Vulkan::DescriptorSet& GetMaterialDescriptorSet() const;

	private:
		mutable bool IsDirty = false;
		bool HasUniformBuffer = false;

		std::shared_ptr<const Material> BaseMaterial;
		std::unordered_map<String, AssetHandle> CurrentlyBoundRefCountedTextures;
		std::vector<uint8> CPUBuffer;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;
		std::unique_ptr<Vulkan::Buffer> UniformBuffer;

		MaterialInstance(std::shared_ptr<const Material> Material, size_t UniformBufferSize);

		void SetScalarPropertyFromJSON(const String& Name, const JSONValue& Value);

		void SetVectorPropertyFromJSON(const String& Name, const JSONValue& Value);

		void SetTexturePropertyFromJSON(const String& Name, const JSONValue& Value);

		void SetPropertiesFromJSON(const JSONObject& PropertiesObject);

		friend class Material;
	};

	// TODO : add type checking
	template<typename ValueType>
	void MaterialInstance::SetNumericProperty(const String& Name, const ValueType& Value, size_t ArrayIndex)
	{
		auto* Property = BaseMaterial->FindProperty(Name);
		HERMES_ASSERT_LOG(Property, "Unknown material property '%s'", Name.c_str());

		auto SizeOfSingleElement = Property->Size / Property->ArrayLength;
		HERMES_ASSERT(sizeof(ValueType) <= SizeOfSingleElement);
		HERMES_ASSERT(ArrayIndex < Property->ArrayLength)
		memcpy(CPUBuffer.data() + Property->Offset + ArrayIndex * SizeOfSingleElement, &Value, sizeof(ValueType));

		IsDirty = true;
	}
}
