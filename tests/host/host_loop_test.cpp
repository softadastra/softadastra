/**
 *
 *  @file host_loop_test.cpp
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
#include "host/HostLoop.hpp"
#include "host/HostProfile.hpp"
#include "host/HostStateFile.hpp"
#include "platform/ManagedNetwork.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/Process.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/Service.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace
{
  class CountingProcess final : public softadastra::Process
  {
  public:
    explicit CountingProcess(std::atomic_size_t &refresh_count) noexcept
        : refresh_count_(refresh_count)
    {
    }

    bool stop() override
    {
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      ++refresh_count_;
      return true;
    }

    [[nodiscard]] std::optional<int> exit_code() noexcept override
    {
      return std::nullopt;
    }

  private:
    std::atomic_size_t &refresh_count_;
  };

  class CountingProcessLauncher final : public softadastra::ProcessLauncher
  {
  public:
    explicit CountingProcessLauncher(
        std::atomic_size_t &refresh_count) noexcept
        : refresh_count_(refresh_count)
    {
    }

    [[nodiscard]] softadastra::ProcessLaunchResult launch(
        const softadastra::ProcessSpec &) override
    {
      return std::make_unique<CountingProcess>(refresh_count_);
    }

  private:
    std::atomic_size_t &refresh_count_;
  };

  struct ShutdownProcessState
  {
    bool running{true};
    bool stop_succeeds{true};
    std::size_t stop_count{0};
  };

  class ShutdownProcess final : public softadastra::Process
  {
  public:
    explicit ShutdownProcess(std::shared_ptr<ShutdownProcessState> state) noexcept
        : state_(std::move(state))
    {
    }

    bool stop() override
    {
      ++state_->stop_count;

      if (!state_->stop_succeeds)
      {
        return false;
      }

      state_->running = false;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return state_->running;
    }

    [[nodiscard]] std::optional<int> exit_code() noexcept override
    {
      return state_->running ? std::nullopt : std::optional<int>(0);
    }

  private:
    std::shared_ptr<ShutdownProcessState> state_;
  };

  class ShutdownProcessLauncher final : public softadastra::ProcessLauncher
  {
  public:
    explicit ShutdownProcessLauncher(bool stop_succeeds) noexcept
        : stop_succeeds_(stop_succeeds)
    {
    }

    [[nodiscard]] softadastra::ProcessLaunchResult launch(
        const softadastra::ProcessSpec &) override
    {
      auto state = std::make_shared<ShutdownProcessState>();
      state->stop_succeeds = stop_succeeds_;
      states_.push_back(state);
      return std::make_unique<ShutdownProcess>(std::move(state));
    }

    [[nodiscard]] const std::vector<std::shared_ptr<ShutdownProcessState>> &
    states() const noexcept
    {
      return states_;
    }

  private:
    bool stop_succeeds_;
    std::vector<std::shared_ptr<ShutdownProcessState>> states_;
  };

  class RestorationProcessLauncher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] softadastra::ProcessLaunchResult launch(
        const softadastra::ProcessSpec &spec) override
    {
      launched_.push_back(spec.executable());

      if (spec.executable() == "failing")
      {
        return softadastra::ProcessLaunchError::LaunchFailed;
      }

      return std::make_unique<ShutdownProcess>(
          std::make_shared<ShutdownProcessState>());
    }

    [[nodiscard]] const std::vector<std::string> &launched() const noexcept
    {
      return launched_;
    }

  private:
    std::vector<std::string> launched_;
  };

  class TestNetwork final : public softadastra::Network
  {
  public:
    [[nodiscard]] bool is_available() const noexcept override { return false; }
    [[nodiscard]] bool is_connected() const noexcept override { return false; }
  };

  class TestService final : public softadastra::Service
  {
  public:
    bool start() override { return true; }
    bool stop() override { return true; }
    [[nodiscard]] bool is_running() const noexcept override { return true; }
  };

  class TestManagedNetwork final : public softadastra::ManagedNetwork
  {
  public:
    [[nodiscard]] softadastra::ManagedNetworkStatus status() const override
    {
      return current;
    }
    [[nodiscard]] softadastra::ManagedNetworkStartResult start() override
    {
      ++start_calls;
      if (start_result == softadastra::ManagedNetworkStartResult::Started)
      {
        current.state = softadastra::ManagedNetworkState::Running;
      }
      return start_result;
    }
    bool stop() override
    {
      current.state = softadastra::ManagedNetworkState::Stopped;
      return true;
    }

    softadastra::ManagedNetworkStatus current{
        softadastra::ManagedNetworkCapability::Available,
        softadastra::ManagedNetworkState::Stopped, "wlan1", "10.42.0.1", "test"};
    softadastra::ManagedNetworkStartResult start_result{
        softadastra::ManagedNetworkStartResult::Started};
    int start_calls{0};
  };

  class TestPlatform final : public softadastra::Platform
  {
  public:
    [[nodiscard]] softadastra::ProcessLauncher &process_launcher() noexcept override { return launcher_; }
    [[nodiscard]] const softadastra::ProcessLauncher &process_launcher() const noexcept override { return launcher_; }
    [[nodiscard]] softadastra::Service &service() noexcept override { return service_; }
    [[nodiscard]] const softadastra::Service &service() const noexcept override { return service_; }
    [[nodiscard]] softadastra::Network &network() noexcept override { return network_; }
    [[nodiscard]] const softadastra::Network &network() const noexcept override { return network_; }
    [[nodiscard]] softadastra::ManagedNetwork &managed_network() noexcept override { return managed_; }
    [[nodiscard]] const softadastra::ManagedNetwork &managed_network() const noexcept override { return managed_; }

    std::atomic_size_t refresh_count{0};
    CountingProcessLauncher launcher_{refresh_count};
    TestService service_;
    TestNetwork network_;
    TestManagedNetwork managed_;
  };

  template <typename Predicate>
  bool wait_until(Predicate predicate)
  {
    for (int attempt = 0; attempt < 100; ++attempt)
    {
      if (predicate())
      {
        return true;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return predicate();
  }

  void run_until_started(softadastra::HostLoop &loop, std::function<void()> check)
  {
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });
    EXPECT_TRUE(wait_until([&loop]()
                           { return loop.is_running(); }));
    check();
    loop.request_stop();
    thread.join();
    EXPECT_TRUE(completed);
  }

  TEST(HostLoopTest, StandardProfileNeverStartsManagedNetwork)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-standard-profile";
    std::filesystem::remove_all(directory);
    TestPlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostProfileStore profile(directory / "host-profile");
    ASSERT_TRUE(profile.load("host"));
    softadastra::HostLoop loop(service, state_file, std::chrono::milliseconds(1), nullptr, &profile);
    run_until_started(loop, [&platform]()
                      { EXPECT_EQ(platform.managed_.start_calls, 0); });
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, BoxStartsAvailableStoppedManagedNetwork)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-box-start";
    std::filesystem::remove_all(directory);
    TestPlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostProfileStore profile(directory / "host-profile");
    ASSERT_TRUE(profile.provision_box("host"));
    softadastra::HostLoop loop(service, state_file, std::chrono::milliseconds(1), nullptr, &profile);
    run_until_started(loop, [&platform]()
                      { EXPECT_EQ(platform.managed_.start_calls, 1); EXPECT_EQ(platform.managed_.status().state, softadastra::ManagedNetworkState::Running); });
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, BoxDoesNotStartAnAlreadyRunningManagedNetwork)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-box-running";
    std::filesystem::remove_all(directory);
    TestPlatform platform;
    platform.managed_.current.state = softadastra::ManagedNetworkState::Running;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostProfileStore profile(directory / "host-profile");
    ASSERT_TRUE(profile.provision_box("host"));
    softadastra::HostLoop loop(service, state_file, std::chrono::milliseconds(1), nullptr, &profile);
    run_until_started(loop, [&platform]()
                      { EXPECT_EQ(platform.managed_.start_calls, 0); EXPECT_EQ(platform.managed_.status().state, softadastra::ManagedNetworkState::Running); });
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, BoxRemainsRunningWhenManagedNetworkIsUnavailable)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-box-unavailable";
    std::filesystem::remove_all(directory);
    TestPlatform platform;
    platform.managed_.current.capability = softadastra::ManagedNetworkCapability::Unavailable;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostProfileStore profile(directory / "host-profile");
    ASSERT_TRUE(profile.provision_box("host"));
    softadastra::HostLoop loop(service, state_file, std::chrono::milliseconds(1), nullptr, &profile);
    run_until_started(loop, [&platform]()
                      { EXPECT_EQ(platform.managed_.start_calls, 0); EXPECT_EQ(platform.managed_.status().capability, softadastra::ManagedNetworkCapability::Unavailable); });
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, BoxRemainsRunningWhenManagedNetworkStartFails)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-box-failure";
    std::filesystem::remove_all(directory);
    TestPlatform platform;
    platform.managed_.start_result = softadastra::ManagedNetworkStartResult::Failed;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostProfileStore profile(directory / "host-profile");
    ASSERT_TRUE(profile.provision_box("host"));
    softadastra::HostLoop loop(service, state_file, std::chrono::milliseconds(1), nullptr, &profile);
    run_until_started(loop, [&platform]()
                      { EXPECT_EQ(platform.managed_.start_calls, 1); EXPECT_EQ(platform.managed_.status().state, softadastra::ManagedNetworkState::Stopped); });
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, RunsUntilAnInternalStopRequest)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-loop-stop";
    std::filesystem::remove_all(directory);
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });

    EXPECT_TRUE(wait_until([&loop]()
                           { return loop.is_running(); }));
    loop.request_stop();
    thread.join();

    EXPECT_TRUE(completed);
    EXPECT_FALSE(loop.is_running());
    EXPECT_TRUE(std::filesystem::exists(directory / "host-state"));
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, RestoresStateBeforeBecomingActive)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-loop-restore";
    const auto path = directory / "host-state";
    std::filesystem::remove_all(directory);
    softadastra::NativePlatform source_platform;
    softadastra::Host source(source_platform);
    ASSERT_TRUE(source.state().add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("example"),
        softadastra::ProcessSpec("/usr/bin/example"))));
    ASSERT_TRUE(softadastra::HostStateFile(path).save(source.state()));

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(path);
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });

    EXPECT_TRUE(wait_until([&loop]()
                           { return loop.is_running(); }));
    EXPECT_EQ(host.state().software_count(), 1U);
    const auto *entry = host.state().find_software(
        softadastra::SoftwareId("example"));
    EXPECT_NE(entry, nullptr);

    if (entry != nullptr)
    {
      EXPECT_EQ(entry->state(), softadastra::SoftwareState::Stopped);
    }

    loop.request_stop();
    thread.join();
    EXPECT_TRUE(completed);
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, PerformsMultipleSupervisionCycles)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-loop-refresh";
    std::filesystem::remove_all(directory);
    std::atomic_size_t refresh_count{0};
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    CountingProcessLauncher launcher(refresh_count);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("example");
    ASSERT_TRUE(service.register_software(
        id,
        softadastra::ProcessSpec("example")));
    ASSERT_TRUE(service.start_software(id));
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });

    EXPECT_TRUE(wait_until([&refresh_count]()
                           { return refresh_count >= 3U; }));
    loop.request_stop();
    thread.join();

    EXPECT_TRUE(completed);
    EXPECT_GE(refresh_count, 3U);
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, RestoresOnlyDesiredSoftwareAndContinuesAfterFailure)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-loop-desired-restore";
    const auto path = directory / "host-state";
    std::filesystem::remove_all(directory);

    softadastra::NativePlatform source_platform;
    softadastra::Host source(source_platform);
    ASSERT_TRUE(source.state().add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("restore"),
        softadastra::ProcessSpec("restore"))));
    ASSERT_TRUE(source.state().add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("stopped"),
        softadastra::ProcessSpec("stopped"))));
    ASSERT_TRUE(source.state().add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("failing"),
        softadastra::ProcessSpec("failing"))));
    source.state().find_software(softadastra::SoftwareId("restore"))
        ->set_desired_running(true);
    source.state().find_software(softadastra::SoftwareId("failing"))
        ->set_desired_running(true);
    ASSERT_TRUE(softadastra::HostStateFile(path).save(source.state()));

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    RestorationProcessLauncher launcher;
    softadastra::HostService service(host, launcher);
    softadastra::HostStateFile state_file(path);
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });

    EXPECT_TRUE(wait_until([&loop]()
                           { return loop.is_running(); }));
    EXPECT_EQ(launcher.launched().size(), 2U);
    EXPECT_EQ(service.software_state(softadastra::SoftwareId("restore")),
              softadastra::SoftwareState::Running);
    EXPECT_EQ(service.software_state(softadastra::SoftwareId("stopped")),
              softadastra::SoftwareState::Stopped);
    EXPECT_EQ(service.software_state(softadastra::SoftwareId("failing")),
              softadastra::SoftwareState::Failed);

    loop.request_stop();
    thread.join();
    EXPECT_TRUE(completed);
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, StopsManagedSoftwareAndPersistsRegistrations)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-loop-shutdown";
    const auto path = directory / "host-state";
    std::filesystem::remove_all(directory);
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    ShutdownProcessLauncher launcher(true);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("example");
    ASSERT_TRUE(service.register_software(
        id,
        softadastra::ProcessSpec("example")));
    ASSERT_TRUE(service.start_software(id));
    softadastra::HostStateFile state_file(path);
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });

    EXPECT_TRUE(wait_until([&loop]()
                           { return loop.is_running(); }));
    loop.request_stop();
    thread.join();

    ASSERT_EQ(launcher.states().size(), 1U);
    EXPECT_EQ(launcher.states().front()->stop_count, 1U);
    EXPECT_FALSE(launcher.states().front()->running);
    EXPECT_TRUE(completed);
    const auto state = service.software_state(id);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state.value(), softadastra::SoftwareState::Stopped);
    EXPECT_TRUE(host.state().find_software(id)->desired_running());
    softadastra::HostState restored;
    EXPECT_TRUE(softadastra::HostStateFile(path).load(restored));
    ASSERT_NE(restored.find_software(id), nullptr);
    EXPECT_TRUE(restored.find_software(id)->desired_running());
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, StopsEveryManagedSoftwareProcess)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-loop-multiple-shutdown";
    std::filesystem::remove_all(directory);
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    ShutdownProcessLauncher launcher(true);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId first("first");
    const softadastra::SoftwareId second("second");
    ASSERT_TRUE(service.register_software(
        first,
        softadastra::ProcessSpec("first")));
    ASSERT_TRUE(service.register_software(
        second,
        softadastra::ProcessSpec("second")));
    ASSERT_TRUE(service.start_software(first));
    ASSERT_TRUE(service.start_software(second));
    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });

    EXPECT_TRUE(wait_until([&loop]()
                           { return loop.is_running(); }));
    loop.request_stop();
    thread.join();

    ASSERT_EQ(launcher.states().size(), 2U);
    EXPECT_EQ(launcher.states().at(0)->stop_count, 1U);
    EXPECT_EQ(launcher.states().at(1)->stop_count, 1U);
    EXPECT_FALSE(launcher.states().at(0)->running);
    EXPECT_FALSE(launcher.states().at(1)->running);
    EXPECT_TRUE(completed);
    EXPECT_EQ(host.state().software_count(), 2U);
    std::filesystem::remove_all(directory);
  }

  TEST(HostLoopTest, PreservesFailedStopDiagnosticAndRegistrations)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-loop-failed-shutdown";
    const auto path = directory / "host-state";
    std::filesystem::remove_all(directory);
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    ShutdownProcessLauncher launcher(false);
    softadastra::HostService service(host, launcher);
    const softadastra::SoftwareId id("example");
    ASSERT_TRUE(service.register_software(
        id,
        softadastra::ProcessSpec("example")));
    ASSERT_TRUE(service.start_software(id));
    softadastra::HostStateFile state_file(path);
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{true};
    std::thread thread([&loop, &completed]()
                       { completed = loop.run(); });

    EXPECT_TRUE(wait_until([&loop]()
                           { return loop.is_running(); }));
    loop.request_stop();
    thread.join();

    ASSERT_EQ(launcher.states().size(), 1U);
    EXPECT_EQ(launcher.states().front()->stop_count, 1U);
    EXPECT_TRUE(launcher.states().front()->running);
    EXPECT_FALSE(completed);
    const auto state = service.software_state(id);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state.value(), softadastra::SoftwareState::Failed);
    const auto result = service.software_result(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->error(), softadastra::SoftwareOperationError::StopFailed);
    softadastra::HostState restored;
    EXPECT_TRUE(softadastra::HostStateFile(path).load(restored));
    EXPECT_NE(restored.find_software(id), nullptr);
    std::filesystem::remove_all(directory);
  }
} // namespace
