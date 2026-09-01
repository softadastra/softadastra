/**
 *
 *  @file host_observation_test.cpp
 */

#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostObservation.hpp"
#include "host/HostService.hpp"
#include "platform/HostInstanceLock.hpp"
#include "platform/NativePlatform.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <string>

namespace
{
  std::filesystem::path temporary_directory()
  {
    return std::filesystem::temp_directory_path() /
           ("softadastra-host-observation-" +
            std::to_string(
                std::chrono::steady_clock::now()
                    .time_since_epoch()
                    .count()));
  }

  TEST(HostObservationTest, ReportsRunningFromTheValidatedControlServer)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);

    EXPECT_EQ(
        softadastra::observe_host(client, temporary_directory()).state,
        softadastra::HostAvailability::Running);
  }

  TEST(HostObservationTest, DistinguishesStoppedUnavailableAndUnknown)
  {
    const auto directory = temporary_directory();
    std::filesystem::create_directories(directory);
    softadastra::ControlClient client(directory / "control.sock");

    EXPECT_EQ(
        softadastra::observe_host(client, directory).state,
        softadastra::HostAvailability::Stopped);

    {
      softadastra::HostInstanceLock lock;
      ASSERT_TRUE(lock.acquire(directory));

      EXPECT_EQ(
          softadastra::observe_host(client, directory).state,
          softadastra::HostAvailability::Unavailable);
    }

    std::filesystem::remove(directory / "host.lock");
    std::filesystem::create_directory(directory / "host.lock");

    EXPECT_EQ(
        softadastra::observe_host(client, directory).state,
        softadastra::HostAvailability::Unknown);

    std::filesystem::remove_all(directory);
  }

  TEST(HostObservationTest, UsesCanonicalPublicNames)
  {
    EXPECT_STREQ(
        softadastra::host_availability_name(
            softadastra::HostAvailability::Running),
        "running");
    EXPECT_STREQ(
        softadastra::host_availability_name(
            softadastra::HostAvailability::Stopped),
        "stopped");
    EXPECT_STREQ(
        softadastra::host_availability_name(
            softadastra::HostAvailability::Unavailable),
        "unavailable");
    EXPECT_STREQ(
        softadastra::host_availability_name(
            softadastra::HostAvailability::Unknown),
        "unknown");
  }

  TEST(HostObservationTest, AllowsUserHostOnlyWhenBoxIsStopped)
  {
    EXPECT_TRUE(softadastra::box_allows_user_host_start(
        {softadastra::HostAvailability::Stopped}));
    EXPECT_FALSE(softadastra::box_allows_user_host_start(
        {softadastra::HostAvailability::Running}));
    EXPECT_FALSE(softadastra::box_allows_user_host_start(
        {softadastra::HostAvailability::Unavailable}));
    EXPECT_FALSE(softadastra::box_allows_user_host_start(
        {softadastra::HostAvailability::Unknown}));
  }
} // namespace
