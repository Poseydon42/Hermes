#pragma once

#include <memory>
#include <vector>

#include "Core/Core.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Material/Material.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Descriptor.h"

namespace Hermes
{
	class Material;
	class Texture;

	/*
	 * An instantiation of a material that stores the values of every material property.
	 *
	 * Can be applied to an object in the scene to change its visual appearance.
	 */
	class HERMES_API MaterialInstance
	{
	public:
		template<typename ValueType>
		void SetNumericProperty(const String& Name, const ValueType& Value, size_t ArrayIndex = 0);

		void SetTextureProperty(const String& Name, const Texture& Value);

		void PrepareForRender() const;

		const Material& GetBaseMaterial() const;

		const Vulkan::DescriptorSet& GetMaterialDescriptorSet() const;

	private:
		mutable bool IsDirty = false;
		bool HasUniformBuffer = false;

		std::shared_ptr<const Material> BaseMaterial;
		std::vector<uint8> CPUBuffer;
		std::unique_ptr<Vulkan::DescriptorSet> DescriptorSet;
		std::unique_ptr<Vulkan::Buffer> UniformBuffer;

		MaterialInstance(std::shared_ptr<const Material> Material, size_t UniformBufferSize);

		friend class Material;
	};

	// TODO : add type checking
	template<typename ValueType>
	void MaterialInstance::SetNumericProperty(const String& Name, const ValueType& Value, size_t ArrayIndex)
	{
		auto* Property = BaseMaterial->FindProperty(Name);
		HERMES_ASSERT_LOG(Property, L"Unknown material property '%s'", Name.c_str());

		auto SizeOfSingleElement = Property->Size / Property->ArrayLength;
		HERMES_ASSERT(sizeof(ValueType) <= SizeOfSingleElement);
		HERMES_ASSERT(ArrayIndex < Property->ArrayLength)
		memcpy(CPUBuffer.data() + Property->Offset + ArrayIndex * SizeOfSingleElement, &Value, sizeof(ValueType));

		IsDirty = true;
	}
}
