#include "Image.h"

namespace Hermes::UI
{
	std::shared_ptr<Image> Image::Create(AssetHandle<Texture2D> InTexture)
	{
		return std::shared_ptr<UI::Image>(new UI::Image(std::move(InTexture)));
	}

	const Texture2D* Image::GetTexture() const
	{
		return Texture.get();
	}

	void Image::SetTexture(AssetHandle<Texture2D> NewTexture)
	{
		Texture = std::move(NewTexture);
	}

	Image::Image(AssetHandle<Texture2D> InTexture)
		: Texture(std::move(InTexture))
	{
	}

	Vec2 Image::ComputePreferredSize() const
	{
		return Vec2(Texture->GetDimensions());
	}

	void Image::Draw(DrawingContext& Context) const
	{
		Context.DrawTexturedRectangle(BoundingBox, Texture);
	}
}
