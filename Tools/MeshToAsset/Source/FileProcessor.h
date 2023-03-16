#pragma once

#include <memory>

#include "AssetSystem/MeshAsset.h"
#include "InputFileReader.h"

namespace Hermes::Tools
{
	class FileProcessor
	{
	public:
		FileProcessor(StringView InFileName, bool InFlipVertexOrder);

		bool Run() const;

	private:
		std::unique_ptr<IInputFileReader> InputFileReader;

		String InputFileName;

		bool ShouldFlipVertexOrder = false;

		static Vertex ApplyVertexTransformation(Vertex Input, Mat4 TransformationMatrix);
		static Mesh ApplyVertexTransformation(const Mesh& Input, Mat4 TransformationMatrix);
	};
}
