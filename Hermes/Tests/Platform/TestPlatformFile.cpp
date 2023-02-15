#include <gtest/gtest.h>

#include "Platform/GenericPlatform/PlatformFile.h"

using namespace Hermes;

TEST(TestPlatformFile, OpenNewFile)
{
	auto File = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Read | IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(File);

	File->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt"));
}

TEST(TestPlatformFile, OpenExistingFile)
{
	auto NewFile = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Read | IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile);

	NewFile->Close();

	EXPECT_TRUE(PlatformFilesystem::FileExists(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt"));

	auto ExistingFile = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Read | IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::OpenExisting);
	EXPECT_TRUE(ExistingFile);

	ExistingFile->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt"));
}

TEST(TestPlatformFile, WriteAndReadFile)
{
	uint64 TestValue = 0xCAFEBABECAFEBABE;

	auto NewFile = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile);
	ASSERT_TRUE(NewFile->Write(&TestValue, sizeof(TestValue)));
	NewFile->Close();

	auto ExistingFile = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
	ASSERT_TRUE(ExistingFile);

	uint64 ReadValue;
	ASSERT_TRUE(ExistingFile->Read(&ReadValue, sizeof(ReadValue)));
	EXPECT_EQ(TestValue, ReadValue);
	ExistingFile->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt"));
}

TEST(TestPlatformFile, SizeSeekTell)
{
	auto NewFile = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile);

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

	ASSERT_TRUE(PlatformFilesystem::RemoveFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt"));
}

TEST(TestPlatformFile, ReadFileAsString)
{
	String Data = "CAFEBABE";

	auto NewFile = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Write, IPlatformFile::FileOpenMode::Create);
	ASSERT_TRUE(NewFile);
	ASSERT_TRUE(NewFile->Write(Data.c_str(), Data.length()));
	NewFile->Close();

	auto ExistingFile = PlatformFilesystem::OpenFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt", IPlatformFile::FileAccessMode::Read, IPlatformFile::FileOpenMode::OpenExisting);
	ASSERT_TRUE(ExistingFile);

	String ReadData(ExistingFile->Size(), '\0');
	ASSERT_TRUE(ExistingFile->Read(ReadData.data(), ExistingFile->Size()));
	EXPECT_EQ(Data, ReadData);

	ExistingFile->Close();

	ASSERT_TRUE(PlatformFilesystem::RemoveFile(String(HERMES_CURRENT_EXECUTABLE_DIR) + "/bar.txt"));
}
