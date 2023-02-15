#ifdef HERMES_PLATFORM_WINDOWS

#include "WindowsFile.h"

#include "Logging/Logger.h"

namespace Hermes
{
	static constexpr size_t GMaxFilePathLength = 4096;

	WindowsFile::WindowsFile(HANDLE InFile)
		: File(InFile)
	{
	}

	WindowsFile::~WindowsFile()
	{
		Close();
	}

	WindowsFile::WindowsFile(WindowsFile&& Other) noexcept
		: File(Other.File)
	{
		Other.File = INVALID_HANDLE_VALUE;
	}

	WindowsFile& WindowsFile::operator=(WindowsFile&& Other) noexcept
	{
		std::swap(File, Other.File);
		return *this;
	}

	size_t WindowsFile::Size() const
	{
		LARGE_INTEGER Win32Size;
		if (!GetFileSizeEx(File, &Win32Size)) return 0;
		return Win32Size.QuadPart;
	}

	void WindowsFile::Seek(size_t NewPosition)
	{
		if (NewPosition > Size())
			NewPosition = Size();

		LARGE_INTEGER NewPointer;
		NewPointer.QuadPart = static_cast<LONGLONG>(NewPosition);
		SetFilePointerEx(File, NewPointer, 0, FILE_BEGIN);
	}

	size_t WindowsFile::Tell() const
	{
		LARGE_INTEGER NewPointer;
		NewPointer.QuadPart = 0;
		LARGE_INTEGER CurrentPointer;
		SetFilePointerEx(File, NewPointer, &CurrentPointer, FILE_CURRENT);

		return CurrentPointer.QuadPart;
	}

	bool WindowsFile::Write(const void* Data, size_t Size)
	{
		DWORD Dummy;
		return WriteFile(File, Data, (DWORD)Size, &Dummy, 0);
	}

	bool WindowsFile::Read(void* Buffer, size_t Size)
	{
		DWORD Dummy;
		return ReadFile(File, Buffer, (DWORD)Size, &Dummy, 0);
	}

	void WindowsFile::Flush()
	{
		FlushFileBuffers(File);
	}

	void WindowsFile::Close()
	{
		CloseHandle(File);
		File = INVALID_HANDLE_VALUE;
	}

	static DWORD GetFileAttributesImpl(StringView Path)
	{
		wchar_t UTF16Path[GMaxFilePathLength];
		MultiByteToWideChar(CP_UTF8, 0, Path.data(), -1, UTF16Path, GMaxFilePathLength);

		auto Attributes = GetFileAttributesW(UTF16Path);
		return Attributes;
	}
	
	bool PlatformFilesystem::FileExists(StringView Path)
	{
		return GetFileAttributesImpl(Path) != INVALID_FILE_ATTRIBUTES;
	}

	std::unique_ptr<IPlatformFile> PlatformFilesystem::OpenFile(StringView Path, IPlatformFile::FileAccessMode Access, IPlatformFile::FileOpenMode OpenMode)
	{
		DWORD Win32Access = 0;
		if (static_cast<int>(Access & IPlatformFile::FileAccessMode::Read))
		{
			Win32Access |= GENERIC_READ;
		}
		if (static_cast<int>(Access & IPlatformFile::FileAccessMode::Write))
		{
			Win32Access |= GENERIC_WRITE;
		}

		DWORD Win32OpenMode = 0;
		switch (OpenMode)
		{
		case IPlatformFile::FileOpenMode::Create:
			Win32OpenMode = CREATE_ALWAYS;
			break;
		case IPlatformFile::FileOpenMode::CreateAlways:
			Win32OpenMode = CREATE_NEW;
			break;
		case IPlatformFile::FileOpenMode::OpenExisting:
			Win32OpenMode = OPEN_EXISTING;
			break;
		case IPlatformFile::FileOpenMode::OpenExistingOverwrite:
			Win32OpenMode = TRUNCATE_EXISTING;
			break;
		default:
			HERMES_ASSERT_LOG(false, "Unknown IPlatformFile::OpenMode value in WindowsFile::WindowsFile");
			break;
		}
		// TODO : sharing?

		wchar_t UTF16Path[GMaxFilePathLength];
		MultiByteToWideChar(CP_UTF8, 0, Path.data(), -1, UTF16Path, GMaxFilePathLength);
		auto File = CreateFileW(UTF16Path, Win32Access, FILE_SHARE_READ, nullptr, Win32OpenMode, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (File == INVALID_HANDLE_VALUE)
			return nullptr;
		return std::make_unique<WindowsFile>(File);
	}

	std::optional<String> PlatformFilesystem::ReadFileAsString(StringView Path)
	{
		auto File = OpenFile(Path, IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
		if (!File)
			return {};

		auto FileSize = File->Size();
		String Result(FileSize, '\0');
		File->Read(Result.data(), FileSize);

		return Result;
	}

	bool PlatformFilesystem::RemoveFile(StringView Path)
	{
		wchar_t UTF16Path[GMaxFilePathLength];
		MultiByteToWideChar(CP_UTF8, 0, Path.data(), -1, UTF16Path, GMaxFilePathLength);
		return DeleteFileW(UTF16Path);
	}
}

#endif
