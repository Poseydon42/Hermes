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
		
		String MeshName = InputFileName.substr(0, InputFileName.find_last_of('.')); // input file name without extension
		Mesh MergedMesh(std::move(MeshName), {}, {}, {}, true);
		std::function<bool(const Node&)> TraverseTree = [&](const Node& CurrentNode) -> bool
		{
			for (const auto& Child : CurrentNode.GetChildren())
			{
				if (!TraverseTree(Child))
					return false;
			}

			if (CurrentNode.GetPayloadType() != NodePayloadType::Mesh)
				return true;

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

			MergedMesh = MergeMeshes(MergedMesh.GetName(), MergedMesh, Mesh);
			return true;
		};

		if (!TraverseTree(InputFileReader->GetRootNode()))
			return false;

		String OutputFileName = std::format("{}.hac", MergedMesh.GetName());
		if (!MeshWriter::Write(OutputFileName, MergedMesh))
			return false;

		return true;
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

		return { String(Input.GetName()), Vertices, Input.GetIndices(), Input.GetPrimitives(), false };
	}

	Mesh FileProcessor::MergeMeshes(StringView OutputName, const Mesh& First, const Mesh& Second)
	{
		std::vector<Vertex> MergedVertices(First.GetVertices().size() + Second.GetVertices().size());

		std::ranges::copy(First.GetVertices(), MergedVertices.begin());
		std::ranges::copy(Second.GetVertices(), MergedVertices.begin() + static_cast<ptrdiff_t>(First.GetVertices().size()));

		std::vector<uint32> MergedIndices(First.GetIndices().size() + Second.GetIndices().size());
		std::ranges::copy(First.GetIndices(), MergedIndices.begin());
		std::ranges::copy(Second.GetIndices(), MergedIndices.begin() + static_cast<ptrdiff_t>(First.GetIndices().size()));

		auto VertexOffset = static_cast<uint32>(First.GetVertices().size());
		std::for_each(MergedIndices.begin() + static_cast<ptrdiff_t>(First.GetIndices().size()), MergedIndices.end(), [VertexOffset](auto& Element)
		{
			if (Element != static_cast<uint32>(-1))
				Element += VertexOffset;
		});

		std::vector<MeshPrimitiveHeader> MergedPrimitives(First.GetPrimitives().size() + Second.GetPrimitives().size());
		std::ranges::copy(First.GetPrimitives(), MergedPrimitives.begin());
		std::ranges::copy(Second.GetPrimitives(), MergedPrimitives.begin() + static_cast<ptrdiff_t>(First.GetPrimitives().size()));

		auto IndexOffset = static_cast<uint32>(First.GetIndices().size());
		std::for_each(MergedPrimitives.begin() + static_cast<ptrdiff_t>(First.GetPrimitives().size()), MergedPrimitives.end(), [IndexOffset](auto& Element)
		{
			Element.IndexBufferOffset += IndexOffset;
		});

		return { String(OutputName), std::move(MergedVertices), std::move(MergedIndices), std::move(MergedPrimitives), First.HasTangents() && Second.HasTangents() };
	}
}
