#pragma once

#include "Core/Core.h"
#include "UIEngine/Font.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	/**
	 * A line of text without wrapping. Supports UTF-8 strings.
	 */
	class HERMES_API Label : public Widget
	{
	public:
		static std::shared_ptr<Label> Create(String InText, uint32 InFontSize, AssetHandle<Font> InFont);

	private:
		String Text;
		uint32 FontSize;
		AssetHandle<Font> Font;

		Label(String InText, uint32 InFontSize, AssetHandle<class Font> InFont);

		virtual Vec2 ComputePreferredSize() const override;

		virtual void Draw(DrawingContext& Context) const override;
	};
}