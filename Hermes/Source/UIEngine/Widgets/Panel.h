#pragma once

#include "Core/Core.h"
#include "Math/Vector.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	/*
	 * A simple panel that fills all the space allocated to it with a single color.
	 */
	class HERMES_API Panel : public Widget
	{
	public:
		virtual ~Panel() override = default;

		Panel(Panel&&) = default;
		Panel& operator=(Panel&&) = default;

		static std::shared_ptr<Panel> Create(Vec2 InPreferredSize, Vec4 InColor);

	protected:
		Panel(Vec2 InPreferredSize, Vec4 InColor);

		Vec2 PreferredSize;
		Vec4 Color;

		virtual Vec2 ComputePreferredSize() const override;

		virtual void Draw(DrawingContext& Context) const override;
	};
}