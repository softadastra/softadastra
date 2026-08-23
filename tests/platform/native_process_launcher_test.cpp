/**
 *
 *  @file native_process_launcher_test.cpp
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

#include "platform/NativeProcess.hpp"
#include "platform/NativeProcessLauncher.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <thread>

#if defined(__linux__)

#include <unistd.h>

#endif

namespace
{

  std::optional<int> wait_for_exit_code(
      softadastra::Process &process)
  {
    for (int attempt = 0; attempt < 100; ++attempt)
    {
      const auto code = process.exit_code();

      if (code.has_value())
      {
        return code;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(10));
    }

    return std::nullopt;
  }

  TEST(NativeProcessLauncherTest, RejectsEmptyExecutable)
  {
    softadastra::NativeProcessLauncher launcher;

    auto process = launcher.launch(
        softadastra::ProcessSpec(""));

    EXPECT_EQ(process, nullptr);
  }

  TEST(NativeProcessLauncherTest, RejectsUnknownExecutable)
  {
    softadastra::NativeProcessLauncher launcher;

    auto process = launcher.launch(
        softadastra::ProcessSpec(
            "softadastra-executable-that-does-not-exist"));

    EXPECT_EQ(process, nullptr);
    EXPECT_EQ(
        process.error(),
        softadastra::ProcessLaunchError::ExecutableNotFound);
  }

  TEST(NativeProcessLauncherTest, LaunchesRunningProcess)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());
    EXPECT_FALSE(process->exit_code().has_value());

    EXPECT_TRUE(process->stop());
  }

  TEST(NativeProcessLauncherTest, IsolatesManagedProcessFromTerminalGroup)
  {
#if defined(__linux__)
    softadastra::NativeProcessLauncher launcher;
    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    const auto *native_process = dynamic_cast<softadastra::NativeProcess *>(
        &*process);
    ASSERT_NE(native_process, nullptr);
    EXPECT_EQ(
        ::getpgid(native_process->id()),
        native_process->id());
    EXPECT_TRUE(process->stop());
#endif
  }

  TEST(NativeProcessLauncherTest, ReportsSuccessfulNaturalExit)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "exit 0",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "exit 0",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);

    const auto code = wait_for_exit_code(*process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 0);
    EXPECT_FALSE(process->is_running());
  }

  TEST(NativeProcessLauncherTest, ReportsFailedNaturalExit)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "exit 7",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "exit 7",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);

    const auto code = wait_for_exit_code(*process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 7);
    EXPECT_FALSE(process->is_running());
  }

  TEST(NativeProcessLauncherTest, PreservesArguments)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());

    EXPECT_TRUE(process->stop());
  }

  TEST(NativeProcessLauncherTest, SupportsProcessLauncherInterface)
  {
    softadastra::NativeProcessLauncher native_launcher;
    softadastra::ProcessLauncher &launcher = native_launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());

    EXPECT_TRUE(process->stop());
  }

} // namespace
