#include "GLTFReader.h"

#include <format>
#include <iostream>

#include "JSON/JSONParser.h"
#include "Math/Quaternion.h"
#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes::Tools
{
	template<typename ComponentType>
	class AccessorStream
	{
	public:
		explicit AccessorStream(ComponentType InDefaultValue)
			: DefaultValue(InDefaultValue)
		{
		}

		// NOTE: ComponentsPerElement represent the width of the data type stored in this accessor
		//       2 for Vec2, 3 for Vec3 and 4 for Vec4
		AccessorStream(std::span<const uint8> InData, size_t InComponentsPerElement, size_t InElementCount, size_t InStride, ComponentType InDefaultValue = static_cast<ComponentType>(0))
			: Data(InData)
			, ComponentsPerElement(InComponentsPerElement)
			, ElementCount(InElementCount)
			, Stride(InStride)
			, DefaultValue(InDefaultValue)
		{
			if (Stride == 0)
				Stride = sizeof(ComponentType) * ComponentsPerElement;
		}

		/*
		 * Returns the next element or default value if reading beyond the end of the buffer
		 */
		ComponentType Next()
		{
			if (CurrentByteOffset >= Math::Min(ElementCount * Stride, Data.size()))
				return DefaultValue;

			auto Result = *reinterpret_cast<const ComponentType*>(&Data[CurrentByteOffset]);

			CurrentByteOffset += sizeof(ComponentType);
			CurrentComponentOffset++;

			if (CurrentComponentOffset == ComponentsPerElement)
			{
				CurrentComponentOffset = 0;
				CurrentByteOffset += Stride - sizeof(ComponentType) * ComponentsPerElement;
			}

			return Result;
		}

		template<typename T>
		T NextAs()
		{
			auto Value = Next();

			return static_cast<T>(Value);
		}

	private:
		std::span<const uint8> Data;
		size_t ComponentsPerElement = 0;
		size_t ElementCount = 0;
		size_t Stride = 0;

		size_t CurrentByteOffset = 0;
		size_t CurrentComponentOffset = 0;

		ComponentType DefaultValue = static_cast<ComponentType>(0);
	};

	// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.pdf
	bool GLTFReader::Read(StringView Path)
	{
		auto MaybeFileContents = PlatformFilesystem::ReadFileAsString(Path);
		if (!MaybeFileContents)
		{
			std::cerr << "Could not read input file " << Path << std::endl;
			return false;
		}

		auto FileContents = std::move(MaybeFileContents.value());

		auto MaybeJSONRoot = JSONParser::FromString(FileContents);
		if (!MaybeJSONRoot.has_value())
		{
			std::cerr << "Could not parse file " << Path << std::endl;
			return false;
		}

		auto JSONRoot = std::move(MaybeJSONRoot.value());
		
		if (!JSONRoot->Contains("asset") || !JSONRoot->Get("asset").Is(JSONValueType::Object))
			return false;

		if (!CheckAssetVersion(JSONRoot->Get("asset").AsObject()))
			return false;

		if (!JSONRoot->Contains("buffers") || !JSONRoot->Get("buffers").Is(JSONValueType::Array))
			return false;

		if (!ReadBuffers(JSONRoot->Get("buffers").AsArray(), Path))
			return false;

		if (!JSONRoot->Contains("bufferViews") || !JSONRoot->Get("bufferViews").Is(JSONValueType::Array))
			return false;

		if (!ReadBufferViews(JSONRoot->Get("bufferViews").AsArray()))
			return false;

		if (!JSONRoot->Contains("accessors") || !JSONRoot->Get("accessors").Is(JSONValueType::Array))
			return false;

		if (!ReadAccessors(JSONRoot->Get("accessors").AsArray()))
			return false;

		if (!JSONRoot->Contains("meshes") || !JSONRoot->Get("meshes").Is(JSONValueType::Array))
			return false;

		if (!ReadMeshes(JSONRoot->Get("meshes").AsArray()))
			return false;

		if (!ReadSceneHierarchy(*JSONRoot))
			return false;

		return true;
	}

	const Node& GLTFReader::GetRootNode() const
	{
		return *Root;
	}

	std::optional<const Mesh*> GLTFReader::GetMesh(StringView MeshName) const
	{
		for (const auto& Mesh : Meshes)
		{
			if (Mesh.GetName() == MeshName)
			{
				return &Mesh;
			}
		}
		return {};
	}

	bool GLTFReader::CheckAssetVersion(const JSONObject& AssetObject) const
	{
		if (AssetObject.Contains("minVersion") && AssetObject["minVersion"].Is(JSONValueType::String))
		{
			auto MaybeMinVersion = Version::FromString(AssetObject["minVersion"].AsString());
			if (MaybeMinVersion.has_value())
			{
				auto MinVersion = MaybeMinVersion.value();
				return (MinVersion <= LoaderVersion);
			}
		}

		// NOTE: "asset" object MUST contain a string property called "version"
		// that specifies version of the glTF spec that this assets is compliant with
		if (!AssetObject.Contains("version") ||
			!AssetObject["version"].Is(JSONValueType::String))
		{
			return false;
		}

		auto MaybeVersion = Version::FromString(AssetObject["version"].AsString());
		if (!MaybeVersion.has_value())
		{
			return false;
		}

		auto Version = MaybeVersion.value();
		// NOTE: we only need to check the major version here according to the spec
		return Version.Major == LoaderVersion.Major;
	}

	bool GLTFReader::ReadBuffers(std::span<const JSONValue> JSONBuffers, StringView InputFileName)
	{
		for (const auto& Buffer : JSONBuffers)
		{
			if (!Buffer.Is(JSONValueType::Object))
				return false;

			if (!ReadBuffer(Buffer.AsObject(), InputFileName))
				return false;
		}

		return true;
	}

	bool GLTFReader::ReadBuffer(const JSONObject& JSONBuffer, StringView InputFileName)
	{
		if (!JSONBuffer.Contains("byteLength") || !JSONBuffer.Get("byteLength").Is(JSONValueType::Number))
			return false;

		size_t CurrentBufferIndex = Buffers.size();

		auto MaybeLength = JSONBuffer.Get("byteLength").AsInteger();
		if (MaybeLength <= 0)
			return false;
		auto Length = static_cast<size_t>(MaybeLength);

		if (!JSONBuffer.Contains("uri") || !JSONBuffer.Get("uri").Is(JSONValueType::String))
		{
			return false;
		}

		auto RelativeFileName = JSONBuffer.Get("uri").AsString();
		if (RelativeFileName.starts_with("data:"))
		{
			std::cerr << std::format("Buffer {} has data stored in inline base64 form, which is not supported", CurrentBufferIndex) << std::endl;
			return false;
		}

		auto IndexOfLastForwardSlash = InputFileName.find_last_of('/');
		if (IndexOfLastForwardSlash == String::npos)
			IndexOfLastForwardSlash = 0;

		auto IndexOfLastBackwardSlash = InputFileName.find_last_of('\\');
		if (IndexOfLastBackwardSlash == String::npos)
			IndexOfLastBackwardSlash = 0;

		auto IndexOfLastSlash = Math::Max(IndexOfLastForwardSlash, IndexOfLastBackwardSlash);
		if (IndexOfLastSlash == String::npos)
		{
			IndexOfLastSlash = 0;
		}
		auto InputFilePathWithoutFileName = InputFileName.substr(0, IndexOfLastSlash);

		auto BufferFileName = String(InputFilePathWithoutFileName) + '/' + String(RelativeFileName);

		auto BufferFile = PlatformFilesystem::OpenFile(BufferFileName, IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
		if (!BufferFile)
		{
			std::cerr << std::format("Buffer {} in GLTF file {} references binary file {} that cannot be found", CurrentBufferIndex, InputFileName, BufferFileName);
			return false;
		}

		Buffers.emplace_back(Length);
		if (!BufferFile->Read(Buffers.back().data(), Length))
		{
			return false;
		}

		return true;
	}

	bool GLTFReader::ReadBufferViews(std::span<const JSONValue> JSONBufferViews)
	{
		for (const auto& BufferView : JSONBufferViews)
		{
			if (!BufferView.Is(JSONValueType::Object))
				return false;

			if (!ReadBufferView(BufferView.AsObject()))
				return false;
		}

		return true;
	}

	bool GLTFReader::ReadBufferView(const JSONObject& JSONBufferView)
	{
		BufferView View = {};

		if (!JSONBufferView.Contains("buffer") || !JSONBufferView.Get("buffer").Is(JSONValueType::Number))
			return false;
		View.Index = static_cast<size_t>(JSONBufferView.Get("buffer").AsInteger());
		if (View.Index >= Buffers.size())
			return false;

		if (JSONBufferView.Contains("byteOffset") && JSONBufferView.Get("byteOffset").Is(JSONValueType::Number))
		{
			View.Offset = static_cast<size_t>(JSONBufferView.Get("byteOffset").AsInteger());
		}
		else
		{
			View.Offset = 0;
		}
		if (View.Offset >= Buffers[View.Index].size())
			return false;

		if (!JSONBufferView.Contains("byteLength") || !JSONBufferView.Get("byteLength").Is(JSONValueType::Number))
			return false;
		View.Length = static_cast<size_t>(JSONBufferView.Get("byteLength").AsInteger());
		if (View.Offset + View.Length > Buffers[View.Index].size())
			return false;

		if (JSONBufferView.Contains("byteStride") && JSONBufferView.Get("byteStride").Is(JSONValueType::Number))
		{
			View.Stride = static_cast<size_t>(JSONBufferView.Get("byteStride").AsInteger());
		}
		else
		{
			View.Stride = 0;
		}

		BufferViews.push_back(View);

		return true;
	}

	bool GLTFReader::ReadAccessors(std::span<const JSONValue> JSONAccessors)
	{
		for (const auto& Accessor : JSONAccessors)
		{
			if (!Accessor.Is(JSONValueType::Object))
				return false;

			if (!ReadAccessor(Accessor.AsObject()))
				return false;
		}

		return true;
	}

	bool GLTFReader::ReadAccessor(const JSONObject& JSONAccessor)
	{
		Accessor Accessor = {};

		if (JSONAccessor.Contains("bufferView") && JSONAccessor.Get("bufferView").Is(JSONValueType::Number))
			Accessor.BufferViewIndex = static_cast<size_t>(JSONAccessor["bufferView"].AsInteger());
		if (Accessor.BufferViewIndex != static_cast<size_t>(-1) && Accessor.BufferViewIndex >= BufferViews.size())
			return false;

		if (JSONAccessor.Contains("byteOffset") && JSONAccessor.Get("byteOffset").Is(JSONValueType::Number))
			Accessor.Offset = static_cast<size_t>(JSONAccessor["byteOffset"].AsInteger());
		if (Accessor.BufferViewIndex != static_cast<size_t>(-1) && Accessor.Offset >= BufferViews[Accessor.BufferViewIndex].Length)
			return false;
		
		if (!JSONAccessor.Contains("componentType") || !JSONAccessor["componentType"].Is(JSONValueType::Number))
			return false;
		int64 RawComponentType = JSONAccessor["componentType"].AsInteger();
		// NOTE: allowed values are 5120, 5121, 5122, 5123, 5125 and 5126
		if (RawComponentType < 5120 || RawComponentType > 5126 || RawComponentType == 5124)
			return false;
		Accessor.ComponentType = static_cast<AccessorComponentType>(RawComponentType);

		if (JSONAccessor.Contains("normalized") && JSONAccessor["normalized"].Is(JSONValueType::Bool))
			Accessor.Normalized = JSONAccessor["normalized"].AsBool();

		if (!JSONAccessor.Contains("count") || !JSONAccessor["count"].Is(JSONValueType::Number))
			return false;
		Accessor.Count = JSONAccessor["count"].AsInteger();

		if (!JSONAccessor.Contains("type") || !JSONAccessor["type"].Is(JSONValueType::String))
			return false;
		Accessor.Type = AccessorTypeFromString(JSONAccessor["type"].AsString());

		Accessors.push_back(Accessor);
		return true;
	}

	bool GLTFReader::ReadMeshes(std::span<const JSONValue> JSONMeshes)
	{
		for (const auto& Mesh : JSONMeshes)
		{
			if (!Mesh.Is(JSONValueType::Object))
				return false;

			if (!ReadMesh(Mesh.AsObject()))
				return false;
		}

		return true;
	}

	bool GLTFReader::ReadMesh(const JSONObject& JSONMesh)
	{
		static size_t MeshIndex = 0;

		// NOTE: let's hope that such name will not exist in the GLTF file
		String MeshName = std::format("$UNNAMED_MESH${}$", MeshIndex);
		MeshIndex++;
		if (JSONMesh.Contains("name") && JSONMesh["name"].Is(JSONValueType::String))
		{
			MeshName = JSONMesh["name"].AsString();
		}
		
		if (!JSONMesh.Contains("primitives") || !JSONMesh["primitives"].Is(JSONValueType::Array) || JSONMesh["primitives"].AsArray().empty())
			return false;

		std::vector<Vertex> MergedVertices;
		std::vector<uint32> MergedIndices;
		std::vector<MeshPrimitiveHeader> Primitives;
		bool TangentsComputed = true;
		for (const auto& JSONPrimitive : JSONMesh["primitives"].AsArray())
		{
			if (!JSONPrimitive.Is(JSONValueType::Object))
				return false;

			auto MaybePrimitive = ReadPrimitive(JSONPrimitive.AsObject());
			if (!MaybePrimitive.has_value())
			{
				return false;
			}

			auto RawPrimitive = std::move(MaybePrimitive.value());

			size_t IndexOffset = MergedVertices.size();
			size_t FirstIndex = MergedIndices.size();
			MergedVertices.insert(MergedVertices.end(), RawPrimitive.Vertices.begin(), RawPrimitive.Vertices.end());
			MergedIndices.insert(MergedIndices.end(), RawPrimitive.Indices.begin(), RawPrimitive.Indices.end());

			std::for_each(MergedIndices.begin() + static_cast<ptrdiff_t>(FirstIndex), MergedIndices.end(), [&](auto& Element)
			{
				if (Element != static_cast<uint32>(-1))
					Element += static_cast<uint32>(IndexOffset);
			});

			Primitives.emplace_back(static_cast<uint32>(FirstIndex), static_cast<uint32>(RawPrimitive.Indices.size()));
			TangentsComputed &= RawPrimitive.HasTangents;
		}

		Meshes.emplace_back(MeshName, MergedVertices, MergedIndices, Primitives, TangentsComputed);

		return true;
	}

	std::optional<GLTFReader::GLTFMeshPrimitive> GLTFReader::ReadPrimitive(const JSONObject& GLTFPrimitive) const
	{
		GLTFMeshPrimitive Primitive = {};

		if (GLTFPrimitive.Contains("mode") && GLTFPrimitive["mode"].Is(JSONValueType::Number) && GLTFPrimitive["mode"].AsInteger() != 4)
		{
			std::cerr << "Geometry type other than triangle list is not supported" << std::endl;
			return {};
		}

		if (!GLTFPrimitive.Contains("attributes") || !GLTFPrimitive["attributes"].Is(JSONValueType::Object))
			return {};

		int32 PositionAccessorIndex = -1, NormalAccessorIndex = -1, TangentAccessorIndex = -1, TexCoordAccessorIndex = -1;
		for (const auto& Attribute : GLTFPrimitive["attributes"].AsObject())
		{
			if (!Attribute.second.Is(JSONValueType::Number))
				return {};

			auto Type = AttributeTypeFromString(Attribute.first);
			auto Accessor = static_cast<int32>(Attribute.second.AsInteger());
			switch (Type)
			{
			case AttributeType::Position:
				PositionAccessorIndex = Accessor;
				break;
			case AttributeType::Normal:
				NormalAccessorIndex = Accessor;
				break;
			case AttributeType::Tangent:
				TangentAccessorIndex = Accessor;
				break;
			case AttributeType::TexCoord0:
				TexCoordAccessorIndex = Accessor;
				break;
			default:
				break;
			}
		}

		if (PositionAccessorIndex == -1 || NormalAccessorIndex == -1)
			return {};
		if (TangentAccessorIndex != -1)
			Primitive.HasTangents = true;

		AccessorStream<float> PositionAccessor(GetDataForAccessor(PositionAccessorIndex), 3, Accessors[PositionAccessorIndex].Count, GetStrideForAccessor(PositionAccessorIndex));
		AccessorStream<float> NormalAccessor(GetDataForAccessor(NormalAccessorIndex), 3, Accessors[NormalAccessorIndex].Count, GetStrideForAccessor(NormalAccessorIndex));
		AccessorStream<float> TangentAccessor(0.0f);
		if (TangentAccessorIndex >= 0)
			TangentAccessor = AccessorStream<float>(GetDataForAccessor(TangentAccessorIndex), 3, Accessors[TangentAccessorIndex].Count, GetStrideForAccessor(TangentAccessorIndex));
		AccessorStream<float> TexCoordAccessor(0.0f);
		if (TexCoordAccessorIndex >= 0)
			TexCoordAccessor = AccessorStream<float>(GetDataForAccessor(TexCoordAccessorIndex), 2, Accessors[TexCoordAccessorIndex].Count, GetStrideForAccessor(TexCoordAccessorIndex));

		// NOTE: this should *technically* work in 99.9% of cases unless there's some weird memory saving stuff going on
		size_t VertexCount = Accessors[PositionAccessorIndex].Count;
		
		for (size_t VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			Vertex NewVertex = {};
			NewVertex.Position = { PositionAccessor.Next(), PositionAccessor.Next(), PositionAccessor.Next() };
			NewVertex.Normal = { NormalAccessor.Next(), NormalAccessor.Next(), NormalAccessor.Next() };
			NewVertex.Tangent = { TangentAccessor.Next(), TangentAccessor.Next(), TangentAccessor.Next() };
			NewVertex.TextureCoordinates = { TexCoordAccessor.Next(), TexCoordAccessor.Next() };

			Primitive.Vertices.push_back(NewVertex);
		}


		
		if (!GLTFPrimitive.Contains("indices") || !GLTFPrimitive["indices"].Is(JSONValueType::Number))
		{
			for (uint32 Index = 0; Index < static_cast<uint32>(VertexCount); Index++)
			{
				Primitive.Indices.push_back(Index);
				if (Index % 3 == 2)
					Primitive.Indices.push_back(static_cast<uint32>(-1));
			}
		}
		else
		{
			auto IndexAccessorIndex = GLTFPrimitive["indices"].AsInteger();
			if (static_cast<size_t>(IndexAccessorIndex) >= Accessors.size())
				return {};

			if (Accessors[IndexAccessorIndex].Type != AccessorType::Scalar)
				return {};

			if (Accessors[IndexAccessorIndex].ComponentType != AccessorComponentType::UnsignedShort &&
				Accessors[IndexAccessorIndex].ComponentType != AccessorComponentType::UnsignedInt)
				return {};

			switch (Accessors[IndexAccessorIndex].ComponentType)
			{
			case AccessorComponentType::UnsignedShort:
			{
				AccessorStream<uint16> Accessor(GetDataForAccessor(IndexAccessorIndex), 1, Accessors[IndexAccessorIndex].Count, sizeof(uint16));
				for (size_t Index = 0; Index < Accessors[IndexAccessorIndex].Count; Index++)
				{
					Primitive.Indices.push_back(Accessor.NextAs<uint32>());
					if (Index % 3 == 2)
						Primitive.Indices.push_back(static_cast<uint32>(-1));
				}
				break;
			}
			case AccessorComponentType::UnsignedInt:
			{
				AccessorStream<uint32> Accessor(GetDataForAccessor(IndexAccessorIndex), 1, Accessors[IndexAccessorIndex].Count, sizeof(uint32));
				for (size_t Index = 0; Index < Accessors[IndexAccessorIndex].Count; Index++)
				{
					Primitive.Indices.push_back(Accessor.NextAs<uint32>());
					if (Index % 3 == 2)
						Primitive.Indices.push_back(static_cast<uint32>(-1));
				}
				break;
			}
			default:
				HERMES_ASSERT(false)
			}
		}

		return Primitive;
	}

	bool GLTFReader::ReadSceneHierarchy(const JSONObject& RootObject)
	{
		if (!RootObject.Contains("scenes") ||
			!RootObject["scenes"].Is(JSONValueType::Array) ||
			 RootObject["scenes"].AsArray().empty() ||
			!RootObject["scenes"].AsArray()[0].Is(JSONValueType::Object))
		{
			std::cerr << "GLTF asset does not define any scenes and cannot be loaded" << std::endl;
			return false;
		}

		if (RootObject["scenes"].AsArray().size() > 1)
		{
			std::cerr << "GLTF asset defines more than one scene; only the first scene will be used" << std::endl;
		}

		const auto& JSONScene = RootObject["scenes"].AsArray()[0].AsObject();
		if (!JSONScene.Contains("nodes") || !JSONScene["nodes"].Is(JSONValueType::Array))
			return false;
		if (!RootObject.Contains("nodes") || !RootObject["nodes"].Is(JSONValueType::Array))
			return false;

		auto JSONNodes = RootObject["nodes"].AsArray();
		for (const auto& SceneRootNode : JSONScene["nodes"].AsArray())
		{
			if (!SceneRootNode.Is(JSONValueType::Number))
				return false;

			auto NodeIndex = static_cast<size_t>(SceneRootNode.AsInteger());
			if (NodeIndex >= JSONNodes.size())
				return false;

			auto MaybeNode = ReadSceneNode(JSONNodes, NodeIndex);
			if (!MaybeNode.has_value())
				return false;

			MaybeNode.value()->SetParent(Root);
			Root->AddChild(std::move(MaybeNode.value()));
		}

		return true;
	}

	std::optional<std::shared_ptr<Node>> GLTFReader::ReadSceneNode(std::span<const JSONValue> JSONNodeList, size_t NodeIndex) const
	{
		HERMES_ASSERT(NodeIndex < JSONNodeList.size());
		HERMES_ASSERT(JSONNodeList[NodeIndex].Is(JSONValueType::Object));

		const auto& JSONNode = JSONNodeList[NodeIndex].AsObject();

		String NodeName = std::format("$NODE${}$", NodeIndex);
		if (JSONNode.Contains("name") && JSONNode["name"].Is(JSONValueType::String))
			NodeName = JSONNode["name"].AsString();

		String MeshName;
		if (JSONNode.Contains("mesh") && JSONNode["mesh"].Is(JSONValueType::Number))
		{
			auto MeshIndex = static_cast<size_t>(JSONNode["mesh"].AsInteger());
			if (MeshIndex >= Meshes.size())
				return {};

			MeshName = Meshes[MeshIndex].GetName();
		}
		
		Vec3 Translation = {}, Scale = { 1.0f };
		Quaternion Rotation = {};
		if (JSONNode.Contains("translation") && JSONNode["translation"].Is(JSONValueType::Array) && JSONNode["translation"].AsArray().size() == 3)
		{
			auto TranslationArray = JSONNode["translation"].AsArray();
			Translation.X = static_cast<float>(TranslationArray[0].AsNumber());
			Translation.Y = static_cast<float>(TranslationArray[1].AsNumber());
			Translation.Z = static_cast<float>(TranslationArray[2].AsNumber());
		}
		if (JSONNode.Contains("scale") && JSONNode["scale"].Is(JSONValueType::Array) && JSONNode["scale"].AsArray().size() == 3)
		{
			auto ScaleArray = JSONNode["scale"].AsArray();
			Scale.X = static_cast<float>(ScaleArray[0].AsNumber());
			Scale.Y = static_cast<float>(ScaleArray[1].AsNumber());
			Scale.Z = static_cast<float>(ScaleArray[2].AsNumber());
		}
		if (JSONNode.Contains("rotation") && JSONNode["rotation"].Is(JSONValueType::Array) && JSONNode["rotation"].AsArray().size() == 4)
		{
			auto RotationArray = JSONNode["rotation"].AsArray();
			Rotation.XYZ.X = static_cast<float>(RotationArray[0].AsNumber());
			Rotation.XYZ.Y = static_cast<float>(RotationArray[1].AsNumber());
			Rotation.XYZ.Z = static_cast<float>(RotationArray[2].AsNumber());
			Rotation.W     = static_cast<float>(RotationArray[3].AsNumber());
		}
		
		auto TranslationMatrix = Mat4::Translation(Translation);
		auto RotationMatrix = Mat4::Rotation(Rotation);
		auto ScaleMatrix = Mat4(Mat3::Scale(Scale));
		ScaleMatrix[3][3] = 1.0f;

		auto TransformationMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;

		auto Result = Node::Create(NodeName, TransformationMatrix, MeshName, MeshName.empty() ? NodePayloadType::None : NodePayloadType::Mesh);
		
		if (!JSONNode.Contains("children") || !JSONNode["children"].Is(JSONValueType::Array))
			return Result;

		for (const auto& JSONChildNode : JSONNode["children"].AsArray())
		{
			if (!JSONChildNode.Is(JSONValueType::Number))
				return {};

			auto ChildNodeIndex = static_cast<size_t>(JSONChildNode.AsInteger());
			if (ChildNodeIndex >= JSONNodeList.size())
				return {};

			auto MaybeChildNode = ReadSceneNode(JSONNodeList, ChildNodeIndex);
			if (!MaybeChildNode.has_value())
				return {};

			auto ChildNode = std::move(MaybeChildNode.value());
			ChildNode->SetParent(Result);
			Result->AddChild(std::move(ChildNode));
		}

		return Result;
	}

	std::span<const uint8> GLTFReader::GetDataForAccessor(size_t AccessorIndex) const
	{
		HERMES_ASSERT(AccessorIndex < Accessors.size());
		auto BufferViewIndex = Accessors[AccessorIndex].BufferViewIndex;

		HERMES_ASSERT(BufferViewIndex < BufferViews.size());
		auto BufferView = BufferViews[BufferViewIndex];

		auto BufferIndex = BufferView.Index;
		HERMES_ASSERT(BufferIndex < Buffers.size());

		return { Buffers[BufferIndex].begin() + static_cast<ptrdiff_t>(BufferView.Offset), BufferView.Length };
	}

	size_t GLTFReader::GetStrideForAccessor(size_t AccessorIndex) const
	{
		HERMES_ASSERT(AccessorIndex < Accessors.size());

		auto Accessor = Accessors[AccessorIndex];
		auto BufferViewIndex = Accessor.BufferViewIndex;

		HERMES_ASSERT(BufferViewIndex < BufferViews.size());

		return BufferViews[BufferViewIndex].Stride;
	}

	GLTFReader::AccessorType GLTFReader::AccessorTypeFromString(StringView String)
	{
		if (String == "SCALAR")
			return AccessorType::Scalar;
		if (String == "VEC2")
			return AccessorType::Vec2;
		if (String == "VEC3")
			return AccessorType::Vec3;
		if (String == "VEC4")
			return AccessorType::Vec4;
		if (String == "MAT2")
			return AccessorType::Mat2;
		if (String == "MAT3")
			return AccessorType::Mat3;
		if (String == "MAT4")
			return AccessorType::Mat4;

		HERMES_ASSERT(false)
	}

	GLTFReader::AttributeType GLTFReader::AttributeTypeFromString(StringView String)
	{
		if (String == "POSITION")
			return AttributeType::Position;
		if (String == "NORMAL")
			return AttributeType::Normal;
		if (String == "TANGENT")
			return AttributeType::Tangent;
		if (String == "TEXCOORD_0")
			return AttributeType::TexCoord0;
		return AttributeType::Unsupported;
	}
}
