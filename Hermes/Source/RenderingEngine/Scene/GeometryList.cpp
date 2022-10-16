#include "GeometryList.h"

namespace Hermes
{
	GeometryList::GeometryList(std::vector<MeshProxy> InMeshList)
		: MeshList(std::move(InMeshList))
	{
	}

	const std::vector<MeshProxy>& GeometryList::GetMeshList() const
	{
		return MeshList;
	}
}
