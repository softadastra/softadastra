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
    [[nodiscard]] softadastra::ProcessLaunchResult launch(
        const softadastra::ProcessSpec &spec) override
    {
      if (launch_fails_)
      {
        return nullptr;
      }

      last_process_ =
          std::make_shared<TestProcessState>();
      last_spec_ = spec;

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

    [[nodiscard]] const std::optional<softadastra::ProcessSpec> &last_spec() const noexcept
    { return last_spec_; }

  private:
    bool launch_fails_{false};
    bool process_running_{true};
    bool stop_succeeds_{true};
    int launch_exit_code_{0};
    std::shared_ptr<TestProcessState> last_process_;
    std::optional<softadastra::ProcessSpec> last_spec_;
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

    [[nodiscard]] std::string host_name() const override
    {
      return "host";
    }

    [[nodiscard]] std::vector<softadastra::LocalNetworkAddress>
    local_addresses() const override
    {
      return {
          {
              softadastra::LocalAddressFamily::IPv4,
              "ethernet",
              "192.168.1.10",
          },
      };
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

  TEST(ControlClientTest, LooksUpSoftwareByPersistentProjectIdentityAndUpdatesRoot)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);
    const softadastra::ProjectIdentity identity("opaque-project-id");

    ASSERT_TRUE(client.register_software(
        softadastra::SoftwareId("project-app"),
        softadastra::ProcessSpec("./build/app", {}, "/old/project"),
        std::nullopt, identity));
    const auto entry = client.software_by_project_identity(identity);
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->id().value(), "project-app");
    EXPECT_TRUE(client.update_project_root(identity, "/new/project"));
    ASSERT_TRUE(client.start_software(entry->id()));
    ASSERT_TRUE(launcher.last_spec().has_value());
    EXPECT_EQ(launcher.last_spec()->executable(), "./build/app");
    EXPECT_EQ(launcher.last_spec()->working_directory(), "/new/project");
  }

  TEST(ControlClientTest, DoesNotReplaceAnExistingProjectIdentityWhenLinking)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);
    const softadastra::SoftwareId id("legacy");
    ASSERT_TRUE(client.register_software(id, softadastra::ProcessSpec("app")));
    EXPECT_TRUE(client.link_project(id, softadastra::ProjectIdentity("first"), "/project"));
    EXPECT_FALSE(client.link_project(id, softadastra::ProjectIdentity("second"), "/other"));
    ASSERT_TRUE(client.project_identity(id).has_value());
    EXPECT_EQ(client.project_identity(id)->value(), "first");
  }

  TEST(ControlClientTest, ReturnsStructuredSoftwareInventory)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);
    ASSERT_TRUE(client.register_software(
        softadastra::SoftwareId("api"),
        softadastra::ProcessSpec("./serve", {}, "/project/api"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8000)));
    const auto inventory = client.software();
    ASSERT_EQ(inventory.size(), 1U);
    EXPECT_EQ(inventory.front().id().value(), "api");
    EXPECT_EQ(inventory.front().process_spec().working_directory(), "/project/api");
    ASSERT_TRUE(inventory.front().access_point().has_value());
    EXPECT_EQ(inventory.front().access_point()->port(), 8000);
  }

  TEST(ControlClientTest, ReturnsLocalHostAccessInformation)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, launcher);
    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const auto access = client.local_access();

    ASSERT_TRUE(access.has_value());
    EXPECT_EQ(access->host_name, "host");
    ASSERT_EQ(access->addresses.size(), 1U);
    EXPECT_EQ(access->addresses[0].family, softadastra::LocalAddressFamily::IPv4);
    EXPECT_EQ(access->addresses[0].interface_name, "ethernet");
    EXPECT_EQ(access->addresses[0].value, "192.168.1.10");
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
