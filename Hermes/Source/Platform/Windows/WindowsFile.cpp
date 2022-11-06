#ifdef HERMES_PLATFORM_WINDOWS

#include "WindowsFile.h"

#include "Logging/Logger.h"

namespace Hermes
{
	// TODO: do we need to expose it?
	static constexpr size_t GMaxFilePathLength = 8192;

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

	WindowsFile::WindowsFile(const String& Path, FileAccessMode Access, FileOpenMode OpenMode)
	{
		DWORD Win32Access = 0;
		if (static_cast<int>(Access & FileAccessMode::Read))
		{
			Win32Access |= GENERIC_READ;
		}
		if (static_cast<int>(Access & FileAccessMode::Write))
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
			HERMES_ASSERT_LOG(false, "Unknown IPlatformFile::OpenMode value in WindowsFile::WindowsFile");
			break;
		}
		// TODO : sharing?
		
		wchar_t UTF16Path[GMaxFilePathLength];
		MultiByteToWideChar(CP_UTF8, 0, Path.c_str(), -1, UTF16Path, GMaxFilePathLength);
		File = CreateFileW(UTF16Path, Win32Access, FILE_SHARE_READ, nullptr, Win32OpenMode, FILE_ATTRIBUTE_NORMAL,		                   nullptr);
	}

	std::multiset<MountRecord> PlatformFilesystem::Mounts;

	static bool TryFindFile(const std::multiset<MountRecord>& Mounts, const String& Path, bool AlwaysSearchForFile, String& Result)
	{
		auto Iterator = Mounts.rbegin();
		while (Iterator != Mounts.rend())
		{
			auto& Record = *Iterator;
			if (Path.rfind(Record.To, 0) != String::npos)
			{
				if (AlwaysSearchForFile)
				{
					Result = Record.From + "/" + String(Path.begin() + static_cast<int32>(Record.To.length()), Path.end());
					
					wchar_t UTF16Result[GMaxFilePathLength];
					MultiByteToWideChar(CP_UTF8, 0, Result.c_str(), -1, UTF16Result, GMaxFilePathLength);

					WIN32_FIND_DATA FindData;
					HANDLE FoundFile = FindFirstFileW(UTF16Result, &FindData);
					if (FoundFile != INVALID_HANDLE_VALUE)
					{
						FindClose(FoundFile);
						return true;
					}
				}
				else
				{
					// We need to just find a suitable directory according to
					// mounting table, not the specific file
					Result = Record.From + "/" + String(Path.begin() + static_cast<int32>(Record.To.length()), Path.end());
					return true;
				}
			}

			++Iterator;
		}
		return false;
	}

	bool PlatformFilesystem::FileExists(const String& Path)
	{
		String TruePath;
		return TryFindFile(Mounts, Path, true, TruePath);
	}

	std::unique_ptr<IPlatformFile> PlatformFilesystem::OpenFile(const String& Path, IPlatformFile::FileAccessMode Access, IPlatformFile::FileOpenMode OpenMode)
	{
		auto FixedPath = Path;
		if (FixedPath.empty() || FixedPath[0] != L'/')
			FixedPath = "/" + FixedPath;
		String TruePath;
		bool AlwaysOpenExistingFile = 
			(OpenMode == IPlatformFile::FileOpenMode::OpenExisting || 
			 OpenMode == IPlatformFile::FileOpenMode::OpenExistingOverwrite);
		if (TryFindFile(Mounts, FixedPath, AlwaysOpenExistingFile, TruePath))
			return std::make_unique<WindowsFile>(TruePath, Access, OpenMode);
		return std::make_unique<WindowsFile>("/", Access, OpenMode);
	}

	void PlatformFilesystem::Mount(const String& FolderPath, const String& MountingPath, uint32 Priority)
	{
		MountRecord NewMount;
		NewMount.From = FolderPath;
		NewMount.To = MountingPath;
		NewMount.Priority = Priority;
		Mounts.insert(NewMount);
	}

	void PlatformFilesystem::ClearMountedFolders()
	{
		Mounts.clear();
	}

	bool PlatformFilesystem::RemoveFile(const String& Path)
	{
		String TruePath;
		if (!TryFindFile(Mounts, Path, true, TruePath))
			return false;
		
		wchar_t UTF16Path[GMaxFilePathLength];
		MultiByteToWideChar(CP_UTF8, 0, Path.c_str(), -1, UTF16Path, GMaxFilePathLength);
		return DeleteFileW(UTF16Path);
	}
}

#endif
