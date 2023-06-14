#pragma once

#include "Core/Core.h"
#include "UIEngine/Widgets/Containers/Container.h"

namespace Hermes::UI
{
	class HERMES_API TableContainer : public Container
	{
	public:
		static std::shared_ptr<TableContainer> Create(uint32 InColumnCount);

		void SetColumnCount(uint32 NewColumnCount);
		uint32 GetColumnCount() const;

		uint32 GetRowCount() const;

	protected:
		explicit TableContainer(uint32 InColumnCount);
		
		uint32 ColumnCount;

		virtual Vec2 ComputePreferredSize() const override;
		virtual void Layout() override;
		virtual void Draw(DrawingContext& Context) const override;

		std::vector<float> ComputePreferredColumnWidths() const;
		std::vector<float> ComputePreferredRowHeights() const;
	};
}
