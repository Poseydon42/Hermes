#include <gtest/gtest.h>

#include "Platform/GenericPlatform/PlatformFile.h"
#include "VirtualFilesystem/DirectoryFSDevice.h"
#include "VirtualFilesystem/VirtualFilesystem.h"

using namespace Hermes;

class TestDirectoryFSDevice : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		Device = std::make_unique<DirectoryFSDevice>(HERMES_TEST_FILES_DIR);
	}

	std::unique_ptr<DirectoryFSDevice> Device;
};

TEST_F(TestDirectoryFSDevice, FileExists)
{
	EXPECT_TRUE(Device->FileExists("/text.txt"));

	EXPECT_FALSE(Device->FileExists("text.txt"));
	EXPECT_FALSE(Device->FileExists("/binary.bin"));
}

TEST_F(TestDirectoryFSDevice, IsMutable)
{
	EXPECT_TRUE(Device->IsMutable());
}

TEST_F(TestDirectoryFSDevice, OpenAndRemove)
{
	EXPECT_TRUE(Device->Open("/text.txt", FileOpenMode::OpenExisting, FileAccessMode::Read));
	EXPECT_TRUE(Device->Open("/text.txt", FileOpenMode::OpenExisting, FileAccessMode::ReadWrite));

	EXPECT_FALSE(Device->Open("/binary.bin", FileOpenMode::OpenExisting, FileAccessMode::Read));
	EXPECT_TRUE(Device->Open("/binary.bin", FileOpenMode::CreateNew, FileAccessMode::ReadWrite));

	EXPECT_TRUE(Device->RemoveFile("/binary.bin")) << "REMOVE THIS FILE IF THE TEST FAILED";

	EXPECT_FALSE(Device->RemoveFile("/binary.bin"));
}
