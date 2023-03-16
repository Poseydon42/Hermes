#include "FileProcessor.h"

#include <format>
#include <functional>
#include <iostream>

#include "GLTFReader.h"
#include "Mesh.h"
#include "MeshWriter.h"
#include "Node.h"
#include "OBJReader.h"

namespace Hermes::Tools
{
	FileProcessor::FileProcessor(StringView InFileName, bool InFlipVertexOrder)
		: InputFileName(InFileName)
		, ShouldFlipVertexOrder(InFlipVertexOrder)
	{
		auto Extension = StringView(InFileName.begin() + static_cast<ptrdiff_t>(InFileName.find_last_of('.')) + 1, InFileName.end());
		if (Extension == "obj")
		{
			InputFileReader = std::make_unique<OBJReader>();
		}
		else if (Extension == "gltf")
		{
			InputFileReader = std::make_unique<GLTFReader>();
		}
	}

	bool FileProcessor::Run() const
	{
		if (!InputFileReader)
		{
			std::cerr << "Unknown file format" << std::endl;
			return false;
		}

		if (!InputFileReader->Read(InputFileName))
		{
			std::cerr << "Could not read or parse file " << InputFileName << std::endl;
			return false;
		}
		
		std::function<bool(const Node&, const String&)> TraverseTree = [&](const Node& CurrentNode, const String& RootName) -> bool
		{
			bool Result = true;

			auto CurrentName = std::format("{}_{}", RootName, CurrentNode.GetNodeName());
			for (const auto& Child : CurrentNode.GetChildren())
			{
				Result &= TraverseTree(Child, CurrentName);
			}

			if (CurrentNode.GetPayloadType() != NodePayloadType::Mesh)
				return Result;

			auto NameOfCurrentOutputFile = std::format("{}.hac", CurrentName);

			auto MaybeMesh = InputFileReader->GetMesh(CurrentNode.GetPayloadName());
			if (!MaybeMesh.has_value())
			{
				return false;
			}

			auto Mesh = *MaybeMesh.value();
			if (!Mesh.IsTriangulated())
			{
				std::cerr << "Converting non-triangulated meshes is not supported yet (mesh name: " << Mesh.GetName() << ")" << std::endl;
				return false;
			}

			auto TransformationMatrix = CurrentNode.ComputeGlobalTransformationMatrix();
			Mesh = ApplyVertexTransformation(Mesh, TransformationMatrix);

			if (!Mesh.HasTangents())
			{
				Mesh.ComputeTangents();
			}

			if (ShouldFlipVertexOrder)
			{
				Mesh.FlipVertexOrder();
			}

			Result &= MeshWriter::Write(NameOfCurrentOutputFile, Mesh);
			return Result;
		};
		
		return TraverseTree(InputFileReader->GetRootNode(), InputFileName);
	}

	Vertex FileProcessor::ApplyVertexTransformation(Vertex Input, Mat4 TransformationMatrix)
	{
		auto NormalMatrix = Mat3(TransformationMatrix.Inverse().Transpose());

		auto Position4 = Vec4(Input.Position, 1.0f);

		return {
			.Position = (TransformationMatrix * Position4).XYZ(),
			.TextureCoordinates = Input.TextureCoordinates,
			.Normal = NormalMatrix * Input.Normal,
			.Tangent = NormalMatrix * Input.Tangent
		};
	}

	Mesh FileProcessor::ApplyVertexTransformation(const Mesh& Input, Mat4 TransformationMatrix)
	{
		std::vector<Vertex> Vertices = Input.GetVertices();

		for (auto& Vertex : Vertices)
		{
			Vertex = ApplyVertexTransformation(Vertex, TransformationMatrix);
		}

		return Mesh(String(Input.GetName()), Vertices, Input.GetIndices(), Input.GetPrimitives(), false);
	}
}
