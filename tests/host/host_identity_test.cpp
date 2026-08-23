/**
 *
 *  @file host_identity_test.cpp
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

#include "host/HostIdentity.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(HostIdentityTest, PersistsGeneratedIdentityAndSecret)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-host-identity-test";
    const auto path = directory / "identity";
    std::filesystem::remove_all(directory);

    softadastra::HostIdentity first(path);
    ASSERT_TRUE(first.load_or_create());
    const auto id = first.id();
    const auto secret = first.secret();

    softadastra::HostIdentity second(path);
    ASSERT_TRUE(second.load_or_create());
    EXPECT_EQ(second.id(), id);
    EXPECT_EQ(second.secret(), secret);
    EXPECT_NE(id, secret);
    std::filesystem::remove_all(directory);
  }
}
