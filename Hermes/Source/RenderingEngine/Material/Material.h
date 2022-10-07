#pragma once

#include <vector>

#include "MaterialProperty.h"
#include "Core/Core.h"
#include "Logging/Logger.h"
#include "RenderingEngine/Texture.h"
#include "RenderInterface/GenericRenderInterface/Forward.h"

namespace Hermes
{
	class HERMES_API Material
	{
	public:
		static std::shared_ptr<Material> Create();

		template<typename ValueType>
		void SetProperty(const String& Name, const ValueType& Value);

		void Update() const;

		const RenderInterface::DescriptorSet& GetMaterialDescriptorSet() const;

		const RenderInterface::Pipeline& GetPipeline() const;

	private:
		std::vector<MaterialProperty> Properties;
		bool IsDirty = true;

		std::unique_ptr<RenderInterface::DescriptorSetLayout> DescriptorSetLayout;
		std::unique_ptr<RenderInterface::DescriptorSet> DescriptorSet;
		std::unique_ptr<RenderInterface::Pipeline> Pipeline;
		std::unique_ptr<RenderInterface::Buffer> UniformBuffer;

		Material();

		size_t CalculateUniformBufferSize() const;
	};

	// TODO: add some type checking (at least in debug builds)?
	template<typename ValueType>
	void Material::SetProperty(const String& Name, const ValueType& Value)
	{
		auto Property = std::ranges::find_if(Properties, [Name](const auto& Element) { return Element.Name == Name; });
		HERMES_ASSERT_LOG(Property != Properties.end(), L"Trying to update unknown property with name '%s'",
		                  Name.c_str());
		HERMES_ASSERT(sizeof(ValueType) <= GetMaterialPropertySize(Property->Type));
		memcpy(&Property->Value, &Value, sizeof(ValueType));

		IsDirty = true;
	}
}
