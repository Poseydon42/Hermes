#include "Node.h"

namespace Hermes::Tools
{
	Node::Node(String InNodeName, String InPayloadName, NodePayloadType InPayloadType)
		: NodeName(std::move(InNodeName))
		, PayloadName(std::move(InPayloadName))
		, PayloadType(InPayloadType)
	{
	}

	StringView Node::GetNodeName() const
	{
		return NodeName;
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
