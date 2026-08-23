/**
 *
 *  @file process_test.cpp
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

#include "platform/Process.hpp"
#include <gtest/gtest.h>

namespace
{
  class TestProcess final : public softadastra::Process
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

} // namespace

TEST(ProcessTest, StartsStopped)
{
  TestProcess process;

  EXPECT_FALSE(process.is_running());
}

TEST(ProcessTest, CanStart)
{
  TestProcess process;

  EXPECT_TRUE(process.start());
  EXPECT_TRUE(process.is_running());
}

TEST(ProcessTest, CanStop)
{
  TestProcess process;

  ASSERT_TRUE(process.start());
  ASSERT_TRUE(process.is_running());

  EXPECT_TRUE(process.stop());
  EXPECT_FALSE(process.is_running());
}

TEST(ProcessTest, SupportsUseThroughProcessInterface)
{
  TestProcess concrete;
  softadastra::Process &process = concrete;

  EXPECT_TRUE(process.start());
  EXPECT_TRUE(process.is_running());

  EXPECT_TRUE(process.stop());
  EXPECT_FALSE(process.is_running());
}
