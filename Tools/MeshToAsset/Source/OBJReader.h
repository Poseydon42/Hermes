#pragma once

#include "InputFileReader.h"
#include "Node.h"

namespace Hermes::Tools
{
	class OBJReader : public IInputFileReader
	{
	public:
		virtual bool Read(StringView Path) override;

		virtual const Node& GetRootNode() const override;

		virtual std::optional<const Mesh*> GetMesh(StringView MeshName) const override;

	private:
		Node Root = Node("ROOT", "", NodePayloadType::None);

		std::vector<Mesh> Meshes;
	};
}