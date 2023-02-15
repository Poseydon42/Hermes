#include "VirtualFilesystem.h"

#include "Platform/GenericPlatform/PlatformFile.h"
#include "VirtualFilesystem/FSDevice.h"

namespace Hermes
{
	std::vector<std::pair<MountPoint, uint32>> VirtualFilesystem::MountPoints;

	bool VirtualFilesystem::FileExists(StringView Path)
	{
		for (const auto& MountPoint : MountPoints)
			if (MountPoint.first.FileExists(Path))
				return true;

		return false;
	}

	std::unique_ptr<IPlatformFile> VirtualFilesystem::Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode)
	{
		for (auto& MountPoint : MountPoints)
		{
			auto MaybeFile = MountPoint.first.Open(Path, OpenMode, AccessMode);
			
			if (MaybeFile)
				return MaybeFile;
		}

		return nullptr;
	}
	
	std::optional<String> VirtualFilesystem::ReadFileAsString(StringView Path)
	{
		auto MaybeBytes = ReadFileAsBytes(Path);
		if (!MaybeBytes.has_value())
			return {};

		return String(MaybeBytes.value().begin(), MaybeBytes.value().end());
	}

	std::optional<std::vector<uint8>> VirtualFilesystem::ReadFileAsBytes(StringView Path)
	{
		auto File = Open(Path, FileOpenMode::OpenExisting, FileAccessMode::Read);
		if (!File)
			return {};

		std::vector<uint8> Bytes(File->Size());
		File->Read(Bytes.data(), Bytes.size());

		return Bytes;
	}

	bool VirtualFilesystem::RemoveFile(StringView Path)
	{
		for (auto& MountPoint : MountPoints)
		{
			if (!MountPoint.first.IsMutable())
				continue;

			if (!MountPoint.first.FileExists(Path))
				continue;

			return MountPoint.first.RemoveFile(Path);
		}

		return false;
	}

	void VirtualFilesystem::Mount(StringView Path, MountMode Mode, uint32 Priority, std::unique_ptr<FSDevice> Device)
	{
		size_t InsertIndex = 0;
		for (size_t Index = 0; Index < MountPoints.size(); Index++)
		{
			if (MountPoints[Index].second <= Priority)
				break;

			InsertIndex = Index;
		}

		MountPoints.insert(MountPoints.begin() + static_cast<ptrdiff_t>(InsertIndex), std::make_pair(MountPoint(String(Path), Mode, std::move(Device)), Priority));
	}
	
	void VirtualFilesystem::UnmountAll()
	{
		MountPoints.clear();
	}
}
