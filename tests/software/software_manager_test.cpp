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

#include <memory>
#include <string>

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
        const softadastra::ProcessSpec &spec) override
    {
      last_executable_ = spec.executable();
      ++launch_count_;

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

    [[nodiscard]] int launch_count() const noexcept
    {
      return launch_count_;
    }

    [[nodiscard]] const std::string &last_executable() const noexcept
    {
      return last_executable_;
    }

  private:
    bool launch_fails_{false};
    bool process_running_{true};
    bool stop_succeeds_{true};
    int launch_count_{0};
    std::string last_executable_;
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

    EXPECT_FALSE(manager.start(id));
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

    EXPECT_FALSE(manager.start(id));

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

    EXPECT_FALSE(manager.stop(id));

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

    EXPECT_FALSE(
        manager.start(
            softadastra::SoftwareId("unknown")));

    EXPECT_EQ(launcher.launch_count(), 0);
  }

  TEST(SoftwareManagerTest, RejectsStopForUnknownSoftware)
  {
    softadastra::HostState host_state;
    TestProcessLauncher launcher;

    softadastra::SoftwareManager manager(
        host_state,
        launcher);

    EXPECT_FALSE(
        manager.stop(
            softadastra::SoftwareId("unknown")));
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

    EXPECT_FALSE(manager.stop(id));

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

} // namespace
