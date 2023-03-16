#include "Node.h"

namespace Hermes::Tools
{
	Node::Node(String InNodeName, Mat4 InTransformationMatrix, String InPayloadName, NodePayloadType InPayloadType)
		: NodeName(std::move(InNodeName))
		, TransformationMatrix(InTransformationMatrix)
		, PayloadName(std::move(InPayloadName))
		, PayloadType(InPayloadType)
	{
	}

	std::shared_ptr<Node> Node::Create(String InNodeName, Mat4 InTransformationMatrix, String InPayloadName, NodePayloadType InPayloadType)
	{
		return std::shared_ptr<Node>(new Node(std::move(InNodeName), InTransformationMatrix, std::move(InPayloadName), InPayloadType));
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
	
	const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const
	{
		return Children;
	}

	void Node::SetParent(std::shared_ptr<Node> NewParent)
	{
		Parent = std::move(NewParent);
	}

	void Node::AddChild(std::shared_ptr<Node> NewChild)
	{
		NewChild->SetParent(shared_from_this());
		Children.push_back(std::move(NewChild));
	}

	bool Node::RemoveChild(StringView ChildNodeName)
	{
		if (auto MaybeChild = std::ranges::find_if(Children, [&](const auto& Element) { return Element->GetNodeName() == ChildNodeName; });
			MaybeChild != Children.end())
		{
			Children.erase(MaybeChild);
			return true;
		}
		return false;
	}
}
