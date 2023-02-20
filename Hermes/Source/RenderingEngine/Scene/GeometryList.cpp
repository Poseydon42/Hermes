#include "GeometryList.h"

namespace Hermes
{
	GeometryList::GeometryList(std::vector<DrawableMesh> InMeshList)
		: MeshList(std::move(InMeshList))
	{
	}

	const std::vector<DrawableMesh>& GeometryList::GetMeshList() const
	{
		return MeshList;
	}
}
