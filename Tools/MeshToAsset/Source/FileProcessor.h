#pragma once

#include <memory>

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
	};
}
