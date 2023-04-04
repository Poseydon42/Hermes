#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/ContainerWidget.h"

namespace Hermes::UI
{
	class HERMES_API VerticalContainerWidget : public ContainerWidget
	{
	public:
		virtual ~VerticalContainerWidget() override = default;

		VerticalContainerWidget(VerticalContainerWidget&&) = default;
		VerticalContainerWidget& operator=(VerticalContainerWidget&&) = default;

		static std::shared_ptr<VerticalContainerWidget> Create(std::shared_ptr<Widget> InParent);

		virtual Vec2ui GetDimensions() const override;

		virtual void Draw(DrawingContext& Context, Vec2ui AbsoluteLocation, Vec2ui MaxDimensions) const override;

	private:
		explicit VerticalContainerWidget(std::shared_ptr<Widget> InParent);
	};
}
