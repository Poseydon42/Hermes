#ifdef HERMES_PLATFORM_WINDOWS

#include "WindowsFile.h"

namespace Hermes
{
	WindowsFile::~WindowsFile()
	{
		Close();
	}

	WindowsFile::WindowsFile(WindowsFile&& Other) : File(Other.File)
	{
		Other.File = INVALID_HANDLE_VALUE;
	}

	WindowsFile& WindowsFile::operator=(WindowsFile&& Other)
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
		LARGE_INTEGER NewPointer;
		NewPointer.QuadPart = NewPosition;
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

	bool WindowsFile::Write(const uint8* Data, size_t Size)
	{
		DWORD Dummy;
		return WriteFile(File, Data, (DWORD)Size, &Dummy, 0);
	}

	bool WindowsFile::Read(uint8* Buffer, size_t Size)
	{
		DWORD Dummy;
		return ReadFile(File, Buffer, (DWORD)Size, &Dummy, 0);
	}

	void WindowsFile::Flush()
	{
		FlushFileBuffers(File);
	}

	bool WindowsFile::IsValid() const
	{
		return File != INVALID_HANDLE_VALUE;
	}

	void WindowsFile::Close()
	{
		if (IsValid())
			CloseHandle(File);
		File = INVALID_HANDLE_VALUE;
	}

	WindowsFile::WindowsFile(const String& Path, IPlatformFile::FileAccessMode Access, IPlatformFile::FileOpenMode OpenMode)
	{
		DWORD Win32Access = 0;
		if ((int)(Access & IPlatformFile::FileAccessMode::Read))
		{
			Win32Access |= GENERIC_READ;
		}
		if ((int)(Access & IPlatformFile::FileAccessMode::Write))
		{
			Win32Access |= GENERIC_WRITE;
		}

		DWORD Win32OpenMode = 0;
		switch (OpenMode)
		{
		case FileOpenMode::Create:
			Win32OpenMode = CREATE_ALWAYS;
			break;
		case FileOpenMode::CreateAlways:
			Win32OpenMode = CREATE_NEW;
			break;
		case FileOpenMode::OpenExisting:
			Win32OpenMode = OPEN_EXISTING;
			break;
		case FileOpenMode::OpenExistingOverwrite:
			Win32OpenMode = TRUNCATE_EXISTING;
			break;
		default:
			HERMES_ASSERT_LOG(false, L"Unknown IPlatformFile::OpenMode value in WindowsFile::WindowsFile");
			break;
		}
		// TODO : sharing?
		File = CreateFileW(Path.c_str(), Win32Access, FILE_SHARE_READ, 0, Win32OpenMode, FILE_ATTRIBUTE_NORMAL, 0);
	}

	bool PlatformFilesystem::FileExists(const String& Path)
	{
		WIN32_FIND_DATA FindData;
		HANDLE FoundFile = FindFirstFileW(Path.c_str(), &FindData);
		bool Result = (FoundFile != INVALID_HANDLE_VALUE);
		if (Result)
			FindClose(FoundFile);
		return Result;
	}

	std::shared_ptr<IPlatformFile> PlatformFilesystem::OpenFile(const String& Path, IPlatformFile::FileAccessMode Access, IPlatformFile::FileOpenMode OpenMode)
	{
		WindowsFile Result = WindowsFile(Path, Access, OpenMode);
		if (!Result.IsValid()) return std::shared_ptr<IPlatformFile>(nullptr);
		return std::make_shared<WindowsFile>(std::move(Result));
	}
	
	bool PlatformFilesystem::RemoveFile(const String& Path)
	{
		return DeleteFileW(Path.c_str());
	}
}

#endif
