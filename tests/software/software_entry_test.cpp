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

#include <string>
#include <vector>

namespace
{

  TEST(SoftwareEntryTest, PreservesSoftwareIdentifier)
  {
    const softadastra::SoftwareEntry entry(
        softadastra::SoftwareId("example"),
        softadastra::ProcessSpec("/usr/bin/example"));

    EXPECT_EQ(entry.id().value(), "example");
  }

  TEST(SoftwareEntryTest, PreservesProcessSpecification)
  {
    const softadastra::SoftwareEntry entry(
        softadastra::SoftwareId("example"),
        softadastra::ProcessSpec(
            "/usr/bin/example",
            {"--port", "8080"}));

    EXPECT_EQ(entry.process_spec().executable(), "/usr/bin/example");

    const std::vector<std::string> expected{"--port", "8080"};
    EXPECT_EQ(entry.process_spec().arguments(), expected);
  }

  TEST(SoftwareEntryTest, StartsStopped)
  {
    const softadastra::SoftwareEntry entry(
        softadastra::SoftwareId("example"),
        softadastra::ProcessSpec("/usr/bin/example"));

    EXPECT_EQ(entry.state(), softadastra::SoftwareState::Stopped);
  }

  TEST(SoftwareEntryTest, UpdatesLifecycleState)
  {
    softadastra::SoftwareEntry entry(
        softadastra::SoftwareId("example"),
        softadastra::ProcessSpec("/usr/bin/example"));

    entry.set_state(softadastra::SoftwareState::Running);

    EXPECT_EQ(entry.state(), softadastra::SoftwareState::Running);
  }

} // namespace
