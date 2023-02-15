#pragma once

#include <optional>

#include "Core/Core.h"
#include "VirtualFilesystem/FSDevice.h"

namespace Hermes
{
	enum class FileAccessMode;
	enum class FileOpenMode;
	class IPlatformFile;

	enum class MountMode
	{
		ReadOnly,
		ReadWrite
	};

	class HERMES_API MountPoint
	{
	public:
		MountPoint(String InPath, MountMode InMode, std::unique_ptr<FSDevice> InDevice);

		std::unique_ptr<IPlatformFile> Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode);

		bool FileExists(StringView Path) const;

		bool IsMutable() const;

		bool RemoveFile(StringView Path);

	private:
		String BasePath;
		MountMode Mode;
		std::unique_ptr<FSDevice> Device;

		std::optional<StringView> GetRelativePath(StringView OriginalPath) const;

		bool FileExistsImpl(StringView RelativePath) const;
	};
}
