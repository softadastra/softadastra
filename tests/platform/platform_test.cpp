/**
 *
 *  @file platform_test.cpp
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

#include "platform/Platform.hpp"
#include <gtest/gtest.h>

namespace
{
  class TestProcess final : public softadastra::Process
  {
  public:
    bool start() override
    {
      running_ = true;
      return true;
    }

    bool stop() override
    {
      running_ = false;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

  private:
    bool running_{false};
  };

  class TestService final : public softadastra::Service
  {
  public:
    bool start() override
    {
      running_ = true;
      return true;
    }

    bool stop() override
    {
      running_ = false;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

  private:
    bool running_{false};
  };

  class TestNetwork final : public softadastra::Network
  {
  public:
    [[nodiscard]] bool is_available() const noexcept override
    {
      return true;
    }

    [[nodiscard]] bool is_connected() const noexcept override
    {
      return true;
    }
  };

  class TestPlatform final : public softadastra::Platform
  {
  public:
    [[nodiscard]] softadastra::Process &process() noexcept override
    {
      return process_;
    }

    [[nodiscard]] const softadastra::Process &process() const noexcept override
    {
      return process_;
    }

    [[nodiscard]] softadastra::Service &service() noexcept override
    {
      return service_;
    }

    [[nodiscard]] const softadastra::Service &service() const noexcept override
    {
      return service_;
    }

    [[nodiscard]] softadastra::Network &network() noexcept override
    {
      return network_;
    }

    [[nodiscard]] const softadastra::Network &network() const noexcept override
    {
      return network_;
    }

  private:
    TestProcess process_;
    TestService service_;
    TestNetwork network_;
  };

} // namespace

TEST(PlatformTest, ExposesProcessCapability)
{
  TestPlatform platform;

  EXPECT_FALSE(platform.process().is_running());

  EXPECT_TRUE(platform.process().start());
  EXPECT_TRUE(platform.process().is_running());
}

TEST(PlatformTest, ExposesServiceCapability)
{
  TestPlatform platform;

  EXPECT_FALSE(platform.service().is_running());

  EXPECT_TRUE(platform.service().start());
  EXPECT_TRUE(platform.service().is_running());
}

TEST(PlatformTest, ExposesNetworkCapability)
{
  TestPlatform platform;

  EXPECT_TRUE(platform.network().is_available());
  EXPECT_TRUE(platform.network().is_connected());
}

TEST(PlatformTest, SupportsConstCapabilityAccess)
{
  const TestPlatform platform;

  EXPECT_FALSE(platform.process().is_running());
  EXPECT_FALSE(platform.service().is_running());
  EXPECT_TRUE(platform.network().is_available());
  EXPECT_TRUE(platform.network().is_connected());
}

TEST(PlatformTest, SupportsUseThroughPlatformInterface)
{
  TestPlatform concrete;
  softadastra::Platform &platform = concrete;

  EXPECT_TRUE(platform.process().start());
  EXPECT_TRUE(platform.process().is_running());

  EXPECT_TRUE(platform.service().start());
  EXPECT_TRUE(platform.service().is_running());

  EXPECT_TRUE(platform.network().is_available());
}
