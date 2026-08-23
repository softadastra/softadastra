/**
 *
 *  @file control_server_test.cpp
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

#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/Network.hpp"
#include "platform/Platform.hpp"
#include "platform/Process.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"
#include "platform/Service.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace
{

  class TestProcess final : public softadastra::Process
  {
  public:
    explicit TestProcess(
        bool running = true,
        bool stop_succeeds = true) noexcept
        : running_(running),
          stop_succeeds_(stop_succeeds)
    {
    }

    bool stop() override
    {
      if (!stop_succeeds_)
      {
        return false;
      }

      running_ = false;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

  private:
    bool running_;
    bool stop_succeeds_;
  };

  class TestProcessLauncher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] std::unique_ptr<softadastra::Process> launch(
        const softadastra::ProcessSpec &) override
    {
      if (launch_fails_)
      {
        return nullptr;
      }

      return std::make_unique<TestProcess>(
          process_running_,
          stop_succeeds_);
    }

    void set_launch_fails(bool value) noexcept
    {
      launch_fails_ = value;
    }

    void set_process_running(bool value) noexcept
    {
      process_running_ = value;
    }

    void set_stop_succeeds(bool value) noexcept
    {
      stop_succeeds_ = value;
    }

  private:
    bool launch_fails_{false};
    bool process_running_{true};
    bool stop_succeeds_{true};
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

    void set_available(bool value) noexcept
    {
      available_ = value;
    }

    void set_connected(bool value) noexcept
    {
      connected_ = value;
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

    TestNetwork &test_network() noexcept
    {
      return network_;
    }

  private:
    TestProcessLauncher process_launcher_;
    TestService service_;
    TestNetwork network_;
  };

  TEST(ControlServerTest, RegistersSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    EXPECT_TRUE(
        server.register_software(
            softadastra::SoftwareId("example"),
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_EQ(host.state().software_count(), 1U);
  }

  TEST(ControlServerTest, RejectsDuplicateSoftwareRegistration)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        server.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_FALSE(
        server.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/other")));
  }

  TEST(ControlServerTest, StartsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        server.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_TRUE(server.start_software(id));

    ASSERT_TRUE(server.software_state(id).has_value());

    EXPECT_EQ(
        server.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(ControlServerTest, StopsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        server.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(server.start_software(id));

    EXPECT_TRUE(server.stop_software(id));

    ASSERT_TRUE(server.software_state(id).has_value());

    EXPECT_EQ(
        server.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(ControlServerTest, PropagatesStartFailure)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;

    launcher.set_launch_fails(true);

    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        server.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_FALSE(server.start_software(id));

    ASSERT_TRUE(server.software_state(id).has_value());

    EXPECT_EQ(
        server.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(ControlServerTest, RejectsProcessThatDoesNotRemainRunning)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;

    launcher.set_process_running(false);

    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        server.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_FALSE(server.start_software(id));

    ASSERT_TRUE(server.software_state(id).has_value());

    EXPECT_EQ(
        server.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(ControlServerTest, PropagatesStopFailure)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;

    launcher.set_stop_succeeds(false);

    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        server.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(server.start_software(id));

    EXPECT_FALSE(server.stop_software(id));

    ASSERT_TRUE(server.software_state(id).has_value());

    EXPECT_EQ(
        server.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(ControlServerTest, ReturnsNoStateForUnknownSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);

    const softadastra::ControlServer server(host_service);

    EXPECT_FALSE(
        server.software_state(
                  softadastra::SoftwareId("unknown"))
            .has_value());
  }

  TEST(ControlServerTest, ReportsConnectivityAvailability)
  {
    TestPlatform platform;
    platform.test_network().set_available(true);

    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);

    const softadastra::ControlServer server(host_service);

    EXPECT_TRUE(server.connectivity_available());
  }

  TEST(ControlServerTest, ReportsConnectedHost)
  {
    TestPlatform platform;
    platform.test_network().set_available(true);
    platform.test_network().set_connected(true);

    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);

    const softadastra::ControlServer server(host_service);

    EXPECT_TRUE(server.connectivity_available());
    EXPECT_TRUE(server.connected());
  }

  TEST(ControlServerTest, DoesNotReportConnectedWithoutNetworkAvailability)
  {
    TestPlatform platform;
    platform.test_network().set_available(false);
    platform.test_network().set_connected(true);

    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);

    const softadastra::ControlServer server(host_service);

    EXPECT_FALSE(server.connectivity_available());
    EXPECT_FALSE(server.connected());
  }

} // namespace
