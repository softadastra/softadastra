/**
 *
 *  @file native_data_directory_test.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Softadastra
 */

#include "platform/NativeDataDirectory.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(NativeDataDirectoryTest, ProvidesStableNonEmptyPath)
  {
    EXPECT_FALSE(softadastra::NativeDataDirectory::path().empty());
    EXPECT_EQ(
        softadastra::NativeDataDirectory::path(),
        softadastra::NativeDataDirectory::path());
  }

  TEST(NativeDataDirectoryTest, CreatesStateDirectory)
  {
    ASSERT_TRUE(softadastra::NativeDataDirectory::ensure_exists());
    EXPECT_TRUE(std::filesystem::is_directory(
        softadastra::NativeDataDirectory::path()));
  }
} // namespace
