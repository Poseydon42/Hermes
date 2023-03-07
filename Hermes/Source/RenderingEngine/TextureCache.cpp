#include "TextureCache.h"

#include "ApplicationCore/GameLoop.h"
#include "AssetSystem/AssetLoader.h"

namespace Hermes
{
	TextureCache::TextureCache()
	{
		auto& AssetCache = GGameLoop->GetAssetCache();

		auto MaybeFallbackTextureAsset = AssetCache.Get<ImageAsset>(String(FallbackTextureAssetPath));
		HERMES_ASSERT_LOG(MaybeFallbackTextureAsset.has_value() && MaybeFallbackTextureAsset.value() != nullptr, "Could not load fallback texture asset %s", FallbackTextureAssetPath.data());

		FallbackTexture = Texture::CreateFromAsset(Asset::As<ImageAsset>(*MaybeFallbackTextureAsset.value()), true);
	}

	const Texture& TextureCache::Acquire(StringView Name)
	{
		if (auto MaybeTexture = LoadedTextures.find(String(Name)); MaybeTexture != LoadedTextures.end())
		{
			auto& Texture = MaybeTexture->second;
			Texture.UseCount++;
			return *Texture.Texture;
		}

		auto* Texture = LoadNewTexture(Name);
		if (!Texture)
		{
			HERMES_LOG_WARNING("Could not load texture %s (color space: %s), using fallback texture instead", Name.data());
			return *FallbackTexture;
		}
		return *Texture->Texture;
	}

	void TextureCache::Release(StringView Name)
	{
		auto MaybeTexture = LoadedTextures.find(String(Name));
		if (MaybeTexture != LoadedTextures.end())
		{
			HERMES_LOG_WARNING("Trying to release texture %s (color space: %s) that was not previously loaded by this texture cache", Name.data());
			return;
		}

		if (--MaybeTexture->second.UseCount == 0)
		{
			LoadedTextures.erase(MaybeTexture);
		}
	}

	TextureCache::LoadedTexture* TextureCache::LoadNewTexture(StringView Name)
	{
		auto& AssetCache = GGameLoop->GetAssetCache();

		auto MaybeAsset = AssetCache.Get<ImageAsset>(String(Name));
		if (!MaybeAsset.has_value() || MaybeAsset.value() == nullptr)
		{
			return nullptr;
		}

		const auto& Asset = *MaybeAsset.value();
		auto Texture = Texture::CreateFromAsset(Asset);

		LoadedTexture LoadedTexture = { std::move(Texture), 1 };
		
		return &LoadedTextures.insert(std::make_pair(String(Name), std::move(LoadedTexture))).first->second;
	}
}
