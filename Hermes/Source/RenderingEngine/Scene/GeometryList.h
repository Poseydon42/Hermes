#pragma once

#include <vector>

#include "Core/Core.h"
#include "RenderingEngine/Scene/SceneProxies.h"

namespace Hermes
{
	class HERMES_API GeometryList
	{
	public:
		GeometryList(std::vector<MeshProxy> InMeshList);

		const std::vector<MeshProxy>& GetMeshList() const;

	private:
		std::vector<MeshProxy> MeshList;
	};
}
