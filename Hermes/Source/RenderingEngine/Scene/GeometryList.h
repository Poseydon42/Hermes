#pragma once

#include <vector>

#include "Core/Core.h"
#include "Math/Matrix.h"

namespace Hermes
{
	class MaterialInstance;
	class Mesh;

	struct DrawableMesh
	{
		Mat4 TransformationMatrix;

		const Mesh* Mesh;
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
