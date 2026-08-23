/**
 *
 *  @file service_test.cpp
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

#include "platform/Service.hpp"
#include <gtest/gtest.h>

namespace
{
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

} // namespace

TEST(ServiceTest, StartsStopped)
{
  TestService service;

  EXPECT_FALSE(service.is_running());
}

TEST(ServiceTest, CanStart)
{
  TestService service;

  EXPECT_TRUE(service.start());
  EXPECT_TRUE(service.is_running());
}

TEST(ServiceTest, CanStop)
{
  TestService service;

  ASSERT_TRUE(service.start());
  ASSERT_TRUE(service.is_running());

  EXPECT_TRUE(service.stop());
  EXPECT_FALSE(service.is_running());
}

TEST(ServiceTest, SupportsUseThroughServiceInterface)
{
  TestService concrete;
  softadastra::Service &service = concrete;

  EXPECT_TRUE(service.start());
  EXPECT_TRUE(service.is_running());

  EXPECT_TRUE(service.stop());
  EXPECT_FALSE(service.is_running());
}
