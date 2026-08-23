/**
 *
 *  @file native_platform_test.cpp
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

#include "platform/NativeNetwork.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/NativeProcessLauncher.hpp"
#include "platform/NativeService.hpp"
#include "platform/Platform.hpp"
#include "platform/ProcessSpec.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <thread>
#include <type_traits>

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

  TEST(NativePlatformTest, ImplementsPlatformContract)
  {
    EXPECT_TRUE((
        std::is_base_of_v<
            softadastra::Platform,
            softadastra::NativePlatform>));
  }

  TEST(NativePlatformTest, ExposesNativeProcessLauncher)
  {
    softadastra::NativePlatform platform;

    auto *launcher =
        dynamic_cast<softadastra::NativeProcessLauncher *>(
            &platform.process_launcher());

    EXPECT_NE(launcher, nullptr);
  }

  TEST(NativePlatformTest, ExposesNativeService)
  {
    softadastra::NativePlatform platform;

    auto *service =
        dynamic_cast<softadastra::NativeService *>(
            &platform.service());

    EXPECT_NE(service, nullptr);
  }

  TEST(NativePlatformTest, ExposesNativeNetwork)
  {
    softadastra::NativePlatform platform;

    auto *network =
        dynamic_cast<softadastra::NativeNetwork *>(
            &platform.network());

    EXPECT_NE(network, nullptr);
  }

  TEST(NativePlatformTest, SupportsConstCapabilityAccess)
  {
    const softadastra::NativePlatform platform;

    const auto *launcher =
        dynamic_cast<const softadastra::NativeProcessLauncher *>(
            &platform.process_launcher());

    const auto *service =
        dynamic_cast<const softadastra::NativeService *>(
            &platform.service());

    const auto *network =
        dynamic_cast<const softadastra::NativeNetwork *>(
            &platform.network());

    EXPECT_NE(launcher, nullptr);
    EXPECT_NE(service, nullptr);
    EXPECT_NE(network, nullptr);
  }

  TEST(NativePlatformTest, RejectsInvalidProcess)
  {
    softadastra::NativePlatform platform;

    auto process =
        platform.process_launcher().launch(
            softadastra::ProcessSpec(""));

    EXPECT_EQ(process, nullptr);
  }

  TEST(NativePlatformTest, LaunchesRealProcess)
  {
    softadastra::NativePlatform platform;

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

    auto process =
        platform.process_launcher().launch(spec);

    ASSERT_NE(process, nullptr);

    EXPECT_TRUE(process->is_running());
    EXPECT_FALSE(process->exit_code().has_value());

    EXPECT_TRUE(process->stop());
    EXPECT_FALSE(process->is_running());
  }

  TEST(NativePlatformTest, DetectsSuccessfulProcessExit)
  {
    softadastra::NativePlatform platform;

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

    auto process =
        platform.process_launcher().launch(spec);

    ASSERT_NE(process, nullptr);

    const auto code =
        wait_for_exit_code(*process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 0);
    EXPECT_FALSE(process->is_running());
  }

  TEST(NativePlatformTest, DetectsFailedProcessExit)
  {
    softadastra::NativePlatform platform;

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

    auto process =
        platform.process_launcher().launch(spec);

    ASSERT_NE(process, nullptr);

    const auto code =
        wait_for_exit_code(*process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 7);
    EXPECT_FALSE(process->is_running());
  }

  TEST(NativePlatformTest, CanInspectNetworkThroughPlatformInterface)
  {
    const softadastra::NativePlatform native_platform;
    const softadastra::Platform &platform = native_platform;

    if (platform.network().is_connected())
    {
      EXPECT_TRUE(platform.network().is_available());
    }
  }

} // namespace
