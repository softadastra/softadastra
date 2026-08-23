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

#include "platform/Network.hpp"
#include "platform/Platform.hpp"
#include "platform/Process.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"
#include "platform/Service.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <optional>

namespace
{

  class TestProcess final : public softadastra::Process
  {
  public:
    bool stop() override
    {
      running_ = false;
      exit_code_ = 0;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

    [[nodiscard]] std::optional<int> exit_code() noexcept override
    {
      return exit_code_;
    }

  private:
    bool running_{true};
    std::optional<int> exit_code_;
  };

  class TestProcessLauncher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] std::unique_ptr<softadastra::Process> launch(
        const softadastra::ProcessSpec &) override
    {
      return std::make_unique<TestProcess>();
    }
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
      return available_;
    }

    [[nodiscard]] bool is_connected() const noexcept override
    {
      return connected_;
    }

  private:
    bool available_{false};
    bool connected_{false};
  };

  class TestPlatform final : public softadastra::Platform
  {
  public:
    [[nodiscard]] softadastra::ProcessLauncher &
    process_launcher() noexcept override
    {
      return process_launcher_;
    }

    [[nodiscard]] const softadastra::ProcessLauncher &
    process_launcher() const noexcept override
    {
      return process_launcher_;
    }

    [[nodiscard]] softadastra::Service &
    service() noexcept override
    {
      return service_;
    }

    [[nodiscard]] const softadastra::Service &
    service() const noexcept override
    {
      return service_;
    }

    [[nodiscard]] softadastra::Network &
    network() noexcept override
    {
      return network_;
    }

    [[nodiscard]] const softadastra::Network &
    network() const noexcept override
    {
      return network_;
    }

  private:
    TestProcessLauncher process_launcher_;
    TestService service_;
    TestNetwork network_;
  };

  TEST(PlatformTest, ExposesProcessLauncherCapability)
  {
    TestPlatform platform;

    const softadastra::ProcessSpec spec("/usr/bin/example");

    auto process =
        platform.process_launcher().launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());
    EXPECT_FALSE(process->exit_code().has_value());
  }

  TEST(PlatformTest, ExposesServiceCapability)
  {
    TestPlatform platform;

    EXPECT_FALSE(platform.service().is_running());

    EXPECT_TRUE(platform.service().start());
    EXPECT_TRUE(platform.service().is_running());

    EXPECT_TRUE(platform.service().stop());
    EXPECT_FALSE(platform.service().is_running());
  }

  TEST(PlatformTest, ExposesNetworkCapability)
  {
    TestPlatform platform;

    EXPECT_FALSE(platform.network().is_available());
    EXPECT_FALSE(platform.network().is_connected());
  }

  TEST(PlatformTest, SupportsConstCapabilityAccess)
  {
    const TestPlatform platform;

    const softadastra::ProcessLauncher &process_launcher =
        platform.process_launcher();

    const softadastra::Service &service =
        platform.service();

    const softadastra::Network &network =
        platform.network();

    EXPECT_EQ(
        &process_launcher,
        &platform.process_launcher());

    EXPECT_EQ(
        &service,
        &platform.service());

    EXPECT_EQ(
        &network,
        &platform.network());
  }

  TEST(PlatformTest, SupportsUseThroughPlatformInterface)
  {
    TestPlatform concrete_platform;
    softadastra::Platform &platform =
        concrete_platform;

    const softadastra::ProcessSpec spec("/usr/bin/example");

    auto process =
        platform.process_launcher().launch(spec);

    ASSERT_NE(process, nullptr);

    EXPECT_TRUE(process->is_running());
    EXPECT_FALSE(process->exit_code().has_value());

    EXPECT_TRUE(process->stop());
    EXPECT_FALSE(process->is_running());

    ASSERT_TRUE(process->exit_code().has_value());
    EXPECT_EQ(process->exit_code().value(), 0);
  }

} // namespace
