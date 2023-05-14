#include "MountPoint.h"

#include "Logging/Logger.h"
#include "Platform/GenericPlatform/PlatformFile.h"
#include "VirtualFilesystem/FSDevice.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

namespace Hermes
{
	MountPoint::MountPoint(String InPath, MountMode InMode, std::unique_ptr<FSDevice> InDevice)
		: BasePath(std::move(InPath))
		, Mode(InMode)
		, Device(std::move(InDevice))
	{
		HERMES_ASSERT_LOG(BasePath.starts_with('/'), "Mount point base path must be absolute");
		if (BasePath.ends_with('/'))
			BasePath = BasePath.substr(0, BasePath.length() - 1); // BasePath should not end with '/'
	}

	std::unique_ptr<IPlatformFile> MountPoint::Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode)
	{
		if (AccessMode == FileAccessMode::ReadWrite && Mode == MountMode::ReadOnly)
			return nullptr;
		auto MaybeRelativePath = GetRelativePath(Path);
		if (!MaybeRelativePath.has_value())
			return nullptr;
		auto RelativePath = MaybeRelativePath.value();

		return Device->Open(RelativePath, OpenMode, AccessMode);
	}

	bool MountPoint::FileExists(StringView Path) const
	{
		auto MaybeRelativePath = GetRelativePath(Path);
		if (!MaybeRelativePath.has_value())
			return false;

		return FileExistsImpl(MaybeRelativePath.value());
	}

	bool MountPoint::IsMutable() const
	{
		return (Mode == MountMode::ReadWrite) && Device->IsMutable();
	}

	bool MountPoint::RemoveFile(StringView Path)
	{
		auto MaybeRelativePath = GetRelativePath(Path);
		if (!MaybeRelativePath.has_value())
			return false;

		if (!FileExistsImpl(MaybeRelativePath.value()))
			return false;

		HERMES_ASSERT(Device->RemoveFile(MaybeRelativePath.value()));
		return true;
	}

	std::optional<StringView> MountPoint::GetRelativePath(StringView OriginalPath) const
	{
		if (!OriginalPath.starts_with('/'))
		{
			HERMES_LOG_WARNING("Path passed to VFS must be absolute (path: %s)", OriginalPath.data());
			return {};
		}

		// FIXME: deal with '.' and '..'
		if (OriginalPath.starts_with(BasePath))
			return OriginalPath.substr(BasePath.length(), OriginalPath.length() - BasePath.length());

		return {};
	}

	bool MountPoint::FileExistsImpl(StringView RelativePath) const
	{
		return Device->FileExists(RelativePath);
	}
}
