#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	/*
	 * Represents a single UI window. Stores a root object as well as the dimensions
	 * of the window, which cannot be changed during the layout process.
	 */
	class HERMES_API Window
	{
	public:
		Window(std::shared_ptr<Widget> InRootWidget, Vec2ui InDimensions);

		/*
		 * Lays out and draws the widget hierarchy in this window and returns a finished
		 * DrawingContext object that contains all primitives that need to be drawn on a GPU.
		 */
		DrawingContext Draw() const;

		Vec2ui GetDimensions() const;
		void SetDimensions(Vec2ui NewDimensions);

	private:
		std::shared_ptr<Widget> RootWidget;

		Vec2ui Dimensions;
	};
}
