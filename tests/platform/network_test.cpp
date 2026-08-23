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
