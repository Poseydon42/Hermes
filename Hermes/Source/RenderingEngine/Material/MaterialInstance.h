#pragma once

#include <memory>
#include <vector>

#include "Material.h"
#include "Core/Core.h"
#include "Logging/Logger.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class Material;

	/*
	 * An instantiation of a material that stores the values of every material property.
	 *
	 * Can be applied to an object in the scene to change its visual appearance.
	 */
	class HERMES_API MaterialInstance
	{
	public:
		template<typename ValueType>
		void SetProperty(const String& Name, const ValueType& Value);

		void PrepareForRender() const;

		const Material& GetBaseMaterial() const;

		const RenderInterface::DescriptorSet& GetMaterialDescriptorSet() const;

	private:
		mutable bool IsDirty = true;

		std::shared_ptr<const Material> BaseMaterial;
		std::vector<uint8> CPUBuffer;
		std::unique_ptr<RenderInterface::DescriptorSet> DescriptorSet;
		std::unique_ptr<RenderInterface::Buffer> UniformBuffer;

		MaterialInstance(std::shared_ptr<const Material> Material, size_t UniformBufferSize);

		friend class Material;
	};

	// TODO : add type checking
	template<typename ValueType>
	void MaterialInstance::SetProperty(const String& Name, const ValueType& Value)
	{
		auto* Property = BaseMaterial->FindProperty(Name);
		HERMES_ASSERT_LOG(Property, L"Unknown material property '%s'", Name.c_str());
		HERMES_ASSERT(sizeof(ValueType) <= GetMaterialPropertySize(Property->Type));
		memcpy(CPUBuffer.data() + Property->Offset, &Value, sizeof(ValueType));

		IsDirty = true;
	}
}
