#include "DirectoryFSDevice.h"

#include "Platform/GenericPlatform/PlatformFile.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes
{
	DirectoryFSDevice::DirectoryFSDevice(StringView InDirectoryPath)
		: DirectoryPath(InDirectoryPath)
	{
	}

	std::unique_ptr<IPlatformFile> DirectoryFSDevice::Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode) const
	{
		IPlatformFile::FileOpenMode PlatformOpenMode = {};
		switch (OpenMode)
		{
		case FileOpenMode::OpenExisting:
			PlatformOpenMode = IPlatformFile::FileOpenMode::OpenExisting;
			break;
		case FileOpenMode::CreateNew:
			PlatformOpenMode = IPlatformFile::FileOpenMode::Create;
			break;
		default:
			HERMES_ASSERT(false);
		}

		IPlatformFile::FileAccessMode PlatformAccessMode = {};
		switch (AccessMode)
		{
		case FileAccessMode::Read:
			PlatformAccessMode = IPlatformFile::FileAccessMode::Read;
			break;
		case FileAccessMode::ReadWrite:
			PlatformAccessMode = IPlatformFile::FileAccessMode::Read | IPlatformFile::FileAccessMode::Write;
			break;
		default:
			HERMES_ASSERT(false);
		}

		return PlatformFilesystem::OpenFile(GetAbsolutePath(Path), PlatformAccessMode, PlatformOpenMode);
	}

	bool DirectoryFSDevice::FileExists(StringView Path) const
	{
		return PlatformFilesystem::FileExists(GetAbsolutePath(Path));
	}

	bool DirectoryFSDevice::IsMutable() const
	{
		return true;
	}

	bool DirectoryFSDevice::RemoveFile(StringView Path) const
	{
		return PlatformFilesystem::RemoveFile(GetAbsolutePath(Path));
	}

	String DirectoryFSDevice::GetAbsolutePath(StringView RelativePath) const
	{
		return DirectoryPath + String(RelativePath);
	}
}
