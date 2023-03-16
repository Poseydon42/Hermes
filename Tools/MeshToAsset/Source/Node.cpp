#include "Node.h"

namespace Hermes::Tools
{
	Node::Node(Node* InParent, String InNodeName, Mat4 InTransformationMatrix, String InPayloadName, NodePayloadType InPayloadType)
		: NodeName(std::move(InNodeName))
		, TransformationMatrix(InTransformationMatrix)
		, PayloadName(std::move(InPayloadName))
		, PayloadType(InPayloadType)
		, Parent(InParent)
	{
	}

	StringView Node::GetNodeName() const
	{
		return NodeName;
	}

	Mat4 Node::GetLocalTransformationMatrix() const
	{
		return TransformationMatrix;
	}

	Mat4 Node::ComputeGlobalTransformationMatrix() const
	{
		auto ParentTransformationMatrix = Mat4::Identity();
		if (Parent)
			ParentTransformationMatrix = Parent->ComputeGlobalTransformationMatrix();
		return TransformationMatrix * ParentTransformationMatrix;
	}

	StringView Node::GetPayloadName() const
	{
		return PayloadName;
	}

	NodePayloadType Node::GetPayloadType() const
	{
		return PayloadType;
	}

	std::vector<Node>& Node::GetChildren()
	{
		return Children;
	}

	const std::vector<Node>& Node::GetChildren() const
	{
		return Children;
	}

	void Node::SetParent(Node* NewParent)
	{
		Parent = NewParent;
	}

	void Node::AddChild(Node NewChild)
	{
		Children.push_back(std::move(NewChild));
	}

	bool Node::RemoveChild(StringView ChildNodeName)
	{
		if (auto MaybeChild = std::ranges::find_if(Children, [&](const auto& Element) { return Element.GetNodeName() == ChildNodeName; });
			MaybeChild != Children.end())
		{
			Children.erase(MaybeChild);
			return true;
		}
		return false;
	}
}
