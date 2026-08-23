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
#include "host/HostLoop.hpp"
#include "host/HostService.hpp"
#include "host/HostStateFile.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <thread>
#include <vector>

#if defined(__linux__)

#include <cerrno>
#include <csignal>
#include <fstream>

#endif

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

  template <typename Predicate>
  bool wait_until(Predicate predicate)
  {
    for (int attempt = 0; attempt < 100; ++attempt)
    {
      if (predicate())
      {
        return true;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return predicate();
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

  class HostServiceCleanup
  {
  public:
    explicit HostServiceCleanup(softadastra::HostService &service) noexcept
        : service_(service)
    {
    }

    ~HostServiceCleanup()
    {
      static_cast<void>(service_.shutdown());
    }

  private:
    softadastra::HostService &service_;
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

  TEST(HostRuntimeTest, RepeatsRealLifecycleOperationsAndRefreshes)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer server(host_service);
    softadastra::ControlClient client(server);
    RunningSoftwareCleanup cleanup(client);

    const softadastra::SoftwareId id("repeated-lifecycle");

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

    for (int attempt = 0; attempt < 3; ++attempt)
    {
      client.refresh();

      EXPECT_EQ(
          client.software_state(id).value(),
          softadastra::SoftwareState::Running);
    }

    ASSERT_TRUE(client.stop_software(id));
    ASSERT_TRUE(client.start_software(id));
    ASSERT_TRUE(client.restart_software(id));

    for (int attempt = 0; attempt < 3; ++attempt)
    {
      client.refresh();

      EXPECT_EQ(
          client.software_state(id).value(),
          softadastra::SoftwareState::Running);
    }

    EXPECT_TRUE(client.stop_software(id));
    EXPECT_EQ(
        client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(HostRuntimeTest, StartsRestoredRealSoftware)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-host-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "state";
    const softadastra::SoftwareId first("first");
    const softadastra::SoftwareId second("second");

    {
      softadastra::NativePlatform platform;
      softadastra::Host host(platform);
      softadastra::HostService service(host, platform.process_launcher());

      ASSERT_TRUE(service.register_software(
          first,
          softadastra::ProcessSpec("sleep", {"30"})));
      ASSERT_TRUE(service.register_software(
          second,
          softadastra::ProcessSpec("sleep", {"30"})));
      ASSERT_TRUE(softadastra::HostStateFile(path).save(host.state()));
    }

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    ASSERT_TRUE(softadastra::HostStateFile(path).load(host.state()));
    softadastra::HostService service(host, platform.process_launcher());
    RunningSoftwareCleanup cleanup(*new softadastra::ControlClient(
        *new softadastra::ControlServer(service)));

    EXPECT_EQ(service.software_state(first).value(), softadastra::SoftwareState::Stopped);
    EXPECT_EQ(service.software_state(second).value(), softadastra::SoftwareState::Stopped);
    ASSERT_TRUE(service.start_software(first));
    EXPECT_EQ(service.software_state(first).value(), softadastra::SoftwareState::Running);
    EXPECT_TRUE(service.stop_software(first));
    std::filesystem::remove_all(directory);
  }

  TEST(HostRuntimeTest, RestoresHostAfterSimulatedMachineReboot)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-reboot-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "host-state";
    const softadastra::SoftwareId running("running-before-reboot");
    const softadastra::SoftwareId stopped("stopped-before-reboot");

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

    {
      softadastra::NativePlatform platform;
      softadastra::Host host(platform);
      softadastra::HostService service(host, platform.process_launcher());
      softadastra::HostStateFile state_file(path);

      ASSERT_TRUE(service.register_software(running, spec));
      ASSERT_TRUE(service.register_software(stopped, spec));
      ASSERT_TRUE(service.start_software(running));

      softadastra::HostLoop loop(
          service,
          state_file,
          std::chrono::seconds(1));
      std::atomic_bool completed{false};
      std::thread thread([&loop, &completed]()
                         {
                           completed = loop.run();
                         });

      EXPECT_TRUE(wait_until([&loop]()
                             {
                               return loop.is_running();
                             }));
      loop.request_stop();
      thread.join();
      EXPECT_TRUE(completed);
    }

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::HostStateFile state_file(path);
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::seconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       {
                         completed = loop.run();
                       });

    EXPECT_TRUE(wait_until([&loop]()
                           {
                             return loop.is_running();
                           }));
    EXPECT_EQ(host.state().software_count(), 2U);
    const auto running_state = service.software_state(running);
    const auto stopped_state = service.software_state(stopped);
    EXPECT_TRUE(running_state.has_value());
    EXPECT_TRUE(stopped_state.has_value());

    if (running_state.has_value())
    {
      EXPECT_EQ(running_state.value(), softadastra::SoftwareState::Stopped);
    }

    if (stopped_state.has_value())
    {
      EXPECT_EQ(stopped_state.value(), softadastra::SoftwareState::Stopped);
    }

    EXPECT_TRUE(service.start_software(running));
    const auto restarted_state = service.software_state(running);
    EXPECT_TRUE(restarted_state.has_value());

    if (restarted_state.has_value())
    {
      EXPECT_EQ(restarted_state.value(), softadastra::SoftwareState::Running);
    }

    loop.request_stop();
    thread.join();
    EXPECT_TRUE(completed);
    const auto shutdown_state = service.software_state(running);
    EXPECT_TRUE(shutdown_state.has_value());

    if (shutdown_state.has_value())
    {
      EXPECT_EQ(shutdown_state.value(), softadastra::SoftwareState::Stopped);
    }

    softadastra::HostState restored;
    EXPECT_TRUE(state_file.load(restored));
    EXPECT_NE(restored.find_software(running), nullptr);
    EXPECT_NE(restored.find_software(stopped), nullptr);
    std::filesystem::remove_all(directory);
  }

  TEST(HostRuntimeTest, SupervisesRealSoftwareWithoutManualRefresh)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-supervision-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const softadastra::SoftwareId long_running("long-running");
    const softadastra::SoftwareId successful("successful-exit");
    const softadastra::SoftwareId failed("failed-exit");
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    HostServiceCleanup cleanup(service);

#if defined(_WIN32)

    const softadastra::ProcessSpec long_running_spec(
        "ping.exe",
        {
            "-n",
            "30",
            "127.0.0.1",
        });
    const softadastra::ProcessSpec successful_spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 2 127.0.0.1 >NUL & exit 0",
        });
    const softadastra::ProcessSpec failed_spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 2 127.0.0.1 >NUL & exit 7",
        });

