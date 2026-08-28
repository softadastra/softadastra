/**
 * @file host_peer_transport_windows_test.cpp
 * @brief Windows V1 explicitly does not provide HostPeerTransport.
 */

#include "host/HostIdentity.hpp"
#include "host/HostPeerClient.hpp"
#include "host/HostPeerServer.hpp"
#include "host/HostPeerTrust.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(HostPeerTransportWindowsTest, IsExplicitlyUnavailable)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-peer-transport-windows-test";
    std::filesystem::remove_all(directory);

    softadastra::HostIdentity identity(directory / "identity");
    ASSERT_TRUE(identity.load_or_create());
    softadastra::HostPeerServer server(
        identity, directory / "tls", "127.0.0.1", 0, "unavailable");
    EXPECT_FALSE(server.start());
    EXPECT_FALSE(server.listening());

    const softadastra::HostPeerClient client(
        softadastra::HostPeerTrust("127.0.0.1", identity.id()), 1);
    EXPECT_FALSE(client.request("ping").has_value());

    std::filesystem::remove_all(directory);
  }
} // namespace
