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
		std::shared_ptr<Node> Root = Node::Create("ROOT", Mat4::Identity(), "", NodePayloadType::None);

		std::vector<Mesh> Meshes;
	};
}