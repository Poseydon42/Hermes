#include "TextureCache.h"

#include "AssetSystem/AssetLoader.h"

namespace Hermes
{
	static String ComposeFullName(StringView Name, bool IsSRGB)
	{
		// FIXME: technically on Linux-based systems '?' is permitted in a file name,
		// so we might need to find a better way to implement this to avoid any name collisions
		return String(Name) + (IsSRGB ? "?srgb" : "?linear");
	}

	TextureCache::TextureCache()
	{
		auto FallbackTextureAsset = AssetLoader::Load(FallbackTextureAssetPath);
		HERMES_ASSERT_LOG(FallbackTextureAsset || !FallbackTextureAsset->IsValid() || FallbackTextureAsset->GetType() != AssetType::Image, "Could not load fallback texture asset %s", FallbackTextureAssetPath.data());

		FallbackTexture = Texture::CreateFromAsset(*Asset::As<ImageAsset>(FallbackTextureAsset), true);
	}

	const Texture& TextureCache::Acquire(StringView Name, bool IsSRGB)
	{
		auto FullName = ComposeFullName(Name, IsSRGB);
		if (auto MaybeTexture = LoadedTextures.find(FullName); MaybeTexture != LoadedTextures.end())
		{
			auto& Texture = MaybeTexture->second;
			Texture.UseCount++;
			return *Texture.Texture;
		}

		auto* Texture = LoadNewTexture(Name, IsSRGB);
		if (!Texture)
		{
			HERMES_LOG_WARNING("Could not load texture %s (color space: %s), using fallback texture instead", Name.data(), (IsSRGB ? "sRGB" : "linear"));
			return *FallbackTexture;
		}
		return *Texture->Texture;
	}

	void TextureCache::Release(StringView Name, bool IsSRGB)
	{
		auto FullName = ComposeFullName(Name, IsSRGB);

		auto MaybeTexture = LoadedTextures.find(FullName);
		if (MaybeTexture != LoadedTextures.end())
		{
			HERMES_LOG_WARNING("Trying to release texture %s (color space: %s) that was not previously loaded by this texture cache", Name.data(), (IsSRGB ? "sRGB" : "linear"));
			return;
		}

		if (--MaybeTexture->second.UseCount == 0)
		{
			LoadedTextures.erase(MaybeTexture);
		}
	}

	TextureCache::LoadedTexture* TextureCache::LoadNewTexture(StringView Name, bool IsSRGB)
	{
		auto Asset = AssetLoader::Load(Name);
		if (!Asset || !Asset->IsValid() || Asset->GetType() != AssetType::Image)
		{
			return nullptr;
		}

		auto ImageAsset = Asset::As<class ImageAsset>(Asset);
		auto Texture = Texture::CreateFromAsset(*ImageAsset, IsSRGB); // FIXME: UseAsSRGB/EnableMipMaps should also be exposed to the user

		LoadedTexture LoadedTexture = { std::move(Texture), 1 };

		auto FullName = ComposeFullName(Name, IsSRGB);
		LoadedTextures[FullName] = std::move(LoadedTexture);
		return &LoadedTextures.at(FullName);
	}
}
