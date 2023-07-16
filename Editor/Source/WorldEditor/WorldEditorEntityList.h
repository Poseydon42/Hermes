#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/TableContainer.h"
#include "World/World.h"

namespace Hermes::Editor
{
	class APP_API WorldEditorEntityList : public UI::TableContainer
	{
	public:
		static std::shared_ptr<WorldEditorEntityList> Create();

		void SetWorld(const World* NewWorld);

	private:
		WorldEditorEntityList();

		const World* World = nullptr;

		virtual void OnUpdate(float DeltaTime) override;
	};
}
