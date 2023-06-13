#include "Panel.h"

namespace Hermes::UI
{
	std::shared_ptr<Panel> Panel::Create(Vec2 InPreferredSize, Vec4 InColor)
	{
		return std::shared_ptr<Panel>(new Panel(InPreferredSize, InColor));
	}

	void Panel::Draw(DrawingContext& Context) const
	{
		Context.DrawRectangle(BoundingBox, Color);
	}

	Panel::Panel(Vec2 InPreferredSize, Vec4 InColor)
		: PreferredSize(InPreferredSize)
		, Color(InColor)
	{
	}

	Vec2 Panel::ComputePreferredSize() const
	{
		return PreferredSize;
	}
}
