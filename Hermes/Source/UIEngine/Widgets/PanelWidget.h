#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	/*
	 * A simple panel filled with a single color
	 */
	class HERMES_API PanelWidget : public Widget
	{
	public:
		virtual ~PanelWidget() override = default;

		PanelWidget(PanelWidget&&) = default;
		PanelWidget& operator=(PanelWidget&&) = default;

		static std::shared_ptr<PanelWidget> Create(std::shared_ptr<Widget> InParent, Vec2ui InLocation, Vec2ui InDimensions, Vec3 InColor);

		virtual Vec2ui GetDimensions() const override;

		virtual void Draw(DrawingContext& Context) const override;

	private:
		PanelWidget(std::shared_ptr<Widget> InParent, Vec2ui InLocation, Vec2ui InDimensions, Vec3 InColor);

		Vec2ui Dimensions;
		Vec3 Color;
	};
}