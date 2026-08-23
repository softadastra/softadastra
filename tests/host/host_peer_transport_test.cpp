/**
 *
 *  @file host_peer_transport_test.cpp
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
#include "host/HostPeerClient.hpp"
#include "host/HostPeerServer.hpp"
#include "host/HostPeerTrust.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(HostPeerTransportTest, PinsCertificateIdentityAndServesMinimalProtocol)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-peer-transport-test";
    std::filesystem::remove_all(directory);
    softadastra::HostIdentity server_identity(directory / "server-identity");
    softadastra::HostIdentity other_identity(directory / "other-identity");
    ASSERT_TRUE(server_identity.load_or_create());
    ASSERT_TRUE(other_identity.load_or_create());
    softadastra::HostPeerServer server(
        server_identity,
        directory / "server-tls",
        "127.0.0.1",
        0,
        "network-ready");
    ASSERT_TRUE(server.start());

    const softadastra::HostPeerClient accepted(
        softadastra::HostPeerTrust("127.0.0.1", server_identity.id()),
        server.port());
    EXPECT_EQ(accepted.request("identity"), "identity " + server_identity.id() + "\n");
    EXPECT_EQ(accepted.request("ping"), "pong\n");
    EXPECT_EQ(accepted.request("infrastructure"), "infrastructure network-ready\n");

    const softadastra::HostPeerClient rejected(
        softadastra::HostPeerTrust("127.0.0.1", other_identity.id()),
        server.port());
    EXPECT_FALSE(rejected.request("ping").has_value());
    server.stop();
    std::filesystem::remove_all(directory);
  }

  TEST(HostPeerTransportTest, KeepsPeersIndependentAcrossPeerLossAndReconnect)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-peer-reconnect-test";
    std::filesystem::remove_all(directory);
    softadastra::HostIdentity a_identity(directory / "a-identity");
    softadastra::HostIdentity b_identity(directory / "b-identity");
    softadastra::HostIdentity c_identity(directory / "c-identity");
    ASSERT_TRUE(a_identity.load_or_create());
    ASSERT_TRUE(b_identity.load_or_create());
    ASSERT_TRUE(c_identity.load_or_create());
    softadastra::HostPeerServer a_server(
        a_identity, directory / "a-tls", "127.0.0.1", 0, "a-ready");
    softadastra::HostPeerServer b_server(
        b_identity, directory / "b-tls", "127.0.0.1", 0, "b-ready");
    softadastra::HostPeerServer c_server(
        c_identity, directory / "c-tls", "127.0.0.1", 0, "c-ready");
    ASSERT_TRUE(a_server.start());
    ASSERT_TRUE(b_server.start());
    ASSERT_TRUE(c_server.start());

    const auto b_port = b_server.port();
    const softadastra::HostPeerClient a_to_b(
        softadastra::HostPeerTrust("127.0.0.1", b_identity.id()), b_port);
    const softadastra::HostPeerClient c_to_b(
        softadastra::HostPeerTrust("127.0.0.1", b_identity.id()), b_port);
    EXPECT_EQ(a_to_b.request("ping"), "pong\n");
    EXPECT_EQ(c_to_b.request("ping"), "pong\n");

    b_server.stop();
    EXPECT_FALSE(a_to_b.request("ping").has_value());
    EXPECT_FALSE(c_to_b.request("ping").has_value());

    const softadastra::HostPeerClient a_to_a(
        softadastra::HostPeerTrust("127.0.0.1", a_identity.id()), a_server.port());
    const softadastra::HostPeerClient c_to_c(
        softadastra::HostPeerTrust("127.0.0.1", c_identity.id()), c_server.port());
    EXPECT_EQ(a_to_a.request("ping"), "pong\n");
    EXPECT_EQ(c_to_c.request("ping"), "pong\n");

    ASSERT_TRUE(b_server.start());
    const softadastra::HostPeerClient reconnected_a_to_b(
        softadastra::HostPeerTrust("127.0.0.1", b_identity.id()), b_server.port());
    const softadastra::HostPeerClient reconnected_c_to_b(
        softadastra::HostPeerTrust("127.0.0.1", b_identity.id()), b_server.port());
    EXPECT_EQ(reconnected_a_to_b.request("ping"), "pong\n");
    EXPECT_EQ(reconnected_c_to_b.request("ping"), "pong\n");
    a_server.stop();
    b_server.stop();
    c_server.stop();
    std::filesystem::remove_all(directory);
  }
} // namespace
