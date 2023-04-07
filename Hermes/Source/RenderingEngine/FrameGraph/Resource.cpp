#include "Resource.h"

namespace Hermes
{
	ViewportRelativeDimensions::ViewportRelativeDimensions()
		: Type(ValueType::Absolute)
	{
		Absolute = { 0, 0 };
	}

	ViewportRelativeDimensions ViewportRelativeDimensions::CreateFromAbsoluteDimensions(Vec2ui Dimensions)
	{
		return ViewportRelativeDimensions(Dimensions);
	}

	ViewportRelativeDimensions ViewportRelativeDimensions::CreateFromRelativeDimensions(Vec2 Dimensions)
	{
		return ViewportRelativeDimensions(Dimensions);
	}

	Vec2ui ViewportRelativeDimensions::GetAbsoluteDimensions(Vec2ui SwapchainDimensions) const
	{
		if (Type == ValueType::Absolute)
			return Absolute;
		return Vec2ui
		{
			static_cast<uint32>(SwapchainDimensions.X * Relative.X),
			static_cast<uint32>(SwapchainDimensions.Y * Relative.Y)
		};
	}

	bool ViewportRelativeDimensions::IsRelative() const
	{
		return Type == ValueType::Relative;
	}

	bool ViewportRelativeDimensions::IsAbsolute() const
	{
		return Type == ValueType::Absolute;
	}

	ViewportRelativeDimensions::ViewportRelativeDimensions(Vec2ui AbsoluteDimensions)
		: Type(ValueType::Absolute)
	{
		Absolute = AbsoluteDimensions;
	}

	ViewportRelativeDimensions::ViewportRelativeDimensions(Vec2 RelativeDimensions)
		: Type(ValueType::Relative)
	{
		Relative = RelativeDimensions;
	}
}
