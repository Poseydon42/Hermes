#include <gtest/gtest.h>

#include "Math/Common.h"
#include "VirtualFilesystem/FSDevice.h"
#include "VirtualFilesystem/VirtualFilesystem.h"
#include "Platform/GenericPlatform/PlatformFile.h"

using namespace Hermes;

// FIXME: move this into a generic testing library to be reused when needed
class FakePlatformFile : public IPlatformFile
{
public:
	virtual ~FakePlatformFile() override = default;

	FakePlatformFile(std::vector<uint8>& InData, bool InIsReadonly)
		: Data(InData)
		, IsReadonly(InIsReadonly)
	{
	}

	virtual size_t Size() const override
	{
		return Data.size();
	}

	virtual void Seek(size_t NewPosition) override
	{
		Pointer = Math::Min(NewPosition, Data.size());
	}

	virtual size_t Tell() const override
	{
		return Pointer;
	}

	virtual bool Write(const void* InData, size_t Size) override
	{
		if (IsReadonly)
			return false;

		if (Pointer + Size > Data.size())
			Data.resize(Pointer + Size);

		std::memcpy(Data.data() + Pointer, InData, Size);
		Pointer += Size;

		return true;
	}

	virtual bool Read(void* Buffer, size_t Size) override
	{
		if (Pointer + Size > Data.size())
			return false;

		std::memcpy(Buffer, Data.data() + Pointer, Size);
		return true;
	}

	virtual void Flush() override
	{
	}

	virtual void Close() override
	{
	}

private:
	std::vector<uint8>& Data;
	bool IsReadonly;
	size_t Pointer = 0;

	friend class FakeFSDevice;
};

class FakeFSDevice : public FSDevice
{
public:
	virtual ~FakeFSDevice() override = default;

	explicit FakeFSDevice(bool InIsMutable, std::vector<std::pair<String, std::vector<uint8>>>& InitialFiles)
		: IsDeviceMutable(InIsMutable)
	{
		for (const auto& File : InitialFiles)
		{
			ExistingFiles.emplace_back(File.first, File.second, false);
		}
	}

	virtual bool FileExists(StringView Path) const override
	{
		bool Result = std::ranges::find_if(ExistingFiles, [&](const auto& Element)
		{
			return Element.Name == Path;
		}) != ExistingFiles.end();

		return Result;
	}

	virtual bool IsMutable() const override
	{
		return IsDeviceMutable;
	}

	virtual std::unique_ptr<IPlatformFile> Open(StringView Path, FileOpenMode OpenMode, FileAccessMode AccessMode) const override
	{
		if (AccessMode == FileAccessMode::ReadWrite && !IsDeviceMutable)
			return nullptr;

		if (!FileExists(Path))
		{
			if (OpenMode == FileOpenMode::OpenExisting)
				return nullptr;

			ExistingFiles.emplace_back(String(Path), std::vector<uint8>(), false);
		}

		bool IsReadonly = (AccessMode != FileAccessMode::ReadWrite);
		auto File = std::ranges::find_if(ExistingFiles, [&](const auto& Element) { return Element.Name == Path;  });
		if (File->IsOpened)
			return nullptr;

		return std::make_unique<FakePlatformFile>(File->Data, IsReadonly);
	}
	
	virtual bool RemoveFile(StringView Path) const override
	{
		auto MaybeFile = std::ranges::find_if(ExistingFiles, [&](const auto& Element) { return Element.Name == Path;  });
		if (MaybeFile == ExistingFiles.end())
			return false;

		ExistingFiles.erase(MaybeFile);
		return true;
	}

private:
	bool IsDeviceMutable;
	struct File
	{
		String Name;
		std::vector<uint8> Data;
		bool IsOpened = false;
	};

	mutable std::vector<File> ExistingFiles;
};

class TestVirtualFilesystem : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		std::vector<std::pair<String, std::vector<uint8>>> ExistingFilesInMutableDevice =
		{
			{ "/text.txt", { 'f', 'o', 'o' }},
		};
		auto MutableDevice = std::make_unique<FakeFSDevice>(true, ExistingFilesInMutableDevice);
		MutableDeviceRef = MutableDevice.get();
		VirtualFilesystem::Mount("/", MountMode::ReadWrite, 0, std::move(MutableDevice));

		std::vector<std::pair<String, std::vector<uint8>>> ExistingFilesInImmutableDevice =
		{
			{ "/text.txt", { 'b', 'a', 'r' }},
			{ "/binary.bin", { 0xCA, 0xFE, 0xBA, 0xBE } }
		};
		auto ImmutableDevice = std::make_unique<FakeFSDevice>(false, ExistingFilesInImmutableDevice);
		ImmutableDeviceRef = ImmutableDevice.get();
		VirtualFilesystem::Mount("/", MountMode::ReadOnly, 1, std::move(ImmutableDevice));
	}

	FakeFSDevice* MutableDeviceRef = nullptr;
	FakeFSDevice* ImmutableDeviceRef = nullptr;
};

