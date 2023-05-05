#include "Panel.h"

namespace Hermes::UI
{
	std::shared_ptr<Panel> Panel::Create(Vec2 InMinimumSize, Vec3 InColor)
	{
		return std::shared_ptr<Panel>(new Panel(InMinimumSize, InColor));
	}

	void Panel::Draw(DrawingContext& Context) const
	{
		Context.DrawRectangle(BoundingBox, Color);
	}

	Panel::Panel(Vec2 InMinimumSize, Vec3 InColor)
		: MinimumSize(InMinimumSize)
		, Color(InColor)
	{
	}

	Vec2 Panel::ComputeMinimumSize() const
	{
		return MinimumSize;
	}
}
