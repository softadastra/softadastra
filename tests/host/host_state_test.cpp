/**
 *
 *  @file host_state_test.cpp
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

#include "host/HostState.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareEntry.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <gtest/gtest.h>

TEST(HostStateTest, StartsEmpty)
{
  const softadastra::HostState state;

  EXPECT_TRUE(state.empty());
  EXPECT_EQ(state.software_count(), 0U);
}

TEST(HostStateTest, AddsSoftwareEntry)
{
  softadastra::HostState state;

  const bool added = state.add_software(
      softadastra::SoftwareEntry(
          softadastra::SoftwareId("inventory"),
          softadastra::ProcessSpec("/usr/bin/inventory")));

  EXPECT_TRUE(added);
  EXPECT_FALSE(state.empty());
  EXPECT_EQ(state.software_count(), 1U);
}

TEST(HostStateTest, RejectsDuplicateSoftwareIdentifier)
{
  softadastra::HostState state;

  ASSERT_TRUE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"),
              softadastra::ProcessSpec("/usr/bin/inventory"))));

  EXPECT_FALSE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"),
              softadastra::ProcessSpec("/usr/bin/other"))));

  EXPECT_EQ(state.software_count(), 1U);
}

TEST(HostStateTest, FindsSoftwareByIdentifier)
{
  softadastra::HostState state;

  ASSERT_TRUE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"),
              softadastra::ProcessSpec("/usr/bin/inventory"))));

  const auto *entry = state.find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);
  EXPECT_EQ(entry->id().value(), "inventory");
  EXPECT_EQ(entry->state(), softadastra::SoftwareState::Stopped);
}

TEST(HostStateTest, PreservesProcessSpecification)
{
  softadastra::HostState state;

  ASSERT_TRUE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"),
              softadastra::ProcessSpec(
                  "/usr/bin/inventory",
                  {"--port", "8080"}))));

  const auto *entry = state.find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);

  EXPECT_EQ(
      entry->process_spec().executable(),
      "/usr/bin/inventory");

  ASSERT_EQ(entry->process_spec().arguments().size(), 2U);
  EXPECT_EQ(entry->process_spec().arguments()[0], "--port");
  EXPECT_EQ(entry->process_spec().arguments()[1], "8080");
}

TEST(HostStateTest, ReturnsNullForUnknownSoftware)
{
  softadastra::HostState state;

  EXPECT_EQ(
      state.find_software(
          softadastra::SoftwareId("unknown")),
      nullptr);
}

TEST(HostStateTest, AllowsInfrastructureStateUpdate)
{
  softadastra::HostState state;

  ASSERT_TRUE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"),
              softadastra::ProcessSpec("/usr/bin/inventory"))));

  auto *entry = state.find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);

  entry->set_state(softadastra::SoftwareState::Running);

  EXPECT_EQ(
      entry->state(),
      softadastra::SoftwareState::Running);
}

TEST(HostStateTest, SupportsConstLookup)
{
  softadastra::HostState state;

  ASSERT_TRUE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"),
              softadastra::ProcessSpec("/usr/bin/inventory"))));

  const softadastra::HostState &const_state = state;

  const auto *entry = const_state.find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);
  EXPECT_EQ(entry->id().value(), "inventory");
  EXPECT_EQ(
      entry->process_spec().executable(),
      "/usr/bin/inventory");
}
