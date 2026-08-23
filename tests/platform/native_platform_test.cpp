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

#if defined(_WIN32)

#include <windows.h>

#else

#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

namespace
{

  TEST(NativeProcessTest, PreservesProcessIdentifier)
  {
#if defined(_WIN32)
    const auto id =
        static_cast<softadastra::NativeProcess::Id>(
            GetCurrentProcessId());
#else
    const auto id =
        static_cast<softadastra::NativeProcess::Id>(
            ::getpid());
#endif

    const softadastra::NativeProcess process(id);

    EXPECT_EQ(process.id(), id);
  }

  TEST(NativeProcessTest, ReportsCurrentProcessAsRunning)
  {
#if defined(_WIN32)
    const auto id =
        static_cast<softadastra::NativeProcess::Id>(
            GetCurrentProcessId());
#else
    const auto id =
        static_cast<softadastra::NativeProcess::Id>(
            ::getpid());
#endif

    const softadastra::NativeProcess process(id);

    EXPECT_TRUE(process.is_running());
  }

  TEST(NativeProcessTest, RejectsZeroProcessIdentifier)
  {
    const softadastra::NativeProcess process(0);

    EXPECT_FALSE(process.is_running());
  }

  TEST(NativeProcessTest, SupportsUseThroughProcessInterface)
  {
#if defined(_WIN32)
    const auto id =
        static_cast<softadastra::NativeProcess::Id>(
            GetCurrentProcessId());
#else
    const auto id =
        static_cast<softadastra::NativeProcess::Id>(
            ::getpid());
#endif

    const softadastra::NativeProcess native_process(id);
    const softadastra::Process &process = native_process;

    EXPECT_TRUE(process.is_running());
  }

#if !defined(_WIN32)

  TEST(NativeProcessTest, StopsRunningChildProcess)
  {
    const pid_t child = ::fork();

    ASSERT_NE(child, -1);

    if (child == 0)
    {
      for (;;)
      {
        ::pause();
      }
    }

    softadastra::NativeProcess process(
        static_cast<softadastra::NativeProcess::Id>(child));

    ASSERT_TRUE(process.is_running());
    EXPECT_TRUE(process.stop());

    int status = 0;

    ASSERT_EQ(
        ::waitpid(child, &status, 0),
        child);

    EXPECT_TRUE(WIFSIGNALED(status));
    EXPECT_EQ(WTERMSIG(status), SIGTERM);
    EXPECT_FALSE(process.is_running());
  }

#endif

} // namespace
