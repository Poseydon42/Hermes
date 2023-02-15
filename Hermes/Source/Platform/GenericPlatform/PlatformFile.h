#pragma once

#include <memory>
#include <optional>

#include "Core/Core.h"
#include "Core/Misc/EnumClassOperators.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	/**
	 * Platform-independent file handle
	 */
	class HERMES_API IPlatformFile
	{
		MAKE_NON_COPYABLE(IPlatformFile)
		
	public:
		enum class FileAccessMode
		{
			Read = 0x01,
			Write = 0x02,
		};

		enum class FileOpenMode
		{
			Create, // Opens existing file and puts pointer at its end or creates new file is it does not exist
			CreateAlways, // Always creates new files and fails if file already exists
			OpenExisting, // Always open existing file and fails if file is not found
			OpenExistingOverwrite // Always opens existing file, but clears all its data and puts pointer to beginning of the file
		};
		
		virtual ~IPlatformFile() = default;
		IPlatformFile() = default;
		IPlatformFile(IPlatformFile&& Other) = default;
		IPlatformFile& operator=(IPlatformFile&& Other) = default;
		
		/**
		 * Returns size of file in bytes
		 */
		virtual size_t Size() const = 0;

		/**
		 * Sets read/write pointer to a new position relative from start of the file
		 */
		virtual void Seek(size_t NewPosition) = 0;

		/**
		 * Returns read/write pointer position relative from start of the file
		 */
		virtual size_t Tell() const = 0;

		/**
		 * Writes data to a file
		 * @return true if operation succeeded
		 */
		virtual bool Write(const void* Data, size_t Size) = 0;

		/**
		 * Reads data from a file
		 * @param Buffer Pre-allocated buffer where data should be stored
		 * @param Size Amount of bytes to read. If Tell() + Size > Size() then unavailable bytes will be untouched
		 */
		virtual bool Read(void* Buffer, size_t Size) = 0;

		/**
		 * Flushes all buffered writes and forces write to an underlying file storage
		 */
		virtual void Flush() = 0;

		/**
		 * Closes file and makes its handle invalid
		 */
		virtual void Close() = 0;
	};

	ENUM_CLASS_OPERATORS(IPlatformFile::FileAccessMode)

	/**
	 * Set of platform-independent functions for managing filesystem
	 */
	class HERMES_API PlatformFilesystem
	{
	public:
		/**
		 * @return True if file or directory at given path exists
		 */
		static bool FileExists(StringView Path);

		/**
		 * Opens a new file
		 */
		static std::unique_ptr<IPlatformFile> OpenFile(StringView Path, IPlatformFile::FileAccessMode Access, IPlatformFile::FileOpenMode OpenMode);

		/**
		 * Reads the whole file as text file and returns its contents if the file can be opened.
		 */
		static std::optional<String> ReadFileAsString(StringView Path);

		/**
		 * Deletes a file
		 * @return True if file was successfully deleted
		 */
		static bool RemoveFile(StringView Path);
	};
}
