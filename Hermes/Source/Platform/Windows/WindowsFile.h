#pragma once

#ifdef HERMES_PLATFORM_WINDOWS

#include <Windows.h>

#include "Core/Core.h"
#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes
{
	class HERMES_API WindowsFile : public IPlatformFile
	{
	public:
		WindowsFile(const String& Path, IPlatformFile::FileAccessMode Access, IPlatformFile::FileOpenMode OpenMode);
		
		~WindowsFile();

		WindowsFile(WindowsFile&& Other);
		WindowsFile(WindowsFile&& Other) noexcept;

		WindowsFile& operator=(WindowsFile&& Other);
		WindowsFile& operator=(WindowsFile&& Other) noexcept;
		
		size_t Size() const override;
		
		void Seek(size_t NewPosition) override;
		
		size_t Tell() const override;
		
		bool Write(const uint8* Data, size_t Size) override;
		bool Write(const void* Data, size_t Size) override;
		
		bool Read(uint8* Buffer, size_t Size) override;
		bool Read(void* Buffer, size_t Size) override;
		
		void Flush() override;

		bool IsValid() const override;
		
		void Close() override;
	private:
		HANDLE File;
	};
}

#endif
