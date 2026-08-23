/**
 *
 *  @file process_spec_test.cpp
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

#include "platform/ProcessSpec.hpp"
#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{
  TEST(ProcessSpecTest, PreservesExecutable)
  {
    const softadastra::ProcessSpec spec("/usr/bin/example");

    EXPECT_EQ(spec.executable(), "/usr/bin/example");
  }

  TEST(ProcessSpecTest, StartsWithNoArguments)
  {
    const softadastra::ProcessSpec spec("/usr/bin/example");

    EXPECT_TRUE(spec.arguments().empty());
  }

  TEST(ProcessSpecTest, PreservesArguments)
  {
    const softadastra::ProcessSpec spec(
        "/usr/bin/example",
        {"--port", "8080", "--verbose"});

    const std::vector<std::string> expected{
        "--port",
        "8080",
        "--verbose"};

    EXPECT_EQ(spec.arguments(), expected);
  }

} // namespace
