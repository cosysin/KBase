
#include "stdafx.h"

#include "gtest\gtest.h"
#include "kbase\files\file_util.h"

#include <iostream>
#include <utility>

#include "kbase\path_service.h"

using namespace kbase;

namespace {

const FilePath dir = PathService::Get(DIR_CURRENT).AppendTo(L"abc_test");
const FilePath file_path = dir.AppendTo(L"abc.txt");

void CreateDirectoryWithFile()
{
    CreateDirectoryW(dir.value().c_str(), nullptr);
    auto handle =
        CreateFileW(file_path.value().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);   
    }
}

}   // namespace

TEST(FileUtilTest, MakeAbsoluteFilePath)
{
    const FilePath path_tags[] { FilePath(L"."), FilePath(L"..") };
    for (const auto& tag : path_tags) {
        auto&& abs_path = MakeAbsoluteFilePath(tag);
        std::wcout << abs_path.value() << std::endl;
        EXPECT_NE(abs_path, tag);
    }
}

TEST(FileUtilTest, PathExists)
{
    typedef std::pair<FilePath, bool> TestPathExistsPair;
    TestPathExistsPair test_exists[] {
        { FilePath(L"C:\\path_exists_test.fuck"), false },
        { FilePath(L"C:\\Windows"), true }
    };

    for (const auto& p : test_exists) {
        EXPECT_EQ(PathExists(p.first), p.second);
    }
}

TEST(FileUtilTest, RemoveFile)
{
    CreateDirectoryWithFile();
    ASSERT_TRUE(PathExists(dir));
    ASSERT_TRUE(PathExists(file_path));
    RemoveFile(file_path, false);
    RemoveFile(dir, false);
    EXPECT_FALSE(PathExists(dir));
    CreateDirectoryWithFile();
    RemoveFile(dir, true);
    EXPECT_FALSE(PathExists(dir));
}