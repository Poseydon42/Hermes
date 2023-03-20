#pragma once

#include <span>
#include <unordered_map>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "RenderingEngine/Material/Material.h"

namespace Hermes
{
	class HERMES_API MaterialAsset : public Asset
	{
	public:
		virtual bool IsValid() const override;

		virtual size_t GetMemorySize() const override;

		virtual const Resource* GetResource() const override;

	private:
		std::vector<std::pair<String, String>> Shaders;

		// FIXME: this has to be shared pointer for now until Material no longer depends on shared_from_this()
		std::shared_ptr<Material> Material;

		MaterialAsset(String Name, std::span<const std::pair<String, String>> InShaders);

		friend class AssetLoader;
	};
}
