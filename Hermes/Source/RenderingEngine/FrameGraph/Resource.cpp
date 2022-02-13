#include "Resource.h"

namespace Hermes
{
	SwapchainRelativeDimensions::SwapchainRelativeDimensions()
		: Type(ValueType::Absolute)
	{
		Absolute = { 0, 0 };
	}

	SwapchainRelativeDimensions SwapchainRelativeDimensions::CreateFromAbsoluteDimensions(Vec2ui Dimensions)
	{
		return SwapchainRelativeDimensions(Dimensions);
	}

	SwapchainRelativeDimensions SwapchainRelativeDimensions::CreateFromRelativeDimensions(Vec2 Dimensions)
	{
		return SwapchainRelativeDimensions(Dimensions);
	}

	Vec2ui SwapchainRelativeDimensions::GetAbsoluteDimensions(Vec2ui SwapchainDimensions) const
	{
		if (Type == ValueType::Absolute)
			return Absolute;
		return Vec2ui
		{
			static_cast<uint32>(SwapchainDimensions.X * Relative.X),
			static_cast<uint32>(SwapchainDimensions.Y * Relative.Y)
		};
	}

	bool SwapchainRelativeDimensions::IsRelative() const
	{
		return Type == ValueType::Relative;
	}

	bool SwapchainRelativeDimensions::IsAbsolute() const
	{
		return Type == ValueType::Absolute;
	}

	SwapchainRelativeDimensions::SwapchainRelativeDimensions(Vec2ui AbsoluteDimensions)
		: Type(ValueType::Absolute)
	{
		Absolute = AbsoluteDimensions;
	}

	SwapchainRelativeDimensions::SwapchainRelativeDimensions(Vec2 RelativeDimensions)
		: Type(ValueType::Relative)
	{
		Relative = RelativeDimensions;
	}
}
