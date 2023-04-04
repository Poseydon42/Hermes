#include "Window.h"

#include "Core/Profiling.h"

namespace Hermes::UI
{
	Window::Window(std::shared_ptr<Widget> InRootWidget, Vec2ui InDimensions)
		: RootWidget(std::move(InRootWidget))
		, Dimensions(InDimensions)
	{
	}

	DrawingContext Window::Draw() const
	{
		HERMES_PROFILE_FUNC();

		DrawingContext Context;
		RootWidget->Draw(Context, { 0, 0 }, Dimensions);

		return Context;
	}

	Vec2ui Window::GetDimensions() const
	{
		return Dimensions;
	}
}
