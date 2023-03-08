#pragma once

#include <vector>

#include "Core/Core.h"
#include "Math/Matrix.h"

namespace Hermes
{
	class MaterialInstance;
	class MeshResource;

	struct DrawableMesh
	{
		Mat4 TransformationMatrix;

		const MeshResource* Mesh;
		const MaterialInstance* Material;
	};

	class HERMES_API GeometryList
	{
	public:
		explicit GeometryList(std::vector<DrawableMesh> InMeshList);

		const std::vector<DrawableMesh>& GetMeshList() const;

	private:
		std::vector<DrawableMesh> MeshList;
	};
}
