/**
 *
 *  @file process_launcher_test.cpp
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

#include "platform/ProcessLauncher.hpp"
#include <gtest/gtest.h>

#include <memory>
#include <string>

namespace
{
  class TestProcess final : public softadastra::Process
  {
  public:
    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

    bool stop() override
    {
      running_ = false;
      return true;
    }

  private:
    bool running_{true};
  };

  class TestProcessLauncher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] std::unique_ptr<softadastra::Process> launch(
        const softadastra::ProcessSpec &spec) override
    {
      last_executable_ = spec.executable();

      if (fail_)
      {
        return nullptr;
      }

      return std::make_unique<TestProcess>();
    }

    void set_fail(bool fail) noexcept
    {
      fail_ = fail;
    }

    [[nodiscard]] const std::string &last_executable() const noexcept
    {
      return last_executable_;
    }

  private:
    bool fail_{false};
    std::string last_executable_;
  };

  TEST(ProcessLauncherTest, LaunchesProcess)
  {
    TestProcessLauncher launcher;
    const softadastra::ProcessSpec spec("/usr/bin/example");

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());
  }

  TEST(ProcessLauncherTest, ReceivesProcessSpecification)
  {
    TestProcessLauncher launcher;
    const softadastra::ProcessSpec spec("/usr/bin/example");

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_EQ(launcher.last_executable(), "/usr/bin/example");
  }

  TEST(ProcessLauncherTest, ReturnsNullWhenLaunchFails)
  {
    TestProcessLauncher launcher;
    launcher.set_fail(true);

    const softadastra::ProcessSpec spec("/usr/bin/example");

    auto process = launcher.launch(spec);

    EXPECT_EQ(process, nullptr);
  }

  TEST(ProcessLauncherTest, SupportsUseThroughProcessLauncherInterface)
  {
    TestProcessLauncher concrete_launcher;
    softadastra::ProcessLauncher &launcher = concrete_launcher;

    const softadastra::ProcessSpec spec("/usr/bin/example");

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());
  }

} // namespace
