/**
 *
 *  @file software_manager_test.cpp
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

#include "host/HostState.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareManager.hpp"
#include "software/SoftwareState.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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
        std::shared_ptr<TestProcessState> state) noexcept
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
      last_executable_ = spec.executable();
      ++launch_count_;

      if (launch_fails_)
      {
        return launch_error_;
      }

      auto process = std::make_shared<TestProcessState>();
      process->running = process_running_;
      process->stop_succeeds = stop_succeeds_;

      processes_.push_back(process);

      return std::make_unique<TestProcess>(std::move(process));
    }

    void set_launch_fails(bool value) noexcept
    {
      launch_fails_ = value;
    }

    void set_launch_error(
        softadastra::ProcessLaunchError error) noexcept
    {
      launch_error_ = error;
    }

    void set_process_running(bool value) noexcept
    {
      process_running_ = value;
    }

    void set_stop_succeeds(bool value) noexcept
    {
      stop_succeeds_ = value;
    }

    [[nodiscard]] int launch_count() const noexcept
    {
      return launch_count_;
    }

    [[nodiscard]] const std::string &last_executable() const noexcept
    {
      return last_executable_;
    }

    [[nodiscard]] std::size_t active_process_count() const noexcept
    {
      return static_cast<std::size_t>(std::count_if(
          processes_.begin(),
          processes_.end(),
          [](const std::shared_ptr<TestProcessState> &process)
          {
            return process->running;
          }));
    }

    [[nodiscard]] std::shared_ptr<TestProcessState> last_process() const noexcept
    {
      return processes_.empty() ? nullptr : processes_.back();
    }

  private:
    bool launch_fails_{false};
    softadastra::ProcessLaunchError launch_error_{
        softadastra::ProcessLaunchError::LaunchFailed};
    bool process_running_{true};
    bool stop_succeeds_{true};
    int launch_count_{0};
    std::string last_executable_;
    std::vector<std::shared_ptr<TestProcessState>> processes_;
  };

  TEST(SoftwareManagerTest, RegistersSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    EXPECT_TRUE(
        manager.register_software(
            softadastra::SoftwareId("example"),
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_EQ(host_state.software_count(), 1U);
  }

  TEST(SoftwareManagerTest, RejectsDuplicateSoftwareRegistration)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    EXPECT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_FALSE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/other")));
  }

  TEST(SoftwareManagerTest, StartsRegisteredSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_TRUE(manager.start(id));
    EXPECT_EQ(launcher.launch_count(), 1);
    EXPECT_EQ(
        launcher.last_executable(),
        "/usr/bin/example");

    ASSERT_TRUE(manager.state(id).has_value());
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(SoftwareManagerTest, RejectsSecondStartWhileSoftwareIsRunning)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(manager.start(id));

    const auto result = manager.start(id);

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::AlreadyRunning);
    EXPECT_EQ(launcher.launch_count(), 1);
  }

  TEST(SoftwareManagerTest, MarksSoftwareFailedWhenLaunchFails)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    launcher.set_launch_fails(true);

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    const auto result = manager.start(id);

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::LaunchFailed);

    ASSERT_TRUE(manager.state(id).has_value());
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(SoftwareManagerTest, RejectsProcessThatIsNotRunningAfterLaunch)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    launcher.set_process_running(false);

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_FALSE(manager.start(id));

    ASSERT_TRUE(manager.state(id).has_value());
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(SoftwareManagerTest, StopsRunningSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(manager.start(id));

    EXPECT_TRUE(manager.stop(id));

    ASSERT_TRUE(manager.state(id).has_value());
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(SoftwareManagerTest, MarksSoftwareFailedWhenStopFails)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    launcher.set_stop_succeeds(false);

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    ASSERT_TRUE(manager.start(id));

    const auto result = manager.stop(id);

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::StopFailed);

    ASSERT_TRUE(manager.state(id).has_value());
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(SoftwareManagerTest, RejectsStartForUnknownSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const auto result = manager.start(
        softadastra::SoftwareId("unknown"));

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::SoftwareUnknown);

    EXPECT_EQ(launcher.launch_count(), 0);
  }

  TEST(SoftwareManagerTest, RejectsStopForUnknownSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const auto result = manager.stop(
        softadastra::SoftwareId("unknown"));

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::SoftwareUnknown);
  }

  TEST(SoftwareManagerTest, RejectsStopWhenSoftwareIsNotRunning)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    const auto result = manager.stop(id);

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::NotRunning);

    ASSERT_TRUE(manager.state(id).has_value());
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(SoftwareManagerTest, ReturnsNoStateForUnknownSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    const softadastra::SoftwareManager manager(
        host_state,
        launcher);

    EXPECT_FALSE(
        manager.state(
                   softadastra::SoftwareId("unknown"))
            .has_value());
  }

  TEST(SoftwareManagerTest, ManagesIndependentSoftwareProcesses)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    const softadastra::SoftwareId first("first");
    const softadastra::SoftwareId second("second");

    ASSERT_TRUE(
        manager.register_software(
            first,
            softadastra::ProcessSpec("/usr/bin/first")));

    ASSERT_TRUE(
        manager.register_software(
            second,
            softadastra::ProcessSpec("/usr/bin/second")));

    EXPECT_TRUE(manager.start(first));
    EXPECT_TRUE(manager.start(second));

    EXPECT_EQ(launcher.launch_count(), 2);

    EXPECT_EQ(
        manager.state(first).value(),
        softadastra::SoftwareState::Running);

    EXPECT_EQ(
        manager.state(second).value(),
        softadastra::SoftwareState::Running);

    EXPECT_TRUE(manager.stop(first));

    EXPECT_EQ(
        manager.state(first).value(),
        softadastra::SoftwareState::Stopped);

    EXPECT_EQ(
        manager.state(second).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(SoftwareManagerTest, RestartsRunningSoftwareWithOneProcess)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));
    ASSERT_TRUE(manager.start(id));

    EXPECT_TRUE(manager.restart(id));
    EXPECT_EQ(launcher.launch_count(), 2);
    EXPECT_EQ(launcher.active_process_count(), 1U);
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(SoftwareManagerTest, RestartsStoppedSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));

    EXPECT_TRUE(manager.restart(id));
    EXPECT_EQ(launcher.launch_count(), 1);
    EXPECT_EQ(launcher.active_process_count(), 1U);
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(SoftwareManagerTest, RestartsFailedSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    launcher.set_launch_fails(true);

    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));
    EXPECT_FALSE(manager.start(id));

    launcher.set_launch_fails(false);

    EXPECT_TRUE(manager.restart(id));
    EXPECT_EQ(launcher.launch_count(), 2);
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(SoftwareManagerTest, DoesNotLaunchReplacementWhenStopFails)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    launcher.set_stop_succeeds(false);

    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));
    ASSERT_TRUE(manager.start(id));

    EXPECT_FALSE(manager.restart(id));
    EXPECT_EQ(launcher.launch_count(), 1);
    EXPECT_EQ(launcher.active_process_count(), 1U);
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(SoftwareManagerTest, MarksSoftwareFailedWhenReplacementLaunchFails)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(
        manager.register_software(
            id,
            softadastra::ProcessSpec("/usr/bin/example")));
    ASSERT_TRUE(manager.start(id));

    launcher.set_launch_fails(true);

    EXPECT_FALSE(manager.restart(id));
    EXPECT_EQ(launcher.launch_count(), 2);
    EXPECT_EQ(launcher.active_process_count(), 0U);
    EXPECT_EQ(
        manager.state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(SoftwareManagerTest, ReportsExecutableNotFound)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    launcher.set_launch_fails(true);
    launcher.set_launch_error(
        softadastra::ProcessLaunchError::ExecutableNotFound);
    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(manager.register_software(
        id,
        softadastra::ProcessSpec("/usr/bin/example")));

    const auto result = manager.start(id);

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::ExecutableNotFound);
  }

  TEST(SoftwareManagerTest, ReportsPermissionDenied)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    launcher.set_launch_fails(true);
    launcher.set_launch_error(
        softadastra::ProcessLaunchError::PermissionDenied);
    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(manager.register_software(
        id,
        softadastra::ProcessSpec("/usr/bin/example")));

    const auto result = manager.start(id);

    EXPECT_FALSE(result);
    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::PermissionDenied);
  }

  TEST(SoftwareManagerTest, ReportsNonZeroProcessExit)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;
    softadastra::SoftwareManager manager(host_state, launcher);

    const softadastra::SoftwareId id("example");

    ASSERT_TRUE(manager.register_software(
        id,
        softadastra::ProcessSpec("/usr/bin/example")));
    ASSERT_TRUE(manager.start(id));

    const auto process = launcher.last_process();

    ASSERT_NE(process, nullptr);
    process->running = false;
    process->exit_code = 7;

    manager.refresh();

    const auto result = manager.result(id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(
        result->error(),
        softadastra::SoftwareOperationError::ProcessExitedWithNonZeroCode);
    EXPECT_EQ(result->exit_code(), 7);
  }

  TEST(SoftwareManagerTest, EnforcesUniqueNamesAndKeepsFailedRenameAtomic)
  {
    softadastra::HostState state;
    TestProcessLauncher launcher;
    softadastra::SoftwareManager manager(state, launcher);
    const softadastra::SoftwareId api("A"), worker("B");
    const auto http8000 = softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8000);
    const auto http9000 = softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 9000);
    ASSERT_TRUE(manager.register_software(api, softadastra::ProcessSpec("command-A", {}, "/project/A"), http8000, std::nullopt, "api"));
    ASSERT_TRUE(manager.register_software(worker, softadastra::ProcessSpec("command-B"), std::nullopt, std::nullopt, "worker"));
    EXPECT_FALSE(manager.register_software(softadastra::SoftwareId("C"), softadastra::ProcessSpec("other"), std::nullopt, std::nullopt, "api"));
    EXPECT_FALSE(manager.synchronize(api, softadastra::ProcessSpec("command-new", {}, "/project/new"), http9000, "worker"));
    const auto entry = manager.find_by_name("api");
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->process_spec().executable(), "command-A");
    EXPECT_EQ(entry->process_spec().working_directory(), "/project/A");
    ASSERT_TRUE(entry->access_point().has_value());
    EXPECT_EQ(entry->access_point()->port(), 8000);
    EXPECT_FALSE(manager.find_by_name("").has_value());
    EXPECT_FALSE(manager.find_by_name("unknown").has_value());
    EXPECT_TRUE(manager.synchronize(api, softadastra::ProcessSpec("command-A", {}, "/project/A"), http8000, "backend"));
    EXPECT_FALSE(manager.find_by_name("api").has_value());
    EXPECT_EQ(manager.find_by_name("backend")->id(), api);
  }

} // namespace
