/**
 *
 *  @file control_client_test.cpp
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

#include "control/ControlClient.hpp"
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
#include <optional>

namespace
{
  struct TestProcessState
  {
    bool running{true};
    bool stop_succeeds{true};
    std::optional<int> exit_code;
  };

  class TestProcess final : public softadastra::Process
  {
  public:
    explicit TestProcess(
        std::shared_ptr<TestProcessState> state)
        : state_(std::move(state))
    {
    }

    bool stop() override
    {
      if (!state_->stop_succeeds)
      {
        return false;
      }

      state_->running = false;
      state_->exit_code = 0;

      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return state_->running;
    }

    [[nodiscard]] std::optional<int> exit_code() noexcept override
    {
      return state_->exit_code;
    }

  private:
    std::shared_ptr<TestProcessState> state_;
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

      last_process_ =
          std::make_shared<TestProcessState>();

      last_process_->running = process_running_;
      last_process_->stop_succeeds = stop_succeeds_;

      if (!process_running_)
      {
        last_process_->exit_code = launch_exit_code_;
      }

      return std::make_unique<TestProcess>(
          last_process_);
    }

    void set_launch_fails(bool value) noexcept
    {
      launch_fails_ = value;
    }

    void set_process_running(bool value) noexcept
    {
      process_running_ = value;
    }

    void set_launch_exit_code(int code) noexcept
    {
      launch_exit_code_ = code;
    }

    void set_stop_succeeds(bool value) noexcept
    {
      stop_succeeds_ = value;
    }

    [[nodiscard]] std::shared_ptr<TestProcessState>
    last_process() const noexcept
    {
      return last_process_;
    }

  private:
    bool launch_fails_{false};
    bool process_running_{true};
    bool stop_succeeds_{true};
    int launch_exit_code_{0};
    std::shared_ptr<TestProcessState> last_process_;
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

  TEST(ControlClientTest, RegistersSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    EXPECT_TRUE(
        client.register_software(
            softadastra::SoftwareId("example"),
            softadastra::ProcessSpec("/usr/bin/example")));
  }

  TEST(ControlClientTest, StartsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_TRUE(client.start_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(ControlClientTest, StopsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(client.start_software(id));
    EXPECT_TRUE(client.stop_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(ControlClientTest, RestartsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));
    ASSERT_TRUE(client.start_software(id));

    EXPECT_TRUE(client.restart_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(ControlClientTest, DetectsSuccessfulProcessExitAfterRefresh)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(client.start_software(id));

    auto process = launcher.last_process();

    ASSERT_NE(process, nullptr);

    process->running = false;
    process->exit_code = 0;

    client.refresh();

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(ControlClientTest, DetectsFailedProcessExitAfterRefresh)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(client.start_software(id));

    auto process = launcher.last_process();

    ASSERT_NE(process, nullptr);

    process->running = false;
    process->exit_code = 7;

    client.refresh();

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(ControlClientTest, KeepsRunningSoftwareRunningAfterRefresh)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(client.start_software(id));

    client.refresh();

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(ControlClientTest, PropagatesLaunchFailure)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    launcher.set_launch_fails(true);

    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_FALSE(client.start_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(ControlClientTest, ReturnsNoStateForUnknownSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::ControlClient client(server);

    EXPECT_FALSE(
        client.software_state(
                  softadastra::SoftwareId("unknown"))
            .has_value());
  }

  TEST(ControlClientTest, ReportsConnectivityAvailability)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);

    const softadastra::ControlClient client(server);

    EXPECT_FALSE(client.connectivity_available());
  }

} // namespace
