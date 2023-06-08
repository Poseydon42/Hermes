#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/ViewportContainer.h"

namespace Hermes::Editor
{
	class APP_API WorldEditorViewport : public UI::ViewportContainer
	{
	public:
		static std::shared_ptr<WorldEditorViewport> Create();

	private:
		WorldEditorViewport() = default;

		virtual bool OnMouseDown(MouseButton Button) override;
		virtual bool OnMouseUp(MouseButton Button) override;
	};
}
