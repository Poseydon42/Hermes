#pragma once

#include <algorithm>
#include <vector>

#include "Core/Core.h"

namespace Hermes::Tools
{
	enum class NodePayloadType
	{
		None,

		Mesh
	};

	/*
	 * A class that represents a single-linked node of a model hierarchy tree.
	 *
	 * The node itself doesn't store any payload, but rather the type and name of the
	 * payload that can then be retrieved from the InputFileReader instance using its name.
	 *
	 * Each node has two individual string properties - the node name and the payload name.
	 * The node name is only used to perform operations on the hierarchy tree like erasing nodes,
	 * whereas the payload name is used to retrieve the payload data from the InputFileReader.
	 * This is done because two separate nodes can be pointing to the same payload (e.g. two
	 * different nodes pointing to the same mesh payload).
	 */
	class HERMES_API Node
	{
	public:
		Node(String InNodeName, String InPayloadName, NodePayloadType InPayloadType);

		StringView GetNodeName() const;

		StringView GetPayloadName() const;
		NodePayloadType GetPayloadType() const;

		std::vector<Node>& GetChildren();
		const std::vector<Node>& GetChildren() const;

		void AddChild(Node NewChild);

		bool RemoveChild(StringView ChildNodeName);

	private:
		String NodeName;

		String PayloadName;
		NodePayloadType PayloadType;

		std::vector<Node> Children;
	};
}
