#pragma once

#include "Core/Core.h"
#include "Core/Misc/Version.h"
#include "InputFileReader.h"
#include "JSON/JSONObject.h"
#include "Mesh.h"
#include "Node.h"

namespace Hermes::Tools
{
	class HERMES_API GLTFReader : public IInputFileReader
	{
	public:
		virtual bool Read(StringView Path) override;

		virtual const Node& GetRootNode() const override;

		virtual std::optional<const Mesh*> GetMesh(StringView MeshName) const override;

	private:
		Node Root = Node("", "", NodePayloadType::None);

		std::vector<Mesh> Meshes;

		enum class AccessorComponentType : uint32
		{
			Byte = 5120,
			UnsignedByte = 5121,
			Short = 5122,
			UnsignedShort = 5123,
			UnsignedInt = 5125,
			Float = 5126
		};
		enum class AccessorType
		{
			Scalar,
			Vec2,
			Vec3,
			Vec4,
			Mat2,
			Mat3,
			Mat4
		};
		enum class AttributeType
		{
			Position,
			Normal,
			Tangent,
			TexCoord0,

			Unsupported
		};

		// FIXME: at the moment this design might cause duplication of vertices if they are used by two or more primitives
		struct GLTFMeshPrimitive
		{
			std::vector<Vertex> Vertices;
			bool HasTangents = false;
			std::vector<uint32> Indices;
		};
		struct Accessor
		{
			size_t BufferViewIndex = static_cast<size_t>(-1); // -1 means that buffer view index was not set, so accessor must return zeros for any read operation
			size_t Offset = 0;
			AccessorComponentType ComponentType;
			bool Normalized = false;
			size_t Count = 0;
			AccessorType Type;
			// FIXME: accessor sparse
		};
		struct BufferView
		{
			size_t Index;
			size_t Offset;
			size_t Length;
			size_t Stride;
		};
		std::vector<std::vector<uint8>> Buffers;
		std::vector<BufferView> BufferViews;
		std::vector<Accessor> Accessors;

		static constexpr Version LoaderVersion = Version(2, 0, 0);

		bool CheckAssetVersion(const JSONObject& AssetObject) const;

		bool ReadBuffers(std::span<const JSONValue> JSONBuffers, StringView InputFileName);
		bool ReadBuffer(const JSONObject& JSONBuffer, StringView InputFileName);

		bool ReadBufferViews(std::span<const JSONValue> JSONBufferViews);
		bool ReadBufferView(const JSONObject& JSONBufferView);

		bool ReadAccessors(std::span<const JSONValue> JSONAccessors);
		bool ReadAccessor(const JSONObject& JSONAccessor);

		bool ReadMeshes(std::span<const JSONValue> JSONMeshes);
		bool ReadMesh(const JSONObject& JSONMesh);

		std::optional<GLTFMeshPrimitive> ReadPrimitive(const JSONObject& GLTFPrimitive) const;

		bool ReadSceneHierarchy(const JSONObject& RootObject);
		std::optional<Node> ReadSceneNode(std::span<const JSONValue> JSONNodeList, size_t NodeIndex) const;

		std::span<const uint8> GetDataForAccessor(size_t AccessorIndex) const;
		size_t GetStrideForAccessor(size_t AccessorIndex) const;

		static AccessorType AccessorTypeFromString(StringView String);
		static AttributeType AttributeTypeFromString(StringView String);
	};
}
