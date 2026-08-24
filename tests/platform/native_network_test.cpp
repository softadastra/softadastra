/**
 *
 *  @file native_network_test.cpp
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

#include "platform/NativeNetwork.hpp"

#include <gtest/gtest.h>

namespace
{
  TEST(NativeNetworkTest, ReportsHostNameWithoutInternetAccess)
  {
    const softadastra::NativeNetwork network;

    EXPECT_FALSE(network.host_name().empty());
  }

  TEST(NativeNetworkTest, ReportsOnlyUsableLocalAddresses)
  {
    const softadastra::NativeNetwork network;
    const auto addresses = network.local_addresses();

    for (const auto &address : addresses)
    {
      EXPECT_FALSE(address.interface_name.empty());
      EXPECT_FALSE(address.value.empty());
      EXPECT_FALSE(address.value.starts_with("127."));
      EXPECT_NE(address.value, "::1");
      EXPECT_TRUE(address.family == softadastra::LocalAddressFamily::IPv4 ||
                  address.family == softadastra::LocalAddressFamily::IPv6);
    }
  }

  TEST(NativeNetworkTest, ReportsPrimaryIpv4FromCurrentNetworkState)
  {
    const softadastra::NativeNetwork network;
    const auto primary = network.primary_ipv4();

    if (primary.empty())
    {
      SUCCEED();
      return;
    }

    const auto addresses = network.local_addresses();
    bool found = false;

    for (const auto &address : addresses)
    {
      if (address.family == softadastra::LocalAddressFamily::IPv4 &&
          address.value == primary)
      {
        found = true;
        break;
      }
    }

    EXPECT_TRUE(found);
  }

  TEST(NativeNetworkTest, ReportsNetworkCapabilityConsistently)
  {
    const softadastra::NativeNetwork network;
    const auto capability = network.network_capability();

    if (capability.state == softadastra::NetworkState::Unavailable)
    {
      EXPECT_TRUE(capability.primary_ipv4.empty());
      EXPECT_TRUE(capability.primary_interface.empty());
      EXPECT_EQ(capability.local_network_state,
                softadastra::LocalNetworkState::Unavailable);
      return;
    }

    EXPECT_FALSE(capability.primary_ipv4.empty());
    EXPECT_FALSE(capability.primary_interface.empty());
    EXPECT_NE(capability.primary_ipv4.rfind("127.", 0), 0U);
    EXPECT_EQ(capability.local_network_state,
              softadastra::LocalNetworkState::Existing);
  }
} // namespace