TEST_F(TestVirtualFilesystem, FileExists)
{
	EXPECT_TRUE(VirtualFilesystem::FileExists("/text.txt"));
	EXPECT_TRUE(VirtualFilesystem::FileExists("/binary.bin"));

	EXPECT_FALSE(VirtualFilesystem::FileExists("/foo.bar"));
}

TEST_F(TestVirtualFilesystem, OpenFile)
{
	EXPECT_TRUE(VirtualFilesystem::Open("/text.txt", FileOpenMode::OpenExisting, FileAccessMode::Read));
	EXPECT_TRUE(VirtualFilesystem::Open("/text.txt", FileOpenMode::OpenExisting, FileAccessMode::ReadWrite));

	EXPECT_TRUE(VirtualFilesystem::Open("/binary.bin", FileOpenMode::OpenExisting, FileAccessMode::Read));
	EXPECT_FALSE(VirtualFilesystem::Open("/binary.bin", FileOpenMode::OpenExisting, FileAccessMode::ReadWrite));

	EXPECT_FALSE(VirtualFilesystem::Open("/file_that_should_not_exist.txt", FileOpenMode::OpenExisting, FileAccessMode::Read));
	EXPECT_TRUE(VirtualFilesystem::Open("/file_that_should_not_exist.txt", FileOpenMode::CreateNew, FileAccessMode::ReadWrite));

	EXPECT_TRUE(MutableDeviceRef->FileExists("/file_that_should_not_exist.txt"));
	EXPECT_FALSE(ImmutableDeviceRef->FileExists("/file_that_should_not_exist.txt"));
}

TEST_F(TestVirtualFilesystem, Priority)
{
	auto ReadOnlyFile = VirtualFilesystem::Open("/text.txt", FileOpenMode::OpenExisting, FileAccessMode::Read);
	ASSERT_TRUE(ReadOnlyFile);

	char Buffer[3];
	ASSERT_TRUE(ReadOnlyFile->Read(&Buffer, sizeof(Buffer)));
	EXPECT_EQ(memcmp(Buffer, "bar", sizeof(Buffer)), 0);

	auto ReadWriteFile = VirtualFilesystem::Open("/text.txt", FileOpenMode::OpenExisting, FileAccessMode::ReadWrite);
	ASSERT_TRUE(ReadWriteFile);

	ASSERT_TRUE(ReadWriteFile->Read(&Buffer, sizeof(Buffer)));
	EXPECT_EQ(memcmp(Buffer, "foo", sizeof(Buffer)), 0);
}

TEST_F(TestVirtualFilesystem, ReadFileAsString)
{
	EXPECT_EQ(VirtualFilesystem::ReadFileAsString("/text.txt").value_or(""), "bar");
}

TEST_F(TestVirtualFilesystem, ReadFileAsBytes)
{
	std::vector<uint8> Expected = { 0xCA, 0xFE, 0xBA, 0xBE };
	EXPECT_EQ(VirtualFilesystem::ReadFileAsBytes("/binary.bin").value_or(std::vector<uint8>()), Expected);
}

TEST_F(TestVirtualFilesystem, RemoveFile)
{
	ASSERT_FALSE(VirtualFilesystem::FileExists("/foo.bar.baz"));

	EXPECT_TRUE(VirtualFilesystem::Open("/foo.bar.baz", FileOpenMode::CreateNew, FileAccessMode::ReadWrite));
	EXPECT_TRUE(VirtualFilesystem::FileExists("/foo.bar.baz"));
	EXPECT_TRUE(MutableDeviceRef->FileExists("/foo.bar.baz"));
	EXPECT_FALSE(ImmutableDeviceRef->FileExists("/foo.bar.baz"));

	EXPECT_TRUE(VirtualFilesystem::RemoveFile("/foo.bar.baz"));

	EXPECT_FALSE(VirtualFilesystem::FileExists("/foo.bar.baz"));
	EXPECT_FALSE(ImmutableDeviceRef->FileExists("/foo.bar.baz"));

	EXPECT_FALSE(VirtualFilesystem::RemoveFile("/binary.bin"));
}
