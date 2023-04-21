#pragma once

#include <optional>
#include <span>
#include <vector>

#include "AssetSystem/Asset.h"
#include "Core/Core.h"
#include "Math/Vector2.h"

namespace Hermes::UI
{
	struct FontGlyph
	{
		Vec2ui Dimensions;
		std::vector<uint8> Bitmap;
	};

	class HERMES_API Font : public Asset
	{
		HERMES_DECLARE_ASSET(Font)

	public:
		virtual ~Font() override;

		static AssetHandle<Asset> Load(String Name, std::span<const uint8> BinaryData);
		
		std::optional<FontGlyph> RenderGlyph(uint32 CharacterCode) const;

	private:
		Font(String InName, std::span<const uint8> BinaryData);

		struct FontData;
		FontData* FontData;
	};
}
