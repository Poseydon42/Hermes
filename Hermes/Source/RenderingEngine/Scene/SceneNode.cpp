#include "SceneNode.h"

#include "ApplicationCore/GameLoop.h"
#include "AssetSystem/MeshAsset.h"

namespace Hermes
{
	SceneNode::SceneNode(SceneNodeType InType, Hermes::Transform InTransform)
		: Type(InType)
		, LocalTransform(InTransform)
	{
	}

	SceneNodeType SceneNode::GetType() const
	{
		return Type;
	}

	size_t SceneNode::GetChildrenCount() const
	{
		return Children.size();
	}

	const SceneNode& SceneNode::GetChild(size_t Index) const
	{
		return const_cast<SceneNode*>(this)->GetChild(Index);
	}

	SceneNode& SceneNode::GetChild(size_t Index)
	{
		HERMES_ASSERT(Index < Children.size());
		HERMES_ASSERT(Children[Index]);
		return *Children[Index];
	}

	const SceneNode* SceneNode::GetParent() const
	{
		return Parent;
	}

	SceneNode* SceneNode::GetParent()
	{
		return Parent;
	}

	const Transform& SceneNode::GetLocalTransform() const
	{
		return LocalTransform;
	}

	Transform& SceneNode::GetLocalTransform()
	{
		return LocalTransform;
	}

	void SceneNode::SetLocalTransform(Transform NewTransform)
	{
		LocalTransform = NewTransform;
	}

	Mat4 SceneNode::GetLocalTransformationMatrix() const
	{
		return LocalTransform.GetTransformationMatrix();
	}

	Mat4 SceneNode::GetWorldTransformationMatrix() const
	{
		if (!Parent)
			return GetLocalTransformationMatrix();

		return Parent->GetWorldTransformationMatrix() * GetLocalTransformationMatrix();
	}

	void SceneNode::RemoveChild(size_t ChildIndex)
	{
		if (ChildIndex >= Children.size())
			return;

		Children.erase(Children.begin() + static_cast<ptrdiff_t>(ChildIndex));
	}

	void SceneNode::SetParent(SceneNode* NewParent)
	{
		Parent = NewParent;
	}

	SceneNode& SceneNode::AddChildImpl(std::unique_ptr<SceneNode> NewNode)
	{
		NewNode->SetParent(this);
		Children.push_back(std::move(NewNode));
		return *Children.back();
	}

	MeshNode::MeshNode(Transform Transform, AssetHandle InMeshHandle, const MaterialInstance* InMaterial)
		: SceneNode(SceneNodeType::Mesh, Transform)
		, MeshHandle(InMeshHandle)
		, Material(std::move(InMaterial))
	{
	}

	const SphereBoundingVolume& MeshNode::GetBoundingVolume() const
	{
		auto& AssetCache = GGameLoop->GetAssetCache();

		auto Asset = AssetCache.Get<MeshAsset>(MeshHandle);
		HERMES_ASSERT(Asset);

		return Asset.value()->GetBoundingVolume();
	}

	AssetHandle MeshNode::GetMesh() const
	{
		return MeshHandle;
	}

	void MeshNode::SetMesh(AssetHandle NewMeshHandle)
	{
		MeshHandle = NewMeshHandle;
	}

	void MeshNode::SetMaterialInstance(const MaterialInstance* NewMaterialInstance)
	{
		Material = std::move(NewMaterialInstance);
	}

	const MaterialInstance& MeshNode::GetMaterialInstance() const
	{
		HERMES_ASSERT(Material);
		return *Material;
	}

	PointLightNode::PointLightNode(Transform Transform, Vec3 InColor, float InIntensity)
		: SceneNode(SceneNodeType::PointLight, Transform)
		, Color(InColor)
		, Intensity(InIntensity)
	{
	}

	Vec3 PointLightNode::GetColor() const
	{
		return Color;
	}

	void PointLightNode::SetColor(Vec3 NewColor)
	{
		Color = NewColor;
	}

	void PointLightNode::SetIntensity(float NewIntensity)
	{
		Intensity = NewIntensity;
	}

	float PointLightNode::GetIntensity() const
	{
		return Intensity;
	}

	DirectionalLightNode::DirectionalLightNode(Transform Transform, Vec3 InDirection, Vec3 InColor, float InIntensity)
		: SceneNode(SceneNodeType::DirectionalLight, Transform)
		, Direction(InDirection)
		, Color(InColor)
		, Intensity(InIntensity)
	{
	}

	Vec3 DirectionalLightNode::GetDirection() const
	{
		return Direction;
	}

	void DirectionalLightNode::SetDirection(Vec3 NewDirection)
	{
		Direction = NewDirection;
	}

	void DirectionalLightNode::SetColor(Vec3 NewColor)
	{
		Color = NewColor;
	}

	void DirectionalLightNode::SetIntensity(float NewIntensity)
	{
		Intensity = NewIntensity;
	}

	Vec3 DirectionalLightNode::GetColor() const
	{
		return Color;
	}

	float DirectionalLightNode::GetIntensity() const
	{
		return Intensity;
	}
}