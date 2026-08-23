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
    explicit TestProcess(bool running = true) noexcept
        : running_(running)
    {
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
    bool running_;
  };

  TEST(ProcessTest, RepresentsRunningProcess)
  {
    const TestProcess process;

    EXPECT_TRUE(process.is_running());
  }

  TEST(ProcessTest, CanRepresentStoppedProcess)
  {
    const TestProcess process(false);

    EXPECT_FALSE(process.is_running());
  }

  TEST(ProcessTest, CanStopProcess)
  {
    TestProcess process;

    EXPECT_TRUE(process.stop());
    EXPECT_FALSE(process.is_running());
  }

  TEST(ProcessTest, SupportsUseThroughProcessInterface)
  {
    TestProcess concrete_process;
    softadastra::Process &process = concrete_process;

    EXPECT_TRUE(process.is_running());
    EXPECT_TRUE(process.stop());
    EXPECT_FALSE(process.is_running());
  }

} // namespace
