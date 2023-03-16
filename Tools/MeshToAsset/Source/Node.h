#pragma once

#include <algorithm>
#include <vector>

#include "Core/Core.h"
#include "Math/Matrix.h"

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
	class HERMES_API Node : public std::enable_shared_from_this<Node>
	{
	public:
		static std::shared_ptr<Node> Create(String InNodeName, Mat4 InTransformationMatrix, String InPayloadName, NodePayloadType InPayloadType);

		StringView GetNodeName() const;
		Mat4 GetLocalTransformationMatrix() const;

		Mat4 ComputeGlobalTransformationMatrix() const;

		StringView GetPayloadName() const;
		NodePayloadType GetPayloadType() const;

		const std::vector<std::shared_ptr<Node>>& GetChildren() const;

		void SetParent(std::shared_ptr<Node> NewParent);
		void AddChild(std::shared_ptr<Node> NewChild);

		bool RemoveChild(StringView ChildNodeName);

	private:
		Node(String InNodeName, Mat4 InTransformationMatrix, String InPayloadName, NodePayloadType InPayloadType);

		String NodeName;
		Mat4 TransformationMatrix;

		String PayloadName;
		NodePayloadType PayloadType;

		std::shared_ptr<Node> Parent = nullptr;
		std::vector<std::shared_ptr<Node>> Children;

		friend class std::shared_ptr<Node>;
	};
}
