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
          softadastra::SoftwareId("inventory")));

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
              softadastra::SoftwareId("inventory"))));

  EXPECT_FALSE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"))));

  EXPECT_EQ(state.software_count(), 1U);
}

TEST(HostStateTest, FindsSoftwareByIdentifier)
{
  softadastra::HostState state;

  ASSERT_TRUE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"))));

  const auto *entry = state.find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);
  EXPECT_EQ(entry->id().value(), "inventory");
  EXPECT_EQ(entry->state(), softadastra::SoftwareState::Stopped);
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
              softadastra::SoftwareId("inventory"))));

  auto *entry = state.find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);

  entry->set_state(softadastra::SoftwareState::Running);

  EXPECT_EQ(entry->state(), softadastra::SoftwareState::Running);
}

TEST(HostStateTest, SupportsConstLookup)
{
  softadastra::HostState state;

  ASSERT_TRUE(
      state.add_software(
          softadastra::SoftwareEntry(
              softadastra::SoftwareId("inventory"))));

  const softadastra::HostState &const_state = state;

  const auto *entry = const_state.find_software(
      softadastra::SoftwareId("inventory"));

  ASSERT_NE(entry, nullptr);
  EXPECT_EQ(entry->id().value(), "inventory");
}
