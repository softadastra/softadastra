/**
 *
 *  @file connectivity_manager_test.cpp
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

#include "connectivity/ConnectivityManager.hpp"
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

} // namespace

TEST(ConnectivityManagerTest, ReportsUnavailableConnectivity)
{
  TestNetwork network(false, false);
  const softadastra::ConnectivityManager manager(network);

  EXPECT_FALSE(manager.is_available());
  EXPECT_FALSE(manager.is_connected());
}

TEST(ConnectivityManagerTest, ReportsAvailableButDisconnectedConnectivity)
{
  TestNetwork network(true, false);
  const softadastra::ConnectivityManager manager(network);

  EXPECT_TRUE(manager.is_available());
  EXPECT_FALSE(manager.is_connected());
}

TEST(ConnectivityManagerTest, ReportsAvailableConnectedConnectivity)
{
  TestNetwork network(true, true);
  const softadastra::ConnectivityManager manager(network);

  EXPECT_TRUE(manager.is_available());
  EXPECT_TRUE(manager.is_connected());
}

TEST(ConnectivityManagerTest, DoesNotReportConnectedWhenNetworkIsUnavailable)
{
  TestNetwork network(false, true);
  const softadastra::ConnectivityManager manager(network);

  EXPECT_FALSE(manager.is_available());
  EXPECT_FALSE(manager.is_connected());
}
