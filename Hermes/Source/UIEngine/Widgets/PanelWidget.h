#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	/*
	 * A simple panel that fills all the space allocated to it with a single color.
	 */
	class HERMES_API PanelWidget : public Widget
	{
	public:
		virtual ~PanelWidget() override = default;

		PanelWidget(PanelWidget&&) = default;
		PanelWidget& operator=(PanelWidget&&) = default;

		static std::shared_ptr<PanelWidget> Create(Vec2 InMinimumDimensions, Vec3 InColor);

		virtual void Draw(DrawingContext& Context, Rect2D AvailableRect) const override;

	protected:
		PanelWidget(Vec2 InMinimumDimensions, Vec3 InColor);

		Vec2 MinimumDimensions;
		Vec3 Color;

		virtual Vec2 ComputeMinimumDimensions() const override;
	};
}