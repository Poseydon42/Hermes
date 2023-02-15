#pragma once

#include "Core/Core.h"

namespace Hermes
{
	enum class FileAccessMode;
	enum class FileOpenMode;
	class IPlatformFile;

	class HERMES_API FSDevice
	{
	public:
		virtual ~FSDevice() = default;

		virtual bool FileExists(StringView Path) const = 0;

		virtual bool IsMutable() const = 0;

		virtual std::unique_ptr<IPlatformFile> Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode) const = 0;

		virtual bool RemoveFile(StringView Path) const = 0;
	};
}
