#include "MaterialInstance.h"

#include "JSON/JSONParser.h"
#include "RenderingEngine/DescriptorAllocator.h"
#include "RenderingEngine/Renderer.h"
#include "RenderingEngine/Texture.h"
#include "Vulkan/Device.h"

namespace Hermes
{
	static std::optional<std::shared_ptr<Material>> CreateMaterialFromJSON(const JSONObject& Root)
	{
		if (!Root.Contains("shaders") || !Root.Get("shaders").Is(JSONValueType::Array))
		{
			HERMES_LOG_WARNING(R"(Failed to create material instance: no "shaders" value is present inside the root object or its type is not array.)");
			return {};
		}

		String VertexShaderPath, FragmentShaderPath;

		const auto& ShaderArray = Root.Get("shaders").AsArray();
		for (const auto& ShaderArrayValue : ShaderArray)
		{
			if (!ShaderArrayValue.Is(JSONValueType::Object))
				continue;

			const auto& Shader = ShaderArrayValue.AsObject();
			if (!Shader.Contains("type") || !Shader.Get("type").Is(JSONValueType::String))
				continue;
			if (!Shader.Contains("path") || !Shader.Get("path").Is(JSONValueType::String))
				continue;

			auto ShaderType = Shader.Get("type").AsString();
			auto ShaderPath = Shader.Get("path").AsString();
			if (ShaderType == "vertex")
				VertexShaderPath = ShaderPath;
			if (ShaderType == "fragment")
				FragmentShaderPath = ShaderPath;
		}

		if (VertexShaderPath.empty() || FragmentShaderPath.empty())
		{
			HERMES_LOG_WARNING("Failed to create material instance: either vertex or fragment shader is not specified");
			return {};
		}

		auto Material = Material::Create(VertexShaderPath, FragmentShaderPath);
		if (!Material)
		{
			HERMES_LOG_WARNING("Failed to create material instance: material is null");
			return {};
		}

		return Material;
	}

	static void SetPropertiesFromJSON(MaterialInstance& Instance, const JSONObject& PropertiesObject)
	{
		for (const auto& Property : PropertiesObject)
		{
			const auto& PropertyName = Property.first;
			if (Instance.GetBaseMaterial().FindProperty(PropertyName) == nullptr)
				HERMES_LOG_WARNING("Ignoring unknown property %s", PropertyName.c_str());

			// FIXME: properly parse other types of properties (at the moment we assume that all properties are floats
			HERMES_ASSERT(Property.second.Is(JSONValueType::Number));

			auto Value = static_cast<float>(Property.second.AsNumber());
			Instance.SetNumericProperty(PropertyName, Value);
		}
	}

	std::optional<std::unique_ptr<MaterialInstance>> MaterialInstance::CreateFromJSON(StringView JSON)
	{
		auto MaybeRoot = JSONParser::FromString(JSON);
		if (!MaybeRoot.has_value())
		{
			HERMES_LOG_WARNING("Failed to create material instance: JSON parsing failed.");
			return {};
		}

		const auto& Root = *MaybeRoot.value();

		auto MaybeMaterial = CreateMaterialFromJSON(Root);
		if (!MaybeMaterial.has_value())
		{
			return {};
		}

		auto Material = MaybeMaterial.value();
		auto Instance = Material->CreateInstance();
		HERMES_ASSERT(Instance);

		if (!Root.Contains("properties") || !Root.Get("properties").Is(JSONValueType::Object))
			return Instance;

		const auto& Properties = Root.Get("properties").AsObject();
		SetPropertiesFromJSON(*Instance, Properties);

		return Instance;
	}


	void MaterialInstance::SetTextureProperty(const String& Name, const Texture& Value)
	{
		auto* Property = BaseMaterial->FindProperty(Name);
		HERMES_ASSERT(Property);

		DescriptorSet->UpdateWithImageAndSampler(Property->Binding, 0, Value.GetDefaultView(),
		                                         Renderer::Get().GetDefaultSampler(),
		                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
		return *BaseMaterial;
	}

	const Vulkan::DescriptorSet& MaterialInstance::GetMaterialDescriptorSet() const
	{
		return *DescriptorSet;
	}

	MaterialInstance::MaterialInstance(std::shared_ptr<const Material> Material, size_t UniformBufferSize)
		: HasUniformBuffer(UniformBufferSize > 0)
		, BaseMaterial(std::move(Material))
	{
		CPUBuffer.resize(UniformBufferSize);

		auto& Device = Renderer::Get().GetActiveDevice();
		auto& DescriptorAllocator = Renderer::Get().GetDescriptorAllocator();

		DescriptorSet = DescriptorAllocator.Allocate(BaseMaterial->GetDescriptorSetLayout());

		if (HasUniformBuffer)
		{
			UniformBuffer = Device.CreateBuffer(UniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
			DescriptorSet->UpdateWithBuffer(0, 0, *UniformBuffer, 0, static_cast<uint32>(UniformBuffer->GetSize()));
			IsDirty = true;
		}		
	}
}
