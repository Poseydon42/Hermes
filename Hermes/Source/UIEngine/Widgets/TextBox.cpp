#include "TextBox.h"

#include "UIEngine/TextLayout.h"

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

		auto MaxTextWidth = BoundingBox.Width() - 2 * (OutlineWidth + HorizontalPadding + CaretSpacing) - CaretWidth;
		auto TextWidth = Math::Min(TextLayout::Measure(CurrentText, FontSize, *Font).X, MaxTextWidth);
		auto TextHeight = Font->GetMaxAscent(FontSize) + Font->GetMaxDescent(FontSize);
		auto LeftHorizontalPaddingCoefficient = (CurrentText.empty() ? 0.0f : 1.0f);
		Rect2D TextRect = {
			BoundingBox.Min + Vec2(OutlineWidth + LeftHorizontalPaddingCoefficient * HorizontalPadding, OutlineWidth + VerticalPadding),
			BoundingBox.Min + Vec2(OutlineWidth + LeftHorizontalPaddingCoefficient * HorizontalPadding + TextWidth, OutlineWidth + VerticalPadding + TextHeight)
		};
		Context.DrawText(TextRect, CurrentText, FontSize, Font);

		Rect2D CaretRect = {
			.Min = {
				TextRect.Max.X + CaretSpacing,
				TextRect.Min.Y
			},
			.Max = {
				TextRect.Max.X + CaretSpacing + CaretWidth,
				TextRect.Max.Y
			}
		};
		Context.DrawRectangle(CaretRect, Vec4(1.0f));
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
