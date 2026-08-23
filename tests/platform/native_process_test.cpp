/**
 *
 *  @file native_process_test.cpp
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
#include "platform/Process.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <thread>
#include <vix/process/Command.hpp>
#include <vix/process/Spawn.hpp>

namespace
{

  vix::process::Child spawn_long_running_process()
  {
#if defined(_WIN32)

    vix::process::Command command("cmd.exe");
    command.args(
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    vix::process::Command command("sh");
    command.args(
        {
            "-c",
            "sleep 30",
        });

#endif

    command.search_in_path(true);

    auto result = vix::process::spawn(std::move(command));

    EXPECT_TRUE(result);

    if (!result)
    {
      return {};
    }

    return result.value();
  }

  vix::process::Child spawn_exiting_process(int exit_code)
  {
#if defined(_WIN32)

    vix::process::Command command("cmd.exe");
    command.args(
        {
            "/C",
            "exit " + std::to_string(exit_code),
        });

#else

    vix::process::Command command("sh");
    command.args(
        {
            "-c",
            "exit " + std::to_string(exit_code),
        });

#endif

    command.search_in_path(true);

    auto result = vix::process::spawn(std::move(command));

    EXPECT_TRUE(result);

    if (!result)
    {
      return {};
    }

    return result.value();
  }

  std::optional<int> wait_for_exit_code(
      softadastra::NativeProcess &process)
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

  TEST(NativeProcessTest, PreservesProcessIdentifier)
  {
    auto child = spawn_long_running_process();

    ASSERT_TRUE(child.valid());

    softadastra::NativeProcess process(child);

    EXPECT_EQ(process.id(), child.id());

    EXPECT_TRUE(process.stop());
  }

  TEST(NativeProcessTest, ReportsRunningProcess)
  {
    auto child = spawn_long_running_process();

    ASSERT_TRUE(child.valid());

    softadastra::NativeProcess process(child);

    EXPECT_TRUE(process.is_running());
    EXPECT_FALSE(process.exit_code().has_value());

    EXPECT_TRUE(process.stop());
  }

  TEST(NativeProcessTest, ReportsSuccessfulExit)
  {
    auto child = spawn_exiting_process(0);

    ASSERT_TRUE(child.valid());

    softadastra::NativeProcess process(child);

    const auto code = wait_for_exit_code(process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 0);
    EXPECT_FALSE(process.is_running());
  }

  TEST(NativeProcessTest, ReportsFailedExit)
  {
    auto child = spawn_exiting_process(7);

    ASSERT_TRUE(child.valid());

    softadastra::NativeProcess process(child);

    const auto code = wait_for_exit_code(process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 7);
    EXPECT_FALSE(process.is_running());
  }

  TEST(NativeProcessTest, StopsRunningProcess)
  {
    auto child = spawn_long_running_process();

    ASSERT_TRUE(child.valid());

    softadastra::NativeProcess process(child);

    ASSERT_TRUE(process.is_running());

    EXPECT_TRUE(process.stop());
    EXPECT_FALSE(process.is_running());
    EXPECT_TRUE(process.exit_code().has_value());
  }

  TEST(NativeProcessTest, RejectsInvalidChild)
  {
    softadastra::NativeProcess process(
        vix::process::Child{});

    EXPECT_EQ(process.id(), 0U);
    EXPECT_FALSE(process.is_running());
    EXPECT_FALSE(process.exit_code().has_value());
    EXPECT_FALSE(process.stop());
  }

  TEST(NativeProcessTest, SupportsUseThroughProcessInterface)
  {
    auto child = spawn_long_running_process();

    ASSERT_TRUE(child.valid());

    softadastra::NativeProcess native_process(child);
    softadastra::Process &process = native_process;

    EXPECT_TRUE(process.is_running());
    EXPECT_FALSE(process.exit_code().has_value());

    EXPECT_TRUE(process.stop());
    EXPECT_FALSE(process.is_running());
  }

} // namespace
