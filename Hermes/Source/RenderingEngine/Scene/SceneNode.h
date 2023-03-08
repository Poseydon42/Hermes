#pragma once

#include <concepts>
#include <span>
#include <vector>

#include "Core/Core.h"
#include "Math/BoundingVolume.h"
#include "Math/Transform.h"
#include "RenderingEngine/Material/MaterialInstance.h"
#include "RenderingEngine/Resource/MeshResource.h"

namespace Hermes
{
	enum class SceneNodeType
	{
		None,

		Mesh,

		PointLight,
		DirectionalLight,
	};

	class HERMES_API SceneNode
	{
	public:
		virtual ~SceneNode() = default;

		SceneNode(const SceneNode&) = delete;
		SceneNode& operator=(const SceneNode&) = delete;

		SceneNode() = default;

		explicit SceneNode(SceneNodeType InType, Transform InTransform = {});

		SceneNodeType GetType() const;

		size_t GetChildrenCount() const;
		const SceneNode& GetChild(size_t Index) const;
		SceneNode& GetChild(size_t Index);

		const SceneNode* GetParent() const;
		SceneNode* GetParent();

		const Transform& GetLocalTransform() const;
		Transform& GetLocalTransform();
		void SetLocalTransform(Transform NewTransform);

		Mat4 GetLocalTransformationMatrix() const;

		Mat4 GetWorldTransformationMatrix() const;

		template<typename ChildType>
		requires (std::derived_from<ChildType, SceneNode> && std::is_move_constructible_v<ChildType>)
		SceneNode& AddChild(ChildType NewChild);

		template<typename ChildType, typename... ArgsType>
		requires (std::derived_from<ChildType, SceneNode> && std::constructible_from<ChildType, ArgsType...>)
		SceneNode& AddChild(ArgsType... Args);

		void RemoveChild(size_t ChildIndex);

		void SetParent(SceneNode* NewParent);

	private:
		SceneNodeType Type = SceneNodeType::None;

		std::vector<std::unique_ptr<SceneNode>> Children;
		// FIXME: this pointer will probably be invalidated if another node is added to parent and the vector has to reallocate memory
		SceneNode* Parent = nullptr;

		Transform LocalTransform;

		SceneNode& AddChildImpl(std::unique_ptr<SceneNode> NewNode);
	};

	template<typename ChildType>
	requires (std::derived_from<ChildType, SceneNode> && std::is_move_constructible_v<ChildType>)
	SceneNode& SceneNode::AddChild(ChildType NewChild)
	{
		return AddChildImpl(std::make_unique<ChildType>(std::move(NewChild)));
	}

	template<typename ChildType, typename ... ArgsType>
	requires (std::derived_from<ChildType, SceneNode> && std::constructible_from<ChildType, ArgsType...>)
	SceneNode& SceneNode::AddChild(ArgsType... Args)
	{
		return AddChildImpl(std::make_unique<ChildType>(std::forward<ArgsType>(Args)...));
	}

	class HERMES_API MeshNode : public SceneNode
	{
	public:
		virtual ~MeshNode() override = default;

		MeshNode(Transform Transform, String InMeshName, std::shared_ptr<MaterialInstance> InMaterial);

		const SphereBoundingVolume& GetBoundingVolume() const;

		const MeshResource& GetMesh() const;
		void SetMeshBuffer(String NewMeshName);

		const MaterialInstance& GetMaterialInstance() const;
		void SetMaterialInstance(std::shared_ptr<MaterialInstance> NewMaterialInstance);

	private:
		String MeshName; // FIXME: replace with asset handle once it is implemented
		std::shared_ptr<MaterialInstance> Material;
	};

	class HERMES_API PointLightNode : public SceneNode
	{
	public:
		virtual ~PointLightNode() override = default;

		PointLightNode(Transform Transform, Vec3 InColor, float InIntensity);

		Vec3 GetColor() const;
		void SetColor(Vec3 NewColor);

		float GetIntensity() const;
		void SetIntensity(float NewIntensity);

	private:
		Vec3 Color;
		float Intensity;
	};

	class HERMES_API DirectionalLightNode : public SceneNode
	{
	public:
		virtual ~DirectionalLightNode() override = default;

		DirectionalLightNode(Transform Transform, Vec3 InDirection, Vec3 InColor, float InIntensity);

		Vec3 GetDirection() const;
		void SetDirection(Vec3 NewDirection);

		Vec3 GetColor() const;
		void SetColor(Vec3 NewColor);

		float GetIntensity() const;
		void SetIntensity(float NewIntensity);

	private:
		Vec3 Direction;
		Vec3 Color;
		float Intensity;
	};
}
