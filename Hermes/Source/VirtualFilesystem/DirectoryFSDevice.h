#pragma once

#include "Core/Core.h"
#include "VirtualFilesystem/FSDevice.h"

namespace Hermes
{
	class HERMES_API DirectoryFSDevice : public FSDevice
	{
	public:
		DirectoryFSDevice(StringView InDirectoryPath);

		virtual ~DirectoryFSDevice() override = default;

		virtual std::unique_ptr<IPlatformFile> Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode) const override;

		virtual bool FileExists(StringView Path) const override;

		virtual bool IsMutable() const override;

		virtual bool RemoveFile(StringView Path) const override;

	private:
		String DirectoryPath;

		String GetAbsolutePath(StringView RelativePath) const;
	};
}
