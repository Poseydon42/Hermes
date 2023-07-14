#pragma once

#include "Core/Core.h"
#include "Core/Misc/Timer.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	class HERMES_API TextBox : public Widget
	{
	public:
		static std::shared_ptr<TextBox> Create(AssetHandle<Font> InFont, uint32 InFontSize, float InPreferredWidth);

		const String& GetText() const;

	private:
		AssetHandle<Font> Font;
		uint32 FontSize = 11;
		float PreferredWidth = 0.0f;

		String CurrentText;

		uint32 CursorPosition = 0;

		mutable Timer CaretBlinkTimer;
		mutable bool IsCaretInVisiblePhase = true;
		static constexpr float CaretBlinkPeriod = 0.7f;

		static constexpr float CornerRadius = 2.0f;
		static constexpr float VerticalPadding = 3.0f;
		static constexpr float HorizontalPadding = 3.0f;
		static constexpr float OutlineWidth = 2.0f;
		static constexpr float CaretWidth = 1.0f;
		static constexpr Vec4 BackgroundColor = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		static constexpr Vec4 OutlineColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

		TextBox(AssetHandle<class Font> InFont, uint32 InFontSize, float InPreferredWidth);

		virtual Vec2 ComputePreferredSize() const override;

		virtual void Draw(DrawingContext& Context) const override;

		virtual void OnKeyDown(KeyCode Key, std::optional<uint32> Codepoint) override;

		void MoveCursor(int32 Offset);
	};
}
