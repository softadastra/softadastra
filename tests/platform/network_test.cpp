/**
 *
 *  @file network_test.cpp
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

#include "platform/Network.hpp"
#include <gtest/gtest.h>

namespace
{
  class TestNetwork final : public softadastra::Network
  {
  public:
    TestNetwork(bool available, bool connected)
        : available_(available),
          connected_(connected)
    {
    }

    [[nodiscard]] bool is_available() const noexcept override
    {
      return available_;
    }

    [[nodiscard]] bool is_connected() const noexcept override
    {
      return connected_;
    }

  private:
    bool available_;
    bool connected_;
  };

  class AddressNetwork final : public softadastra::Network
  {
  public:
    explicit AddressNetwork(std::vector<softadastra::LocalNetworkAddress> addresses)
        : addresses_(std::move(addresses))
    {
    }

    [[nodiscard]] bool is_available() const noexcept override { return true; }
    [[nodiscard]] bool is_connected() const noexcept override { return true; }
    [[nodiscard]] std::vector<softadastra::LocalNetworkAddress>
    local_addresses() const override { return addresses_; }

  private:
    std::vector<softadastra::LocalNetworkAddress> addresses_;
  };

} // namespace

TEST(NetworkTest, ReportsUnavailableNetwork)
{
  const TestNetwork network(false, false);

  EXPECT_FALSE(network.is_available());
  EXPECT_FALSE(network.is_connected());
}

TEST(NetworkTest, ReportsAvailableDisconnectedNetwork)
{
  const TestNetwork network(true, false);

  EXPECT_TRUE(network.is_available());
  EXPECT_FALSE(network.is_connected());
}

TEST(NetworkTest, ReportsAvailableConnectedNetwork)
{
  const TestNetwork network(true, true);

  EXPECT_TRUE(network.is_available());
  EXPECT_TRUE(network.is_connected());
}

TEST(NetworkTest, SupportsUseThroughNetworkInterface)
{
  const TestNetwork concrete(true, true);
  const softadastra::Network &network = concrete;

  EXPECT_TRUE(network.is_available());
  EXPECT_TRUE(network.is_connected());
}

TEST(NetworkTest, AssociatesPrimaryIpv4WithItsReportedInterface)
{
  const AddressNetwork network({
      {softadastra::LocalAddressFamily::IPv4, "wlp108s0", "10.56.116.55"}});

  const auto capability = network.network_capability();

  EXPECT_EQ(capability.state, softadastra::NetworkState::Available);
  EXPECT_EQ(capability.primary_ipv4, "10.56.116.55");
  EXPECT_EQ(capability.primary_interface, "wlp108s0");
  EXPECT_EQ(capability.local_network_state,
            softadastra::LocalNetworkState::Existing);
}

TEST(NetworkTest, DoesNotReportLoopbackAsLocalNetwork)
{
  const AddressNetwork network({
      {softadastra::LocalAddressFamily::IPv4, "lo", "127.0.0.1"}});

  const auto capability = network.network_capability();

  EXPECT_EQ(capability.state, softadastra::NetworkState::Unavailable);
  EXPECT_EQ(capability.local_network_state,
            softadastra::LocalNetworkState::Unavailable);
}

TEST(NetworkTest, NamesAllInterfaceTypes)
{
  EXPECT_STREQ(softadastra::network_interface_type_name(
                   softadastra::NetworkInterfaceType::Wifi), "wifi");
  EXPECT_STREQ(softadastra::network_interface_type_name(
                   softadastra::NetworkInterfaceType::Ethernet), "ethernet");
  EXPECT_STREQ(softadastra::network_interface_type_name(
                   softadastra::NetworkInterfaceType::Other), "other");
  EXPECT_STREQ(softadastra::network_interface_type_name(
                   softadastra::NetworkInterfaceType::Unknown), "unknown");
}

TEST(NetworkTest, UnsupportedPlatformFallbackDoesNotClaimManagedNetwork)
{
  const AddressNetwork network({});

  const auto capability = network.network_capability();

  EXPECT_EQ(capability.managed_network_capability,
            softadastra::ManagedNetworkCapability::Unavailable);
}
