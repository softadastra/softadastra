/**
 *
 *  @file software_state_test.cpp
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

#include "software/SoftwareState.hpp"
#include <gtest/gtest.h>

TEST(SoftwareStateTest, DefinesStoppedState)
{
  const auto state = softadastra::SoftwareState::Stopped;

  EXPECT_EQ(state, softadastra::SoftwareState::Stopped);
}

TEST(SoftwareStateTest, DefinesStartingState)
{
  const auto state = softadastra::SoftwareState::Starting;

  EXPECT_EQ(state, softadastra::SoftwareState::Starting);
}

TEST(SoftwareStateTest, DefinesRunningState)
{
  const auto state = softadastra::SoftwareState::Running;

  EXPECT_EQ(state, softadastra::SoftwareState::Running);
}

TEST(SoftwareStateTest, DefinesFailedState)
{
  const auto state = softadastra::SoftwareState::Failed;

  EXPECT_EQ(state, softadastra::SoftwareState::Failed);
}

TEST(SoftwareStateTest, KeepsLifecycleStatesDistinct)
{
  EXPECT_NE(
      softadastra::SoftwareState::Stopped,
      softadastra::SoftwareState::Starting);

  EXPECT_NE(
      softadastra::SoftwareState::Starting,
      softadastra::SoftwareState::Running);

  EXPECT_NE(
      softadastra::SoftwareState::Running,
      softadastra::SoftwareState::Failed);
}

TEST(SoftwareStateTest, UsesCanonicalNamesAndRejectsInvalidProtocolValues)
{
  EXPECT_STREQ(softadastra::software_state_name(softadastra::SoftwareState::Stopped), "stopped");
  EXPECT_STREQ(softadastra::software_state_name(softadastra::SoftwareState::Starting), "starting");
  EXPECT_STREQ(softadastra::software_state_name(softadastra::SoftwareState::Running), "running");
  EXPECT_STREQ(softadastra::software_state_name(softadastra::SoftwareState::Failed), "failed");
  EXPECT_FALSE(softadastra::software_state_from_value(99).has_value());
  EXPECT_FALSE(softadastra::software_state_from_value(-1).has_value());
}
