#include <gtest/gtest.h>

#include "Platform/GenericPlatform/PlatformFile.h"

using namespace Hermes;

class TestPlatformFile : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		PlatformFilesystem::Mount(HERMES_CURRENT_EXECUTABLE_DIR, "/", 1);
	}

	virtual void TearDown() override
	{
		PlatformFilesystem::ClearMountedFolders();
	}
};

TEST_F(TestPlatformFile, OpenNewFile)
{
	auto File = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Read | IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(File && File->IsValid());

	File->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile("bar.txt"));
}

TEST_F(TestPlatformFile, OpenExistingFile)
{
	auto NewFile = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Read | IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile && NewFile->IsValid());

	NewFile->Close();

	EXPECT_TRUE(PlatformFilesystem::FileExists("bar.txt"));

	auto ExistingFile = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Read | IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::OpenExisting);
	EXPECT_TRUE(ExistingFile && ExistingFile->IsValid());

	ExistingFile->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile("bar.txt"));
}

TEST_F(TestPlatformFile, WriteAndReadFile)
{
	uint64 TestValue = 0xCAFEBABECAFEBABE;

	auto NewFile = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile && NewFile->IsValid());
	ASSERT_TRUE(NewFile->Write(&TestValue, sizeof(TestValue)));
	NewFile->Close();

	auto ExistingFile = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
	ASSERT_TRUE(ExistingFile && ExistingFile->IsValid());

	uint64 ReadValue;
	ASSERT_TRUE(ExistingFile->Read(&ReadValue, sizeof(ReadValue)));
	EXPECT_EQ(TestValue, ReadValue);
	ExistingFile->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile("bar.txt"));
}

TEST_F(TestPlatformFile, SizeSeekTell)
{
	auto NewFile = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile && NewFile->IsValid());

	EXPECT_EQ(NewFile->Size(), 0);
	EXPECT_EQ(NewFile->Tell(), 0);

	NewFile->Seek(3000);

	EXPECT_EQ(NewFile->Size(), 0);
	EXPECT_EQ(NewFile->Tell(), 0);

	uint64 TestValue = 0xCAFEBABECAFEBABE;
	ASSERT_TRUE(NewFile->Write(&TestValue, sizeof(TestValue)));

	EXPECT_EQ(NewFile->Size(), sizeof(TestValue));
	EXPECT_EQ(NewFile->Tell(), sizeof(TestValue));

	NewFile->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile("bar.txt"));
}

TEST_F(TestPlatformFile, ReadFileAsString)
{
	String Data = "CAFEBABE";

	auto NewFile = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile && NewFile->IsValid());
	ASSERT_TRUE(NewFile->Write(Data.c_str(), Data.length()));
	NewFile->Close();

	auto ExistingFile = PlatformFilesystem::OpenFile("bar.txt", IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
	ASSERT_TRUE(ExistingFile && ExistingFile->IsValid());

	String ReadData(ExistingFile->Size(), '\0');
	ASSERT_TRUE(ExistingFile->Read(ReadData.data(), ExistingFile->Size()));
	EXPECT_EQ(Data, ReadData);
}
