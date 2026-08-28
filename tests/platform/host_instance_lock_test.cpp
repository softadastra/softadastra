/**
 *
 *  @file host_instance_lock_test.cpp
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

#include "platform/HostInstanceLock.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <string>

#if defined(__linux__)

#include <sys/wait.h>
#include <unistd.h>

#endif

namespace
{
  TEST(HostInstanceLockTest, RejectsSecondHostAndReleasesAfterShutdown)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-host-lock-" + std::to_string(
                                                           std::chrono::steady_clock::now()
                                                               .time_since_epoch()
                                                               .count()));
    std::filesystem::create_directories(directory);

    {
      softadastra::HostInstanceLock first;
      ASSERT_TRUE(first.acquire(directory));

#if defined(__linux__)
      const pid_t child = ::fork();
      ASSERT_GE(child, 0);

      if (child == 0)
      {
        softadastra::HostInstanceLock second;
        ::_exit(second.acquire(directory) ? 1 : 0);
      }

      int status = 0;
      ASSERT_EQ(::waitpid(child, &status, 0), child);
      ASSERT_TRUE(WIFEXITED(status));
      EXPECT_EQ(WEXITSTATUS(status), 0);
#endif
    }

    softadastra::HostInstanceLock restarted;
    EXPECT_TRUE(restarted.acquire(directory));
    std::filesystem::remove_all(directory);
  }
} // namespace
