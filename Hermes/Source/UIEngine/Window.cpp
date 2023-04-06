#include "Window.h"

#include "Core/Profiling.h"

namespace Hermes::UI
{
	Window::Window(std::shared_ptr<Widget> InRootWidget, Vec2ui InDimensions)
		: RootWidget(std::move(InRootWidget))
	{
		SetDimensions(InDimensions);
	}

	DrawingContext Window::Draw() const
	{
		HERMES_PROFILE_FUNC();

		DrawingContext Context;
		RootWidget->Draw(Context, { { 0, 0 }, Vec2(Dimensions) });

		return Context;
	}

	Vec2ui Window::GetDimensions() const
	{
		return Dimensions;
	}

	void Window::SetDimensions(Vec2ui NewDimensions)
	{
		auto MinimumDimensions = Vec2ui(RootWidget->ComputeMinimumDimensions());
		NewDimensions.X = Math::Max(MinimumDimensions.X, NewDimensions.X);
		NewDimensions.Y = Math::Max(MinimumDimensions.Y, NewDimensions.Y);
		Dimensions = NewDimensions;
	}
}
