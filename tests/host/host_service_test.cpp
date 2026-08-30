/**
 *
 *  @file host_service_test.cpp
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

#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/Network.hpp"
#include "platform/LocalFirewall.hpp"
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
#include <string>
#include <utility>

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

    [[nodiscard]] std::string primary_ipv4() const override
    {
      return primary_ipv4_;
    }

    [[nodiscard]] softadastra::NetworkCapability network_capability() const override
    {
      return capability_;
    }

    void set_capability(softadastra::NetworkCapability value)
    {
      capability_ = std::move(value);
    }

    void set_available(bool value) noexcept
    {
      available_ = value;
    }

    void set_connected(bool value) noexcept
    {
      connected_ = value;
    }

    void set_primary_ipv4(std::string value)
    {
      primary_ipv4_ = std::move(value);
    }

  private:
    bool available_{false};
    bool connected_{false};
    std::string primary_ipv4_;
    softadastra::NetworkCapability capability_;
  };

  class TestLocalFirewall final : public softadastra::LocalFirewall
  {
  public:
    [[nodiscard]] softadastra::LocalFirewallResult ensure(
        const softadastra::LocalFirewallRule &rule) override
    {
      ++ensure_calls;
      last_rule = rule;
      if (result == softadastra::LocalFirewallResult::Open && !already_open)
        owned = true;
      return result;
    }

    void release(const softadastra::LocalFirewallRule &rule) noexcept override
    {
      if (owned)
      {
        ++release_calls;
        last_released = rule;
        owned = false;
      }
    }

    softadastra::LocalFirewallResult result{softadastra::LocalFirewallResult::Open};
    bool already_open{false};
    bool owned{false};
    int ensure_calls{0};
    int release_calls{0};
    softadastra::LocalFirewallRule last_rule;
    softadastra::LocalFirewallRule last_released;
  };

  class TestManagedNetwork final : public softadastra::ManagedNetwork
  {
  public:
    [[nodiscard]] softadastra::ManagedNetworkStatus status() const override { return {softadastra::ManagedNetworkCapability::Available, state, {}, {}, {}}; }
    [[nodiscard]] softadastra::ManagedNetworkStartResult start() override { return softadastra::ManagedNetworkStartResult::Failed; }
    bool stop() override
    {
      ++stop_calls;
      state = softadastra::ManagedNetworkState::Stopped;
      return true;
    }
    softadastra::ManagedNetworkState state{softadastra::ManagedNetworkState::Stopped};
    int stop_calls{0};
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
    [[nodiscard]] softadastra::LocalFirewall &local_firewall() noexcept override { return firewall_; }
    [[nodiscard]] const softadastra::LocalFirewall &local_firewall() const noexcept override { return firewall_; }
    [[nodiscard]] softadastra::ManagedNetwork &managed_network() noexcept override { return managed_network_; }
    [[nodiscard]] const softadastra::ManagedNetwork &managed_network() const noexcept override { return managed_network_; }

    TestNetwork &test_network() noexcept
    {
      return network_;
    }

    TestLocalFirewall &test_firewall() noexcept { return firewall_; }

  private:
    TestProcessLauncher process_launcher_;
    TestService service_;
    TestNetwork network_;
    TestManagedNetwork managed_network_;
    TestLocalFirewall firewall_;
  };

  softadastra::NetworkCapability existing_network()
  {
    return {softadastra::NetworkState::Available, "192.168.1.6", "wlan0",
            softadastra::NetworkInterfaceType::Wifi,
            softadastra::LocalNetworkState::Existing,
            softadastra::ManagedNetworkCapability::Unavailable,
            "192.168.1.0/24"};
  }

  TEST(HostServiceTest, RegistersSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    EXPECT_TRUE(
        service.register_software(
            softadastra::SoftwareId("example"),
            softadastra::ProcessSpec("/usr/bin/example")));
  }

  TEST(HostServiceTest, StartsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_TRUE(service.start_software(id));

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(HostServiceTest, ReportsLocalAccessWithoutTouchingTheFirewall)
  {
    TestPlatform platform;
    platform.test_network().set_capability(existing_network());
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("example");
    ASSERT_TRUE(service.register_software(
        id, softadastra::ProcessSpec("/usr/bin/example"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8081)));

    EXPECT_TRUE(service.start_software(id));
    ASSERT_TRUE(service.local_access(id).has_value());
    EXPECT_EQ(platform.test_firewall().ensure_calls, 0);
  }

  TEST(HostServiceTest, DoesNotQueryFirewallWhenReportingLocalAccess)
  {
    TestPlatform platform;
    platform.test_network().set_capability(existing_network());
    ASSERT_EQ(platform.test_network().network_capability().local_subnet, "192.168.1.0/24");
    platform.test_firewall().result = softadastra::LocalFirewallResult::PermissionRequired;
    EXPECT_EQ(platform.test_firewall().result, softadastra::LocalFirewallResult::PermissionRequired);
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("example");
    ASSERT_TRUE(service.register_software(
        id, softadastra::ProcessSpec("/usr/bin/example"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8081)));

    ASSERT_TRUE(service.start_software(id));
    const auto access = service.local_access(id);
    ASSERT_TRUE(access.has_value());
    EXPECT_EQ(access->local_subnet, "192.168.1.0/24");
    EXPECT_EQ(access->network, softadastra::LocalAccessNetwork::Existing);
    EXPECT_EQ(platform.test_firewall().ensure_calls, 0);
    EXPECT_EQ(access->state, softadastra::LocalAccessState::Available);
  }

  TEST(HostServiceTest, DoesNotMutateExistingFirewallRules)
  {
    TestPlatform platform;
    platform.test_network().set_capability(existing_network());
    platform.test_firewall().already_open = true;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("example");
    ASSERT_TRUE(service.register_software(
        id, softadastra::ProcessSpec("/usr/bin/example"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8081)));
    ASSERT_TRUE(service.start_software(id));
    ASSERT_TRUE(service.local_access(id).has_value());
    ASSERT_TRUE(service.stop_software(id));
    EXPECT_EQ(platform.test_firewall().release_calls, 0);

    platform.test_firewall().already_open = false;
    ASSERT_TRUE(service.start_software(id));
    ASSERT_TRUE(service.local_access(id).has_value());
    ASSERT_TRUE(service.stop_software(id));
    EXPECT_EQ(platform.test_firewall().release_calls, 0);
  }

  TEST(HostServiceTest, DoesNotMutateFirewallWhenAccessPortChanges)
  {
    TestPlatform platform;
    platform.test_network().set_capability(existing_network());
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("example");
    ASSERT_TRUE(service.register_software(
        id, softadastra::ProcessSpec("/usr/bin/example"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8081)));
    ASSERT_TRUE(service.start_software(id));
    ASSERT_TRUE(service.local_access(id).has_value());
    ASSERT_TRUE(service.stop_software(id));
    ASSERT_TRUE(service.synchronize_software(
        id, softadastra::ProcessSpec("/usr/bin/example"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8082)));
    EXPECT_EQ(platform.test_firewall().release_calls, 0);
  }

  TEST(HostServiceTest, StopsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(service.start_software(id));
    EXPECT_TRUE(service.stop_software(id));

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(HostServiceTest, RestartsRegisteredSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));
    ASSERT_TRUE(service.start_software(id));

    EXPECT_TRUE(service.restart_software(id));

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(HostServiceTest, KeepsRunningSoftwareRunningAfterRefresh)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(service.start_software(id));

    service.refresh();

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(HostServiceTest, DetectsSuccessfulProcessExit)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(service.start_software(id));

    auto process = launcher.last_process();

    ASSERT_NE(process, nullptr);

    process->running = false;
    process->exit_code = 0;

    service.refresh();

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(HostServiceTest, DetectsFailedProcessExit)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(service.start_software(id));

    auto process = launcher.last_process();

    ASSERT_NE(process, nullptr);

    process->running = false;
    process->exit_code = 7;

    service.refresh();

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(HostServiceTest, PropagatesLaunchFailure)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    launcher.set_launch_fails(true);

    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_FALSE(service.start_software(id));

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(HostServiceTest, PropagatesStopFailure)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    launcher.set_stop_succeeds(false);

    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        service.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(service.start_software(id));

    EXPECT_FALSE(service.stop_software(id));

    EXPECT_EQ(
        service.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(HostServiceTest, ReturnsNoStateForUnknownSoftware)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);

    const softadastra::HostService service(host, launcher);

    EXPECT_FALSE(
        service.software_state(
                   softadastra::SoftwareId("unknown"))
            .has_value());
  }

  TEST(HostServiceTest, ReportsConnectedHost)
  {
    TestPlatform platform;
    platform.test_network().set_available(true);
    platform.test_network().set_connected(true);

    TestProcessLauncher launcher;
    softadastra::Host host(platform);

    const softadastra::HostService service(host, launcher);

    EXPECT_TRUE(service.connectivity_available());
    EXPECT_TRUE(service.connected());
  }

  TEST(HostServiceTest, NeverReportsConnectedWhenNetworkIsUnavailable)
  {
    TestPlatform platform;
    platform.test_network().set_available(false);
    platform.test_network().set_connected(true);

    TestProcessLauncher launcher;
    softadastra::Host host(platform);

    const softadastra::HostService service(host, launcher);

    EXPECT_FALSE(service.connectivity_available());
    EXPECT_FALSE(service.connected());
  }

  TEST(HostServiceTest, ReportsCurrentPrimaryIpv4InLocalAccess)
  {
    TestPlatform platform;
    platform.test_network().set_primary_ipv4("192.168.1.6");

    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    const softadastra::HostService service(host, launcher);

    EXPECT_EQ(service.local_access().primary_ipv4, "192.168.1.6");
  }

  TEST(HostServiceTest, StopsRunningManagedNetworkDuringShutdown)
  {
    TestPlatform platform;
    static_cast<void>(platform.managed_network().start());
    auto &managed = static_cast<TestManagedNetwork &>(platform.managed_network());
    managed.state = softadastra::ManagedNetworkState::Running;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);
    EXPECT_TRUE(service.shutdown());
    EXPECT_EQ(managed.stop_calls, 1);
  }

  TEST(HostServiceTest, DoesNotStopManagedNetworkWhenAlreadyStopped)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);
    EXPECT_TRUE(service.shutdown());
    EXPECT_EQ(static_cast<TestManagedNetwork &>(platform.managed_network()).stop_calls, 0);
  }

  TEST(HostServiceTest, ResolvesLocalGatewayTargetsDynamically)
  {
    TestPlatform platform;
    TestProcessLauncher launcher;
    softadastra::Host host(platform);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("stable-id");

    ASSERT_TRUE(service.register_software(
        id, softadastra::ProcessSpec("app"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080),
        std::nullopt, "phone-test"));
    ASSERT_TRUE(service.start_software(id));
    EXPECT_EQ(service.resolve("phone-test").result, softadastra::LocalGatewayLookup::Http);
    EXPECT_EQ(service.resolve("phone-test").port, 8080);
    EXPECT_EQ(service.resolve("phone-test.softadastra.home.arpa").result, softadastra::LocalGatewayLookup::Http);
    EXPECT_EQ(service.resolve("example.com").result, softadastra::LocalGatewayLookup::NotFound);

    ASSERT_TRUE(service.stop_software(id));
    ASSERT_TRUE(service.synchronize_software(
        id, softadastra::ProcessSpec("app"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 9000), "phone-api"));
    EXPECT_EQ(service.resolve("phone-test").result, softadastra::LocalGatewayLookup::NotFound);
    EXPECT_EQ(service.resolve("phone-api").result, softadastra::LocalGatewayLookup::Unavailable);
    ASSERT_TRUE(service.start_software(id));
    EXPECT_EQ(service.resolve("phone-api").port, 9000);

    ASSERT_TRUE(service.stop_software(id));
    EXPECT_EQ(service.resolve("phone-api").result, softadastra::LocalGatewayLookup::Unavailable);
    host.state().find_software(id)->set_state(softadastra::SoftwareState::Failed);
    EXPECT_EQ(service.resolve("phone-api").result, softadastra::LocalGatewayLookup::Unavailable);

    const softadastra::SoftwareId no_access("no-access");
    ASSERT_TRUE(service.register_software(no_access, softadastra::ProcessSpec("app"), std::nullopt, std::nullopt, "no-access"));
    ASSERT_TRUE(service.start_software(no_access));
    EXPECT_EQ(service.resolve("no-access").result, softadastra::LocalGatewayLookup::NotFound);
    ASSERT_TRUE(service.remove_software(id));
    EXPECT_EQ(service.resolve("phone-api").result, softadastra::LocalGatewayLookup::NotFound);
  }

} // namespace