#else

    const softadastra::ProcessSpec long_running_spec(
        "sleep",
        {
            "30",
        });
    const softadastra::ProcessSpec successful_spec(
        "sh",
        {
            "-c",
            "sleep 0.05; exit 0",
        });
    const softadastra::ProcessSpec failed_spec(
        "sh",
        {
            "-c",
            "sleep 0.05; exit 7",
        });

#endif

    ASSERT_TRUE(service.register_software(long_running, long_running_spec));
    ASSERT_TRUE(service.register_software(successful, successful_spec));
    ASSERT_TRUE(service.register_software(failed, failed_spec));
    ASSERT_TRUE(service.start_software(long_running));
    ASSERT_TRUE(service.start_software(successful));
    ASSERT_TRUE(service.start_software(failed));

    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       {
                         completed = loop.run();
                       });

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    loop.request_stop();
    thread.join();

    EXPECT_TRUE(completed);
    const auto long_running_state = service.software_state(long_running);
    const auto successful_state = service.software_state(successful);
    const auto failed_state = service.software_state(failed);
    ASSERT_TRUE(long_running_state.has_value());
    ASSERT_TRUE(successful_state.has_value());
    ASSERT_TRUE(failed_state.has_value());
    EXPECT_EQ(long_running_state.value(), softadastra::SoftwareState::Stopped);
    EXPECT_EQ(successful_state.value(), softadastra::SoftwareState::Stopped);
    EXPECT_EQ(failed_state.value(), softadastra::SoftwareState::Failed);

    const auto successful_result = service.software_result(successful);
    const auto failed_result = service.software_result(failed);
    ASSERT_TRUE(successful_result.has_value());
    ASSERT_TRUE(failed_result.has_value());
    EXPECT_EQ(
        successful_result->error(),
        softadastra::SoftwareOperationError::ProcessExitedSuccessfully);
    EXPECT_EQ(
        failed_result->error(),
        softadastra::SoftwareOperationError::ProcessExitedWithNonZeroCode);
    EXPECT_EQ(failed_result->exit_code(), 7);

    for (int attempt = 0; attempt < 3; ++attempt)
    {
      service.refresh();
    }

    EXPECT_EQ(
        service.software_state(successful).value(),
        softadastra::SoftwareState::Stopped);
    EXPECT_EQ(
        service.software_state(failed).value(),
        softadastra::SoftwareState::Failed);
    std::filesystem::remove_all(directory);
  }

#if defined(__linux__)
  TEST(HostRuntimeTest, DetectsExternallyTerminatedManagedProcess)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-disappeared-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto pid_file = directory / "process-id";
    const softadastra::SoftwareId id("externally-terminated");
    std::filesystem::create_directories(directory);
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    HostServiceCleanup cleanup(service);
    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "echo $$ > " + pid_file.string() + "; exec sleep 30",
        });

    ASSERT_TRUE(service.register_software(id, spec));
    ASSERT_TRUE(service.start_software(id));

    softadastra::HostStateFile state_file(directory / "host-state");
    softadastra::HostLoop loop(
        service,
        state_file,
        std::chrono::milliseconds(1));
    std::atomic_bool completed{false};
    std::thread thread([&loop, &completed]()
                       {
                         completed = loop.run();
                       });

    const bool pid_available = wait_until([&pid_file]()
                                          {
                                            return std::filesystem::exists(pid_file);
                                          });
    EXPECT_TRUE(pid_available);
    int process_id = 0;

    if (pid_available)
    {
      std::ifstream input(pid_file);
      input >> process_id;
      EXPECT_GT(process_id, 0);

      if (process_id > 0)
      {
        EXPECT_EQ(::kill(process_id, SIGTERM), 0);
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    loop.request_stop();
    thread.join();

    EXPECT_TRUE(completed);

    if (process_id > 0)
    {
      errno = 0;
      EXPECT_EQ(::kill(process_id, 0), -1);
      EXPECT_EQ(errno, ESRCH);
    }

    const auto state = service.software_state(id);
    ASSERT_TRUE(state.has_value());
    EXPECT_NE(state.value(), softadastra::SoftwareState::Running);

    if (state.value() == softadastra::SoftwareState::Failed)
    {
      const auto result = service.software_result(id);
      ASSERT_TRUE(result.has_value());
      EXPECT_EQ(
          result->error(),
          softadastra::SoftwareOperationError::ProcessExitedWithNonZeroCode);
    }

    std::filesystem::remove_all(directory);
  }
#endif

} // namespace
