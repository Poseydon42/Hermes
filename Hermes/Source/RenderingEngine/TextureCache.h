#pragma once

#include <unordered_map>

#include "Core/Core.h"
#include "RenderingEngine/Texture.h"

namespace Hermes
{
	class HERMES_API TextureCache
	{
	public:
		TextureCache();

		const Texture& Acquire(StringView Name);

		void Release(StringView Name);

	private:
		struct LoadedTexture
		{
			std::unique_ptr<Texture> Texture;
			uint32 UseCount = 0;
		};

		static constexpr StringView FallbackTextureAssetPath = "/Textures/fallback";

		std::unique_ptr<Texture> FallbackTexture;
		std::unordered_map<String, LoadedTexture> LoadedTextures;

		LoadedTexture* LoadNewTexture(StringView Name);
	};
}
