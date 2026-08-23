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

#include <memory>

#if !defined(_WIN32)

#include <sys/types.h>
#include <sys/wait.h>

#endif

namespace
{
  TEST(NativeProcessLauncherTest, RejectsEmptyExecutable)
  {
    softadastra::NativeProcessLauncher launcher;

    const softadastra::ProcessSpec spec("");

    auto process = launcher.launch(spec);

    EXPECT_EQ(process, nullptr);
  }

  TEST(NativeProcessLauncherTest, RejectsUnknownExecutable)
  {
    softadastra::NativeProcessLauncher launcher;

    const softadastra::ProcessSpec spec(
        "softadastra-executable-that-does-not-exist");

    auto process = launcher.launch(spec);

    EXPECT_EQ(process, nullptr);
  }

  TEST(NativeProcessLauncherTest, LaunchesRealProcess)
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

#if !defined(_WIN32)

    const auto *native_process =
        dynamic_cast<const softadastra::NativeProcess *>(process.get());

    ASSERT_NE(native_process, nullptr);

    int status = 0;

    EXPECT_EQ(
        ::waitpid(
            static_cast<pid_t>(native_process->id()),
            &status,
            0),
        static_cast<pid_t>(native_process->id()));

#endif
  }

  TEST(NativeProcessLauncherTest, PreservesProcessArguments)
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

#if !defined(_WIN32)

    const auto *native_process =
        dynamic_cast<const softadastra::NativeProcess *>(process.get());

    ASSERT_NE(native_process, nullptr);

    int status = 0;

    EXPECT_EQ(
        ::waitpid(
            static_cast<pid_t>(native_process->id()),
            &status,
            0),
        static_cast<pid_t>(native_process->id()));

#endif
  }

  TEST(NativeProcessLauncherTest, SupportsUseThroughProcessLauncherInterface)
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

#if !defined(_WIN32)

    const auto *native_process =
        dynamic_cast<const softadastra::NativeProcess *>(process.get());

    ASSERT_NE(native_process, nullptr);

    int status = 0;

    EXPECT_EQ(
        ::waitpid(
            static_cast<pid_t>(native_process->id()),
            &status,
            0),
        static_cast<pid_t>(native_process->id()));

#endif
  }

} // namespace
