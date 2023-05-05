#include "Label.h"

#include "UIEngine/TextLayout.h"

namespace Hermes::UI
{
	std::shared_ptr<Label> Label::Create(String InText, uint32 InFontSize, AssetHandle<UI::Font> InFont)
	{
		return std::shared_ptr<Label>(new Label(std::move(InText), InFontSize, std::move(InFont)));
	}

	Vec2 Label::ComputeMinimumSize() const
	{
		return TextLayout::Measure(Text, FontSize, *Font);
	}

	void Label::Draw(DrawingContext& Context) const
	{
		Context.DrawText(BoundingBox, Text, FontSize, Font);
	}

	Label::Label(String InText, uint32 InFontSize, AssetHandle<class Font> InFont)
		: Text(std::move(InText))
		, FontSize(InFontSize)
		, Font(std::move(InFont))
	{
	}
}
