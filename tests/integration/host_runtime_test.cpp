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
#include <vector>

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

  class RunningSoftwareCleanup
  {
  public:
    explicit RunningSoftwareCleanup(
        softadastra::ControlClient &client) noexcept
        : client_(client)
    {
    }

    ~RunningSoftwareCleanup()
    {
      for (const auto &id : ids_)
      {
        static_cast<void>(client_.stop_software(id));
      }
    }

    void track(const softadastra::SoftwareId &id)
    {
      ids_.push_back(id);
    }

  private:
    softadastra::ControlClient &client_;
    std::vector<softadastra::SoftwareId> ids_;
  };

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

    const auto result = client.software_result(id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(
        result->error(),
        softadastra::SoftwareOperationError::ProcessExitedSuccessfully);
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

    const auto result = client.software_result(id);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(
        result->error(),
        softadastra::SoftwareOperationError::ProcessExitedWithNonZeroCode);
    EXPECT_EQ(result->exit_code(), 7);
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

    const auto result = client.start_software(id);

    EXPECT_FALSE(result);

    EXPECT_EQ(
        result.error(),
        softadastra::SoftwareOperationError::ExecutableNotFound);

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

  TEST(HostRuntimeTest, StopsOnlyTheRequestedSoftwareProcess)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);
    RunningSoftwareCleanup cleanup(client);

    const softadastra::SoftwareId first("first-running");
    const softadastra::SoftwareId second("second-stopped");
    const softadastra::SoftwareId third("third-running");

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "ping.exe",
        {
            "-n",
            "30",
            "127.0.0.1",
        });

#else

    const softadastra::ProcessSpec spec(
        "sleep",
        {
            "30",
        });

#endif

    ASSERT_TRUE(client.register_software(first, spec));
    ASSERT_TRUE(client.register_software(second, spec));
    ASSERT_TRUE(client.register_software(third, spec));

    ASSERT_TRUE(client.start_software(first));
    cleanup.track(first);
    ASSERT_TRUE(client.start_software(second));
    cleanup.track(second);
    ASSERT_TRUE(client.start_software(third));
    cleanup.track(third);

    EXPECT_TRUE(client.stop_software(second));

    client.refresh();

    EXPECT_EQ(
        client.software_state(first).value(),
        softadastra::SoftwareState::Running);
    EXPECT_EQ(
        client.software_state(second).value(),
        softadastra::SoftwareState::Stopped);
    EXPECT_EQ(
        client.software_state(third).value(),
        softadastra::SoftwareState::Running);
  }

  TEST(HostRuntimeTest, RefreshesIndependentNaturalProcessExits)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);
    RunningSoftwareCleanup cleanup(client);

    const softadastra::SoftwareId successful("successful-exit");
    const softadastra::SoftwareId running("still-running");
    const softadastra::SoftwareId failed("failed-exit");

#if defined(_WIN32)

    const softadastra::ProcessSpec successful_spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 2 127.0.0.1 >NUL & exit 0",
        });
    const softadastra::ProcessSpec running_spec(
        "ping.exe",
        {
            "-n",
            "30",
            "127.0.0.1",
        });
    const softadastra::ProcessSpec failed_spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 2 127.0.0.1 >NUL & exit 7",
        });

#else

    const softadastra::ProcessSpec successful_spec(
        "sh",
        {
            "-c",
            "sleep 0.1; exit 0",
        });
    const softadastra::ProcessSpec running_spec(
        "sleep",
        {
            "30",
        });
    const softadastra::ProcessSpec failed_spec(
        "sh",
        {
            "-c",
            "sleep 0.1; exit 7",
        });

#endif

    ASSERT_TRUE(client.register_software(successful, successful_spec));
    ASSERT_TRUE(client.register_software(running, running_spec));
    ASSERT_TRUE(client.register_software(failed, failed_spec));

    ASSERT_TRUE(client.start_software(successful));
    cleanup.track(successful);
    ASSERT_TRUE(client.start_software(running));
    cleanup.track(running);
    ASSERT_TRUE(client.start_software(failed));
    cleanup.track(failed);

    refresh_until_settled(client, successful);
    refresh_until_settled(client, failed);

    ASSERT_TRUE(client.software_state(successful).has_value());
    ASSERT_TRUE(client.software_state(running).has_value());
    ASSERT_TRUE(client.software_state(failed).has_value());

    EXPECT_EQ(
        client.software_state(successful).value(),
        softadastra::SoftwareState::Stopped);
    EXPECT_EQ(
        client.software_state(running).value(),
        softadastra::SoftwareState::Running);
    EXPECT_EQ(
        client.software_state(failed).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(HostRuntimeTest, RestartsRealRunningAndStoppedSoftware)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);
    RunningSoftwareCleanup cleanup(client);

    const softadastra::SoftwareId id("restartable");

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "ping.exe",
        {
            "-n",
            "30",
            "127.0.0.1",
        });

#else

    const softadastra::ProcessSpec spec(
        "sleep",
        {
            "30",
        });

#endif

    ASSERT_TRUE(client.register_software(id, spec));
    ASSERT_TRUE(client.start_software(id));
    cleanup.track(id);

    ASSERT_TRUE(client.restart_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Running);

    ASSERT_TRUE(client.stop_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);

    EXPECT_TRUE(client.restart_software(id));

    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Running);
  }

} // namespace
