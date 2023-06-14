#include "TableContainer.h"

#include <numeric>

namespace Hermes::UI
{
	std::shared_ptr<TableContainer> TableContainer::Create(uint32 InColumnCount)
	{
		return std::shared_ptr<TableContainer>(new TableContainer(InColumnCount));
	}

	void TableContainer::SetColumnCount(uint32 NewColumnCount)
	{
		ColumnCount = NewColumnCount;
	}

	uint32 TableContainer::GetColumnCount() const
	{
		return ColumnCount;
	}

	uint32 TableContainer::GetRowCount() const
	{
		return (static_cast<uint32>(GetChildrenCount()) + ColumnCount - 1) / ColumnCount;
	}

	TableContainer::TableContainer(uint32 InColumnCount)
		: ColumnCount(InColumnCount)
	{
	}

	Vec2 TableContainer::ComputePreferredSize() const
	{
		std::vector<float> ColumnWidth = ComputePreferredColumnWidths();
		std::vector<float> RowHeight = ComputePreferredRowHeights();
		
		auto Width = std::accumulate(ColumnWidth.begin(), ColumnWidth.end(), 0.0f);
		auto Height = std::accumulate(RowHeight.begin(), RowHeight.end(), 0.0f);

		return { Width, Height };
	}

	void TableContainer::Layout()
	{
		uint32 RowIndex = 0;
		uint32 ColumnIndex = 0;

		auto PreferredColumnWidth = ComputePreferredColumnWidths();
		auto PreferredRowHeight = ComputePreferredRowHeights();

		std::vector<float> RowOffsets(GetRowCount());
		RowOffsets[0] = 0.0f;
		for (uint32 Index = 1; Index < RowOffsets.size(); Index++)
		{
			RowOffsets[Index] = RowOffsets[Index - 1] + PreferredRowHeight[Index - 1];
		}

		std::vector<float> ColumnOffsets(ColumnCount);
		ColumnOffsets[0] = 0.0f;
		for (uint32 Index = 1; Index < ColumnOffsets.size(); Index++)
		{
			ColumnOffsets[Index] = ColumnOffsets[Index - 1] + PreferredColumnWidth[Index - 1];
		}

		/*
		 * FIXME: this is a very dumb way of doing it - basically provide first columns with their preferred horizontal size
		 * and not care about the rest.
		 */
		ForEachChild([&](Widget& Child)
		{
			auto TopLeftCorner = BoundingBox.Min + Vec2(ColumnOffsets[ColumnIndex], RowOffsets[RowIndex]);
			auto BottomRightCorner = TopLeftCorner + Child.ComputePreferredSize();

			Rect2D ChildBoundingBox = Rect2D(TopLeftCorner, BottomRightCorner).Intersect(BoundingBox);
			Child.SetBoundingBox(ChildBoundingBox);

			Child.Layout();

			if (++ColumnIndex == ColumnCount)
			{
				ColumnIndex = 0;
				RowIndex++;
			}
		});
	}

	void TableContainer::Draw(DrawingContext& Context) const
	{
		ForEachChild([&](const Widget& Child) { Child.Draw(Context); });
	}

	std::vector<float> TableContainer::ComputePreferredColumnWidths() const
	{
		std::vector<float> Result(ColumnCount);

		uint32 ColumnIndex = 0;
		ForEachChild([&](const Widget& Child)
		{
			auto ChildSize = Child.ComputePreferredSize();
			Result[ColumnIndex] = Math::Max(Result[ColumnIndex], ChildSize.X);

			if (++ColumnIndex == ColumnCount)
				ColumnIndex = 0;
		});

		return Result;
	}

	std::vector<float> TableContainer::ComputePreferredRowHeights() const
	{
		uint32 RowCount = GetRowCount();
		std::vector<float> Result(RowCount);

		uint32 ColumnIndex = 0;
		uint32 RowIndex = 0;
		ForEachChild([&](const Widget& Child)
		{
			auto ChildSize = Child.ComputePreferredSize();
			Result[RowIndex] = Math::Max(Result[RowIndex], ChildSize.Y);

			if (++ColumnIndex == ColumnCount)
			{
				ColumnIndex = 0;
				RowIndex++;
			}
		});

		return Result;
	}
}
