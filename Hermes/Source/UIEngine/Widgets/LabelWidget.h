#pragma once

#include "Core/Core.h"
#include "UIEngine/Font.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	class HERMES_API LabelWidget : public Widget
	{
	public:
		static std::shared_ptr<LabelWidget> Create(std::shared_ptr<Widget> InParent, String InText, AssetHandle<Font> InFont);

		virtual Vec2 ComputeMinimumDimensions() const override;

		virtual void Draw(DrawingContext& Context, Rect2D AvailableRect) const override;

	private:
		String Text;
		AssetHandle<Font> Font;

		LabelWidget(std::shared_ptr<Widget> InParent, String InText, AssetHandle<class Font> InFont);
	};
}