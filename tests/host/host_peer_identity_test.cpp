/**
 *
 *  @file host_peer_identity_test.cpp
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
#include "host/HostPeerIdentity.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(HostPeerIdentityTest, MatchesOnlyTheExpectedPersistentPublicIdentity)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-peer-identity";
    std::filesystem::remove_all(directory);

    softadastra::HostIdentity first(directory / "first");
    softadastra::HostIdentity second(directory / "second");
    ASSERT_TRUE(first.load_or_create());
    ASSERT_TRUE(second.load_or_create());

    EXPECT_TRUE(softadastra::HostPeerIdentity::valid(first.id()));
    EXPECT_TRUE(softadastra::HostPeerIdentity::matches(first.id(), first.id()));
    EXPECT_FALSE(softadastra::HostPeerIdentity::matches(first.id(), second.id()));
    EXPECT_FALSE(softadastra::HostPeerIdentity::valid("invalid"));

    std::filesystem::remove_all(directory);
  }

} // namespace
