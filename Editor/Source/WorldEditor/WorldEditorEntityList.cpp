#include "WorldEditorEntityList.h"

#include "UIEngine/Widgets/Label.h"
#include "World/Components/TagComponent.h"

namespace Hermes::Editor
{
	std::shared_ptr<WorldEditorEntityList> WorldEditorEntityList::Create()
	{
		return std::shared_ptr<WorldEditorEntityList>(new WorldEditorEntityList());
	}

	void WorldEditorEntityList::SetWorld(const Hermes::World* NewWorld)
	{
		World = NewWorld;
	}

	void WorldEditorEntityList::OnUpdate(float)
	{
		Children.clear();

		if (!World)
			return;

		// FIXME: use a global UI font
		auto Font = AssetLoader::Load<UI::Font>("/Fonts/arial");
		static constexpr uint32 FontSize = 12;

		for (auto Entity : World->View<>())
		{
			String EntityName;
			if (auto Tag = World->GetComponent<TagComponent>(Entity))
			{
				EntityName = Tag->Tag;
			}
			else
			{
				EntityName = "Unnamed entity";
			}

			AddChild(UI::Label::Create(EntityName, FontSize, Font));
			AddChild(UI::Label::Create("Entity", FontSize, Font));
		}
	}

	WorldEditorEntityList::WorldEditorEntityList()
		: TableContainer(2)
	{
	}
}
