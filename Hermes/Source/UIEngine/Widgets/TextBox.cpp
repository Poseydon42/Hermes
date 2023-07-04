#include "TextBox.h"

namespace Hermes::UI
{
	std::shared_ptr<TextBox> TextBox::Create(AssetHandle<UI::Font> InFont, uint32 InFontSize, float InPreferredWidth)
	{
		return std::shared_ptr<TextBox>(new TextBox(std::move(InFont), InFontSize, InPreferredWidth));
	}

	const String& TextBox::GetText() const
	{
		return CurrentText;
	}

	TextBox::TextBox(AssetHandle<UI::Font> InFont, uint32 InFontSize, float InPreferredWidth)
		: Font(std::move(InFont))
		, FontSize(InFontSize)
		, PreferredWidth(InPreferredWidth)
	{
	}

	Vec2 TextBox::ComputePreferredSize() const
	{
		return {
			PreferredWidth + 2 * HorizontalPadding + 2 * OutlineWidth,
			Font->GetMaxAscent(FontSize) + Font->GetMaxDescent(FontSize) + 2 * VerticalPadding + 2 * OutlineWidth
		};
	}

	void TextBox::Draw(DrawingContext& Context) const
	{
		Context.DrawRectangle(BoundingBox, BackgroundColor, OutlineColor, OutlineWidth, CornerRadius);

		Rect2D TextRect = {
			BoundingBox.Min + Vec2(OutlineWidth + HorizontalPadding, OutlineWidth + VerticalPadding),
			BoundingBox.Max - Vec2(OutlineWidth + HorizontalPadding, OutlineWidth + VerticalPadding)
		};
		Context.DrawText(TextRect, CurrentText, FontSize, Font);
	}

	void TextBox::OnKeyDown(KeyCode, std::optional<uint32> Codepoint)
	{
		if (!Codepoint.has_value())
			return;

		// NOTE: ignoring special characters (backspace, delete etc.)
		if (Codepoint.value() < 0x20 || Codepoint.value() == 0x7F || (Codepoint.value() >= 0x80 && Codepoint.value() < 0xA0))
			return;

		// FIXME: add Unicode support later
		if (Codepoint.value() > 0xFF)
			return;

		auto Character = static_cast<char>(Codepoint.value());
		CurrentText += Character;
	}
}
