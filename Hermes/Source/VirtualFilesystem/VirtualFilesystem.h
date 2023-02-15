#pragma once

#include <optional>
#include <vector>

#include "Core/Core.h"
#include "VirtualFilesystem/MountPoint.h"

namespace Hermes
{
	class FSDevice;
	class IPlatformFile;

	enum class FileAccessMode
	{
		Read,
		ReadWrite
	};

	enum class FileOpenMode
	{
		/*
		 * Always open an already existing file
		 */
		OpenExisting,

		/*
		 * Creates a new file if it does not yet exist or overwrites the existing file
		 */
		CreateNew
	};

	class HERMES_API VirtualFilesystem
	{
	public:
		/*
		 * Returns true if a said file exists
		 */
		static bool FileExists(StringView Path);

		static std::unique_ptr<IPlatformFile> Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode);

		static std::optional<String> ReadFileAsString(StringView Path);

		static std::optional<std::vector<uint8>> ReadFileAsBytes(StringView Path);

		static bool RemoveFile(StringView Path);

		/*
		 * Mounts a new device at a given path. Priority is used to allow multiple devices with overlapping files
		 * to be mounted to the same base path. Priority 0 is the lowest, meaning that a file from device with
		 * priority 0 will be used only after all devices with higher priorities did not provide the required file.
		 */
		static void Mount(StringView Path, MountMode Mode, uint32 Priority, std::unique_ptr<FSDevice> Device);

		static void UnmountAll();

	private:
		static std::vector<std::pair<MountPoint, uint32>> MountPoints;
	};
}
