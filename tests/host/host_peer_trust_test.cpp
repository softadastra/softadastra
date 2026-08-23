/**
 *
 *  @file host_peer_trust_test.cpp
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
#include "host/HostPeerTrust.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(HostPeerTrustTest, AcceptsOnlyTheExplicitlyPinnedHostId)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-peer-trust";
    std::filesystem::remove_all(directory);
    softadastra::HostIdentity trusted(directory / "trusted");
    softadastra::HostIdentity untrusted(directory / "untrusted");
    ASSERT_TRUE(trusted.load_or_create());
    ASSERT_TRUE(untrusted.load_or_create());

    const softadastra::HostPeerTrust trust("127.0.0.1", trusted.id());
    EXPECT_TRUE(trust.accepts(trusted.id()));
    EXPECT_FALSE(trust.accepts(untrusted.id()));
    std::filesystem::remove_all(directory);
  }

} // namespace
