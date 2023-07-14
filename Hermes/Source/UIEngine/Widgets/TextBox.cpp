#include "TextBox.h"

#include "Core/UTF8/UTF8Utils.h"
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

		auto MaxTextWidth = BoundingBox.Width() - 2 * (OutlineWidth + HorizontalPadding) - CaretWidth;
		auto TextWidth = Math::Min(TextLayout::Measure(CurrentText, FontSize, *Font).X, MaxTextWidth);
		auto TextHeight = Font->GetMaxAscent(FontSize) + Font->GetMaxDescent(FontSize);
		Rect2D TextRect = {
			BoundingBox.Min + Vec2(OutlineWidth + HorizontalPadding, OutlineWidth + VerticalPadding),
			BoundingBox.Min + Vec2(OutlineWidth + HorizontalPadding + TextWidth, OutlineWidth + VerticalPadding + TextHeight)
		};
		Context.DrawText(TextRect, CurrentText, FontSize, Font);

		if (CaretBlinkTimer.GetElapsedTime() > CaretBlinkPeriod)
		{
			CaretBlinkTimer.Reset();
			IsCaretInVisiblePhase = !IsCaretInVisiblePhase;
		}

		if (IsCaretInVisiblePhase)
		{
			auto CursorUTF8Iterator = UTF8::Begin(CurrentText) + CursorPosition;
			auto CursorIterator = CursorUTF8Iterator.ToStringIterator(CurrentText);
			auto WidthOfTextBeforeCursor = TextLayout::Measure(CurrentText.substr(0, CursorIterator - CurrentText.begin()), FontSize, *Font).X;
			Rect2D CaretRect = {
				.Min = {
					BoundingBox.Left() + OutlineWidth + HorizontalPadding + WidthOfTextBeforeCursor,
					TextRect.Min.Y
				},
				.Max = {
					BoundingBox.Left() + OutlineWidth + HorizontalPadding + WidthOfTextBeforeCursor + CaretWidth,
					TextRect.Max.Y
				}
			};
			Context.DrawRectangle(CaretRect, Vec4(1.0f));
		}
	}

	void TextBox::OnKeyDown(KeyCode Key, std::optional<uint32> Codepoint)
	{
		if (Key == KeyCode::ArrowLeft)
			MoveCursor(-1);
		if (Key == KeyCode::ArrowRight)
			MoveCursor(1);

		if (Key == KeyCode::Backspace)
		{
			if (CursorPosition == 0)
				return;

			Erase(CurrentText, UTF8::Begin(CurrentText) + CursorPosition - 1);
			MoveCursor(-1);
		}
		if (Key == KeyCode::Delete)
		{
			if (CursorPosition == UTF8::Length(CurrentText))
				return;

			Erase(CurrentText, UTF8::Begin(CurrentText) + CursorPosition);
		}

		if (!Codepoint.has_value())
			return;

		// NOTE: ignoring special characters (backspace, delete etc.)
		if (Codepoint.value() < 0x20 || Codepoint.value() == 0x7F || (Codepoint.value() >= 0x80 && Codepoint.value() < 0xA0))
			return;

		Insert(CurrentText, UTF8::Begin(CurrentText) + CursorPosition, Codepoint.value());
		MoveCursor(1);
	}

	void TextBox::MoveCursor(int32 Offset)
	{
		CursorPosition = Math::Clamp(0u, static_cast<uint32>(UTF8::Length(CurrentText)), CursorPosition + Offset);
	}
}
