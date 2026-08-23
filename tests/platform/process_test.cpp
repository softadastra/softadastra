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

#include <optional>

namespace
{

  class TestProcess final : public softadastra::Process
  {
  public:
    explicit TestProcess(bool running = true) noexcept
        : running_(running)
    {
      if (!running_)
      {
        exit_code_ = 0;
      }
    }

    bool stop() override
    {
      running_ = false;
      exit_code_ = 0;
      return true;
    }

    [[nodiscard]] bool is_running() const noexcept override
    {
      return running_;
    }

    [[nodiscard]] std::optional<int> exit_code() noexcept override
    {
      return exit_code_;
    }

    void finish(int code) noexcept
    {
      running_ = false;
      exit_code_ = code;
    }

  private:
    bool running_;
    std::optional<int> exit_code_;
  };

  TEST(ProcessTest, RepresentsRunningProcess)
  {
    TestProcess process;

    EXPECT_TRUE(process.is_running());
    EXPECT_FALSE(process.exit_code().has_value());
  }

  TEST(ProcessTest, ReportsSuccessfulExit)
  {
    TestProcess process;

    process.finish(0);

    EXPECT_FALSE(process.is_running());

    ASSERT_TRUE(process.exit_code().has_value());
    EXPECT_EQ(process.exit_code().value(), 0);
  }

  TEST(ProcessTest, ReportsFailedExit)
  {
    TestProcess process;

    process.finish(7);

    EXPECT_FALSE(process.is_running());

    ASSERT_TRUE(process.exit_code().has_value());
    EXPECT_EQ(process.exit_code().value(), 7);
  }

  TEST(ProcessTest, CanStopProcess)
  {
    TestProcess process;

    EXPECT_TRUE(process.stop());
    EXPECT_FALSE(process.is_running());

    ASSERT_TRUE(process.exit_code().has_value());
    EXPECT_EQ(process.exit_code().value(), 0);
  }

  TEST(ProcessTest, SupportsUseThroughProcessInterface)
  {
    TestProcess concrete_process;
    softadastra::Process &process = concrete_process;

    EXPECT_TRUE(process.is_running());
    EXPECT_FALSE(process.exit_code().has_value());

    concrete_process.finish(3);

    EXPECT_FALSE(process.is_running());

    ASSERT_TRUE(process.exit_code().has_value());
    EXPECT_EQ(process.exit_code().value(), 3);
  }

} // namespace
