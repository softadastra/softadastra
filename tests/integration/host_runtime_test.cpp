/**
 *
 *  @file host_runtime_test.cpp
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

#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace
{
  void refresh_until_settled(
      softadastra::ControlClient &client,
      const softadastra::SoftwareId &id)
  {
    for (int attempt = 0; attempt < 100; ++attempt)
    {
      client.refresh();

      const auto state = client.software_state(id);

      if (!state.has_value())
      {
        return;
      }

      if (state.value() != softadastra::SoftwareState::Running &&
          state.value() != softadastra::SoftwareState::Starting)
      {
        return;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(10));
    }
  }

  TEST(HostRuntimeTest, RunsAndStopsRealSoftware)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("long-running");

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

    ASSERT_TRUE(
        client.register_software(id, spec));

    ASSERT_TRUE(client.start_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Running);

    EXPECT_TRUE(client.stop_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(HostRuntimeTest, DetectsSuccessfulNaturalExit)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("successful-exit");

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 2 127.0.0.1 >NUL & exit 0",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 0.1; exit 0",
        });

#endif

    ASSERT_TRUE(
        client.register_software(id, spec));

    ASSERT_TRUE(client.start_software(id));

    refresh_until_settled(client, id);

    ASSERT_TRUE(client.software_state(id).has_value());

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(HostRuntimeTest, DetectsFailedNaturalExit)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("failed-exit");

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 2 127.0.0.1 >NUL & exit 7",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 0.1; exit 7",
        });

#endif

    ASSERT_TRUE(
        client.register_software(id, spec));

    ASSERT_TRUE(client.start_software(id));

    refresh_until_settled(client, id);

    ASSERT_TRUE(client.software_state(id).has_value());

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(HostRuntimeTest, RejectsSoftwareThatCannotBeLaunched)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("missing-software");

    ASSERT_TRUE(
        client.register_software(
            id,
            softadastra::ProcessSpec(
                "softadastra-executable-that-does-not-exist")));

    EXPECT_FALSE(client.start_software(id));

    ASSERT_TRUE(client.software_state(id).has_value());

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(HostRuntimeTest, KeepsLongRunningSoftwareRunningAfterRefresh)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);

    const softadastra::SoftwareId id("still-running");

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

    ASSERT_TRUE(
        client.register_software(id, spec));

    ASSERT_TRUE(client.start_software(id));

    client.refresh();

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Running);

    EXPECT_TRUE(client.stop_software(id));
  }

} // namespace
