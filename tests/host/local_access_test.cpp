/**
 *
 *  @file local_access_test.cpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#include "host/LocalAccess.hpp"
#include "platform/QrCode.hpp"

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

  softadastra::ManagedNetworkStatus managed_network(
      softadastra::ManagedNetworkState state, std::string ipv4)
  {
    return {softadastra::ManagedNetworkCapability::Available, state, "wlan1",
            std::move(ipv4), "Softadastra-test"};
  }

  softadastra::NetworkCapability no_local_network()
  {
    return {softadastra::NetworkState::Unavailable, {}, {},
            softadastra::NetworkInterfaceType::Unknown,
            softadastra::LocalNetworkState::Unavailable,
            softadastra::ManagedNetworkCapability::Available};
  }

  TEST(LocalAccessTest, ResolvesHttpFromCurrentPrimaryIpv4WhenManagedNetworkIsStopped)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto access = softadastra::resolve_local_access(
        point.value(), existing_network("10.56.116.55"),
        managed_network(softadastra::ManagedNetworkState::Stopped, {}), true);

    EXPECT_EQ(access.state, softadastra::LocalAccessState::Available);
    EXPECT_EQ(access.ipv4, "10.56.116.55");
    EXPECT_EQ(access.url, "http://10.56.116.55:8080");
    EXPECT_EQ(access.network, softadastra::LocalAccessNetwork::Existing);
  }

  TEST(LocalAccessTest, PrefersExistingNetworkOverRunningManagedNetwork)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto access = softadastra::resolve_local_access(
        point.value(), existing_network("192.168.1.20"),
        managed_network(softadastra::ManagedNetworkState::Running, "10.42.0.1"),
        true);

    EXPECT_EQ(access.state, softadastra::LocalAccessState::Available);
    EXPECT_EQ(access.network, softadastra::LocalAccessNetwork::Existing);
    EXPECT_EQ(access.ipv4, "192.168.1.20");
    EXPECT_EQ(access.url, "http://192.168.1.20:8080");
  }

  TEST(LocalAccessTest, UsesResolvedUrlForQrCode)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto access = softadastra::resolve_local_access(
        point.value(), existing_network("192.168.1.20"),
        managed_network(softadastra::ManagedNetworkState::Running, "10.42.0.1"),
        true);

    EXPECT_EQ(softadastra::QrCode::render(access.url),
              softadastra::QrCode::render("http://192.168.1.20:8080"));
  }

  TEST(LocalAccessTest, UsesNewIpv4ForEachResolution)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Https, 8443);
    ASSERT_TRUE(point.has_value());

    const auto first = softadastra::resolve_local_access(
        point.value(), existing_network("192.168.1.6"),
        managed_network(softadastra::ManagedNetworkState::Stopped, {}), true);
    const auto second = softadastra::resolve_local_access(
        point.value(), existing_network("10.56.116.55"),
        managed_network(softadastra::ManagedNetworkState::Stopped, {}), true);

    EXPECT_EQ(first.url, "https://192.168.1.6:8443");
    EXPECT_EQ(second.url, "https://10.56.116.55:8443");
  }

  TEST(LocalAccessTest, UsesNewManagedIpv4ForEachResolution)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto first = softadastra::resolve_local_access(
        point.value(), no_local_network(),
        managed_network(softadastra::ManagedNetworkState::Running, "10.42.0.1"),
        true);
    const auto second = softadastra::resolve_local_access(
        point.value(), no_local_network(),
        managed_network(softadastra::ManagedNetworkState::Running, "10.43.0.1"),
        true);

    EXPECT_EQ(first.url, "http://10.42.0.1:8080");
    EXPECT_EQ(second.url, "http://10.43.0.1:8080");
  }

  TEST(LocalAccessTest, FallsBackWhenRunningManagedNetworkHasNoUsableIpv4)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto access = softadastra::resolve_local_access(
        point.value(), existing_network("192.168.1.20"),
        managed_network(softadastra::ManagedNetworkState::Running, {}),
        true);

    EXPECT_EQ(access.state, softadastra::LocalAccessState::Available);
    EXPECT_EQ(access.network, softadastra::LocalAccessNetwork::Existing);
    EXPECT_EQ(access.url, "http://192.168.1.20:8080");
  }

  TEST(LocalAccessTest, DoesNotResolveStoppedSoftwareOrUnavailableNetwork)
  {
    const auto point = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(point.has_value());

    const auto stopped = softadastra::resolve_local_access(
        point.value(), existing_network("10.56.116.55"),
        managed_network(softadastra::ManagedNetworkState::Stopped, {}), false);
    auto unavailable_network = existing_network("10.56.116.55");
    unavailable_network.state = softadastra::NetworkState::Unavailable;
    unavailable_network.local_network_state =
        softadastra::LocalNetworkState::Unavailable;
    const auto no_network = softadastra::resolve_local_access(
        point.value(), std::move(unavailable_network),
        managed_network(softadastra::ManagedNetworkState::Stopped, {}), true);

    EXPECT_EQ(stopped.state, softadastra::LocalAccessState::Unavailable);
    EXPECT_TRUE(stopped.url.empty());
    EXPECT_EQ(no_network.state, softadastra::LocalAccessState::Unavailable);
    EXPECT_TRUE(no_network.url.empty());
    EXPECT_EQ(no_network.managed_network_capability,
              softadastra::ManagedNetworkCapability::Available);
  }
}
