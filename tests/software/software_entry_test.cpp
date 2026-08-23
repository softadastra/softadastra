/**
 *
 *  @file software_entry_test.cpp
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

#include "software/SoftwareEntry.hpp"
#include <gtest/gtest.h>

TEST(SoftwareEntryTest, PreservesSoftwareIdentifier)
{
  const softadastra::SoftwareEntry entry(
      softadastra::SoftwareId("inventory"));

  EXPECT_EQ(entry.id().value(), "inventory");
}

TEST(SoftwareEntryTest, StartsStopped)
{
  const softadastra::SoftwareEntry entry(
      softadastra::SoftwareId("inventory"));

  EXPECT_EQ(entry.state(), softadastra::SoftwareState::Stopped);
}

TEST(SoftwareEntryTest, UpdatesLifecycleState)
{
  softadastra::SoftwareEntry entry(
      softadastra::SoftwareId("inventory"));

  entry.set_state(softadastra::SoftwareState::Starting);
  EXPECT_EQ(entry.state(), softadastra::SoftwareState::Starting);

  entry.set_state(softadastra::SoftwareState::Running);
  EXPECT_EQ(entry.state(), softadastra::SoftwareState::Running);

  entry.set_state(softadastra::SoftwareState::Failed);
  EXPECT_EQ(entry.state(), softadastra::SoftwareState::Failed);

  entry.set_state(softadastra::SoftwareState::Stopped);
  EXPECT_EQ(entry.state(), softadastra::SoftwareState::Stopped);
}
