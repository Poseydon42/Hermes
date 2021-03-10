#ifdef HERMES_PLATFORM_WINDOWS

#include "Core/Core.h"
#include "Core/Application/Application.h"
#include "Platform/GenericPlatform/PlatformFile.h"

class SandboxApp : public Hermes::IApplication
{
public:
	bool Init() override
	{
		bool ShouldBeFalse = Hermes::PlatformFilesystem::FileExists(L"SomeTextFile.txt");
		HERMES_ASSERT(!ShouldBeFalse);
		std::shared_ptr<Hermes::IPlatformFile> NullFile = Hermes::PlatformFilesystem::OpenFile(L"SomeTextFile.txt", Hermes::IPlatformFile::FileAccessMode::Read, Hermes::IPlatformFile::FileOpenMode::OpenExisting);
		HERMES_ASSERT(NullFile.get() == 0);
		std::shared_ptr<Hermes::IPlatformFile> TxtFile = Hermes::PlatformFilesystem::OpenFile(L"SomeTextFile.txt", Hermes::IPlatformFile::FileAccessMode::Read | Hermes::IPlatformFile::FileAccessMode::Write, Hermes::IPlatformFile::FileOpenMode::Create);
		HERMES_ASSERT(TxtFile.get() != 0);
		std::shared_ptr<Hermes::IPlatformFile> NullFile2 = Hermes::PlatformFilesystem::OpenFile(L"SomeTextFile.txt", Hermes::IPlatformFile::FileAccessMode::Read, Hermes::IPlatformFile::FileOpenMode::CreateAlways);
		HERMES_ASSERT(NullFile2.get() == 0);
		
		HERMES_ASSERT(Hermes::PlatformFilesystem::FileExists(L"SomeTextFile.txt"));
		HERMES_ASSERT(TxtFile->IsValid());
		HERMES_ASSERT(TxtFile->Tell() == 0);

		Hermes::uint8 Data[256];
		for (Hermes::uint16 i = 0; i < sizeof(Data); i++)
			Data[i] = i;
		HERMES_ASSERT(TxtFile->Write(Data, sizeof(Data)));
		TxtFile->Flush();

		Hermes::uint8 ReadBuffer[256];
		TxtFile->Seek(0);
		HERMES_ASSERT(TxtFile->Read(ReadBuffer, sizeof(ReadBuffer)));
		HERMES_ASSERT(memcmp(Data, ReadBuffer, sizeof(Data)) == 0);

		TxtFile->Seek(0x10);
		HERMES_ASSERT(TxtFile->Tell() == 0x10);

		HERMES_ASSERT(TxtFile->Size() == sizeof(Data));
		TxtFile->Close();
		Hermes::PlatformFilesystem::RemoveFile(L"SomeTextFile.txt");
		HERMES_ASSERT(!Hermes::PlatformFilesystem::FileExists(L"SomeTextFile.txt"));
		
		return true;
	}

	void Run(float Delta) override
	{
	}

	void Shutdown() override
	{
		
	}
};

extern "C" _declspec(dllexport) Hermes::IApplication* CreateApplicationInstance()
{
	auto App = new SandboxApp;

	return App;
}

#endif
