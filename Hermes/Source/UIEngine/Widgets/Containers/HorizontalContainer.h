#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/Container.h"

namespace Hermes::UI
{
	/**
	 * Similar to vertical container, but lays out its children in the horizontal, left to right order
	 */
	class HERMES_API HorizontalContainer : public Container
	{
	public:
		static std::shared_ptr<HorizontalContainer> Create();

	protected:
		HorizontalContainer() = default;

		virtual Vec2 ComputePreferredSize() const override;

		virtual void Layout() override;

		virtual void Draw(DrawingContext& Context) const override;
	};
}
