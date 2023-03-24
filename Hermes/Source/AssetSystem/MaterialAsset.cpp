#include "MaterialAsset.h"

namespace Hermes
{
	DEFINE_ASSET_TYPE(MaterialAsset, Material)

	bool MaterialAsset::IsValid() const
	{
		return (Material != nullptr);
	}

	const Resource* MaterialAsset::GetResource() const
	{
		HERMES_ASSERT(Material);
		return Material.get();
	}

	MaterialAsset::MaterialAsset(String Name, std::span<const std::pair<String, String>> InShaders)
		: Asset(std::move(Name), AssetType::Material)
	{
		String VertexShaderPath, FragmentShaderPath;
		for (const auto& [ShaderType, Path] : InShaders)
		{
			if (ShaderType == "vertex")
				VertexShaderPath = Path;
			if (ShaderType == "fragment")
				FragmentShaderPath = Path;
		}

		if (VertexShaderPath.empty() || FragmentShaderPath.empty())
		{
			HERMES_LOG_WARNING("Material asset %s does not specify vertex or fragment shader and cannot be loaded", GetName().c_str());
			return;
		}

		Material = Material::Create(GetName(), VertexShaderPath, FragmentShaderPath);
	}
}
