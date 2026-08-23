/**
 *
 *  @file host_service_test.cpp
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

#include "host/HostService.hpp"
#include <gtest/gtest.h>

namespace
{
  class TestProcess final : public softadastra::Process
  {
  public:
    bool start() override
    {
      ++start_calls_;

      if (!start_result_)
      {
        return false;
      }

      running_ = true;
      return true;
    }

    bool stop() override
    {
      ++stop_calls_;

      if (!stop_result_)
      {
        return false;
      }

      running_ = false;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

    void set_start_result(bool result) noexcept
    {
      start_result_ = result;
    }

    void set_stop_result(bool result) noexcept
    {
      stop_result_ = result;
    }

    [[nodiscard]] int start_calls() const noexcept
    {
      return start_calls_;
    }

    [[nodiscard]] int stop_calls() const noexcept
    {
      return stop_calls_;
    }

  private:
    bool running_{false};
    bool start_result_{true};
    bool stop_result_{true};

    int start_calls_{0};
    int stop_calls_{0};
  };

  class TestService final : public softadastra::Service
  {
  public:
    bool start() override
    {
      running_ = true;
      return true;
    }

    bool stop() override
    {
      running_ = false;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

  private:
    bool running_{false};
  };

  class TestNetwork final : public softadastra::Network
  {
  public:
    TestNetwork(
        bool available,
        bool connected)
        : available_(available),
          connected_(connected)
    {
    }

    [[nodiscard]] bool is_available() const noexcept override
    {
      return available_;
    }

    [[nodiscard]] bool is_connected() const noexcept override
    {
      return connected_;
    }

  private:
    bool available_;
    bool connected_;
  };

  class TestPlatform final : public softadastra::Platform
  {
  public:
    TestPlatform(
        bool network_available = true,
        bool network_connected = true)
        : network_(network_available, network_connected)
    {
    }

    [[nodiscard]] softadastra::Process &process() noexcept override
    {
      return process_;
    }

    [[nodiscard]] const softadastra::Process &process() const noexcept override
    {
      return process_;
    }

    [[nodiscard]] softadastra::Service &service() noexcept override
    {
      return service_;
    }

    [[nodiscard]] const softadastra::Service &service() const noexcept override
    {
      return service_;
    }

    [[nodiscard]] softadastra::Network &network() noexcept override
    {
      return network_;
    }

    [[nodiscard]] const softadastra::Network &network() const noexcept override
    {
      return network_;
    }

  private:
    TestProcess process_;
    TestService service_;
    TestNetwork network_;
  };

} // namespace

TEST(HostServiceTest, CoordinatesHost)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  softadastra::HostService service(host);

  EXPECT_EQ(&service.host(), &host);
}

TEST(HostServiceTest, RegistersSoftwareInHostState)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  softadastra::HostService service(host);

  EXPECT_TRUE(
      service.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_EQ(host.state().software_count(), 1U);

  const auto *entry = host.state().find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);
  EXPECT_EQ(entry->id().value(), "inventory");
}

TEST(HostServiceTest, RejectsDuplicateSoftwareRegistration)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  softadastra::HostService service(host);

  ASSERT_TRUE(
      service.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_FALSE(
      service.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_EQ(host.state().software_count(), 1U);
}

TEST(HostServiceTest, StartsRegisteredSoftware)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  softadastra::HostService service(host);
  TestProcess process;

  ASSERT_TRUE(
      service.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_TRUE(
      service.start_software(
          softadastra::SoftwareId("inventory"),
          process));

  EXPECT_TRUE(process.is_running());
  EXPECT_EQ(process.start_calls(), 1);

  const auto state = service.software_state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(state.has_value());

  EXPECT_EQ(
      *state,
      softadastra::SoftwareState::Running);
}

TEST(HostServiceTest, StopsRegisteredSoftware)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  softadastra::HostService service(host);
  TestProcess process;

  ASSERT_TRUE(
      service.register_software(
          softadastra::SoftwareId("inventory")));

  ASSERT_TRUE(
      service.start_software(
          softadastra::SoftwareId("inventory"),
          process));

  EXPECT_TRUE(
      service.stop_software(
          softadastra::SoftwareId("inventory"),
          process));

  EXPECT_FALSE(process.is_running());
  EXPECT_EQ(process.stop_calls(), 1);

  const auto state = service.software_state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(state.has_value());

  EXPECT_EQ(
      *state,
      softadastra::SoftwareState::Stopped);
}

TEST(HostServiceTest, PropagatesSoftwareStartFailure)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  softadastra::HostService service(host);
  TestProcess process;

  process.set_start_result(false);

  ASSERT_TRUE(
      service.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_FALSE(
      service.start_software(
          softadastra::SoftwareId("inventory"),
          process));

  const auto state = service.software_state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(state.has_value());

  EXPECT_EQ(
      *state,
      softadastra::SoftwareState::Failed);
}

TEST(HostServiceTest, PropagatesSoftwareStopFailure)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  softadastra::HostService service(host);
  TestProcess process;

  ASSERT_TRUE(
      service.register_software(
          softadastra::SoftwareId("inventory")));

  ASSERT_TRUE(
      service.start_software(
          softadastra::SoftwareId("inventory"),
          process));

  process.set_stop_result(false);

  EXPECT_FALSE(
      service.stop_software(
          softadastra::SoftwareId("inventory"),
          process));

  const auto state = service.software_state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(state.has_value());

  EXPECT_EQ(
      *state,
      softadastra::SoftwareState::Failed);
}

TEST(HostServiceTest, ReportsAvailableConnectivity)
{
  TestPlatform platform(true, false);
  softadastra::Host host(platform);
  const softadastra::HostService service(host);

  EXPECT_TRUE(service.connectivity_available());
  EXPECT_FALSE(service.connected());
}

TEST(HostServiceTest, ReportsConnectedHost)
{
  TestPlatform platform(true, true);
  softadastra::Host host(platform);
  const softadastra::HostService service(host);

  EXPECT_TRUE(service.connectivity_available());
  EXPECT_TRUE(service.connected());
}

TEST(HostServiceTest, NeverReportsConnectedWhenNetworkIsUnavailable)
{
  TestPlatform platform(false, true);
  softadastra::Host host(platform);
  const softadastra::HostService service(host);

  EXPECT_FALSE(service.connectivity_available());
  EXPECT_FALSE(service.connected());
}

TEST(HostServiceTest, ReturnsNoStateForUnknownSoftware)
{
  TestPlatform platform;
  softadastra::Host host(platform);
  const softadastra::HostService service(host);

  EXPECT_FALSE(
      service.software_state(
                 softadastra::SoftwareId("unknown"))
          .has_value());
}
