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
#include <filesystem>
#include <fstream>

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

  TEST(ProcessSpecTest, NormalizesExistingRelativeExecutable)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-process-spec";
    std::filesystem::create_directories(directory);
    const auto executable = directory / "program";
    std::ofstream(executable) << "";
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(directory);

    const auto normalized = softadastra::ProcessSpec::normalize_executable("./program");

    std::filesystem::current_path(previous);
    ASSERT_TRUE(normalized.has_value());
    EXPECT_EQ(normalized.value(), "./program");
    std::filesystem::remove_all(directory);
  }

  TEST(ProcessSpecTest, PreservesAbsoluteAndPathExecutables)
  {
    EXPECT_EQ(softadastra::ProcessSpec::normalize_executable("/usr/bin/python3"), "/usr/bin/python3");
    EXPECT_EQ(softadastra::ProcessSpec::normalize_executable("python3"), "python3");
    EXPECT_FALSE(softadastra::ProcessSpec::normalize_executable("./missing-program").has_value());
  }

} // namespace
