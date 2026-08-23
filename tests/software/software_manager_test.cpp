/**
 *
 *  @file software_manager_test.cpp
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

#include "software/SoftwareManager.hpp"
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

} // namespace

TEST(SoftwareManagerTest, RegistersSoftware)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);

  EXPECT_TRUE(
      manager.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_EQ(state.software_count(), 1U);

  const auto software_state = manager.state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(software_state.has_value());

  EXPECT_EQ(
      *software_state,
      softadastra::SoftwareState::Stopped);
}

TEST(SoftwareManagerTest, RejectsDuplicateSoftwareRegistration)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);

  ASSERT_TRUE(
      manager.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_FALSE(
      manager.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_EQ(state.software_count(), 1U);
}

TEST(SoftwareManagerTest, StartsRegisteredSoftware)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);
  TestProcess process;

  ASSERT_TRUE(
      manager.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_TRUE(
      manager.start(
          softadastra::SoftwareId("inventory"),
          process));

  EXPECT_TRUE(process.is_running());
  EXPECT_EQ(process.start_calls(), 1);

  const auto software_state = manager.state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(software_state.has_value());

  EXPECT_EQ(
      *software_state,
      softadastra::SoftwareState::Running);
}

TEST(SoftwareManagerTest, MarksSoftwareFailedWhenStartFails)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);
  TestProcess process;

  process.set_start_result(false);

  ASSERT_TRUE(
      manager.register_software(
          softadastra::SoftwareId("inventory")));

  EXPECT_FALSE(
      manager.start(
          softadastra::SoftwareId("inventory"),
          process));

  EXPECT_FALSE(process.is_running());

  const auto software_state = manager.state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(software_state.has_value());

  EXPECT_EQ(
      *software_state,
      softadastra::SoftwareState::Failed);
}

TEST(SoftwareManagerTest, StopsRunningSoftware)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);
  TestProcess process;

  ASSERT_TRUE(
      manager.register_software(
          softadastra::SoftwareId("inventory")));

  ASSERT_TRUE(
      manager.start(
          softadastra::SoftwareId("inventory"),
          process));

  ASSERT_TRUE(process.is_running());

  EXPECT_TRUE(
      manager.stop(
          softadastra::SoftwareId("inventory"),
          process));

  EXPECT_FALSE(process.is_running());
  EXPECT_EQ(process.stop_calls(), 1);

  const auto software_state = manager.state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(software_state.has_value());

  EXPECT_EQ(
      *software_state,
      softadastra::SoftwareState::Stopped);
}

TEST(SoftwareManagerTest, MarksSoftwareFailedWhenStopFails)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);
  TestProcess process;

  ASSERT_TRUE(
      manager.register_software(
          softadastra::SoftwareId("inventory")));

  ASSERT_TRUE(
      manager.start(
          softadastra::SoftwareId("inventory"),
          process));

  process.set_stop_result(false);

  EXPECT_FALSE(
      manager.stop(
          softadastra::SoftwareId("inventory"),
          process));

  EXPECT_TRUE(process.is_running());

  const auto software_state = manager.state(
      softadastra::SoftwareId("inventory"));

  ASSERT_TRUE(software_state.has_value());

  EXPECT_EQ(
      *software_state,
      softadastra::SoftwareState::Failed);
}

TEST(SoftwareManagerTest, RejectsStartForUnknownSoftware)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);
  TestProcess process;

  EXPECT_FALSE(
      manager.start(
          softadastra::SoftwareId("unknown"),
          process));

  EXPECT_EQ(process.start_calls(), 0);
}

TEST(SoftwareManagerTest, RejectsStopForUnknownSoftware)
{
  softadastra::HostState state;
  softadastra::SoftwareManager manager(state);
  TestProcess process;

  EXPECT_FALSE(
      manager.stop(
          softadastra::SoftwareId("unknown"),
          process));

  EXPECT_EQ(process.stop_calls(), 0);
}

TEST(SoftwareManagerTest, ReturnsNoStateForUnknownSoftware)
{
  softadastra::HostState state;
  const softadastra::SoftwareManager manager(state);

  EXPECT_FALSE(
      manager.state(
                 softadastra::SoftwareId("unknown"))
          .has_value());
}
