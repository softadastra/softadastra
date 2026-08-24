/**
 *
 *  @file local_access_test.cpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#include "host/LocalAccess.hpp"

#include <gtest/gtest.h>

namespace
{
  softadastra::NetworkCapability existing_network(std::string ipv4)
  {
    return {softadastra::NetworkState::Available, std::move(ipv4), "wlp108s0",
            softadastra::NetworkInterfaceType::Wifi,
            softadastra::LocalNetworkState::Existing,
            softadastra::ManagedNetworkCapability::Available};
  }

  TEST(LocalAccessTest, ResolvesHttpFromCurrentPrimaryIpv4)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto access = softadastra::resolve_local_access(
        point.value(), existing_network("10.56.116.55"), true);

    EXPECT_EQ(access.state, softadastra::LocalAccessState::Available);
    EXPECT_EQ(access.ipv4, "10.56.116.55");
    EXPECT_EQ(access.url, "http://10.56.116.55:8080");
  }

  TEST(LocalAccessTest, UsesNewIpv4ForEachResolution)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Https, 8443);
    ASSERT_TRUE(point.has_value());

    const auto first = softadastra::resolve_local_access(
        point.value(), existing_network("192.168.1.6"), true);
    const auto second = softadastra::resolve_local_access(
        point.value(), existing_network("10.56.116.55"), true);

    EXPECT_EQ(first.url, "https://192.168.1.6:8443");
    EXPECT_EQ(second.url, "https://10.56.116.55:8443");
  }

  TEST(LocalAccessTest, DoesNotResolveStoppedSoftwareOrUnavailableNetwork)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto stopped = softadastra::resolve_local_access(
        point.value(), existing_network("10.56.116.55"), false);
    auto unavailable_network = existing_network("10.56.116.55");
    unavailable_network.state = softadastra::NetworkState::Unavailable;
    unavailable_network.local_network_state =
        softadastra::LocalNetworkState::Unavailable;
    const auto no_network = softadastra::resolve_local_access(
        point.value(), std::move(unavailable_network), true);

    EXPECT_EQ(stopped.state, softadastra::LocalAccessState::Unavailable);
    EXPECT_TRUE(stopped.url.empty());
    EXPECT_EQ(no_network.state, softadastra::LocalAccessState::Unavailable);
    EXPECT_TRUE(no_network.url.empty());
    EXPECT_EQ(no_network.managed_network_capability,
              softadastra::ManagedNetworkCapability::Available);
  }
}
