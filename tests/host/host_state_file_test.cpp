/**
 *
 *  @file host_state_file_test.cpp
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

#include "host/HostStateFile.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>

namespace
{
  TEST(HostStateFileTest, SavesRegistrationMetadata)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-state-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "host-state";

    softadastra::HostState state;
    ASSERT_TRUE(state.add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("example"),
        softadastra::ProcessSpec("/usr/bin/example", {"--port", "8080"}))));

    EXPECT_TRUE(softadastra::HostStateFile(path).save(state));
    EXPECT_TRUE(std::filesystem::exists(path));

    std::ifstream input(path);
    const std::string content(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

    EXPECT_NE(content.find("example"), std::string::npos);
    EXPECT_NE(content.find("/usr/bin/example"), std::string::npos);
    std::filesystem::remove_all(directory);
  }

  TEST(HostStateFileTest, PersistsOnlyRegistrationMetadata)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-state-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "host-state";
    const auto application_directory = directory / "application";
    const auto database = application_directory / "application.db";
    const auto output = application_directory / "stdout.log";
    std::filesystem::create_directories(application_directory);

    std::ofstream(database) << "business-data-must-not-be-persisted";
    std::ofstream(output) << "process-output-must-not-be-persisted";

    softadastra::HostState state;
    ASSERT_TRUE(state.add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("example"),
        softadastra::ProcessSpec("/usr/bin/example", {"--port", "8080"}))));
    ASSERT_TRUE(softadastra::HostStateFile(path).save(state));

    std::ifstream input(path);
    const std::string content(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

    EXPECT_EQ(content,
        "softadastra-registrations 5\n1\n7\nexample\n0\n16\n/usr/bin/example\n"
        "0\n0\n2\n6\n--port\n4\n8080\n0\n\n");
    EXPECT_EQ(content.find(database.string()), std::string::npos);
    EXPECT_EQ(content.find("business-data-must-not-be-persisted"), std::string::npos);
    EXPECT_EQ(content.find(output.string()), std::string::npos);
    EXPECT_EQ(content.find("process-output-must-not-be-persisted"), std::string::npos);
    std::filesystem::remove_all(directory);
  }

  TEST(HostStateFileTest, RestoresStoppedRegistrations)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-state-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "host-state";
    softadastra::HostState source;

    ASSERT_TRUE(source.add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("first"),
        softadastra::ProcessSpec("first", {"one"}))));
    ASSERT_TRUE(source.add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("second"),
        softadastra::ProcessSpec("second", {"two", "words"}))));
    ASSERT_TRUE(softadastra::HostStateFile(path).save(source));

    softadastra::HostState restored;

    ASSERT_TRUE(softadastra::HostStateFile(path).load(restored));
    ASSERT_EQ(restored.software_count(), 2U);

    const auto *first = restored.find_software(softadastra::SoftwareId("first"));
    const auto *second = restored.find_software(softadastra::SoftwareId("second"));

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(first->state(), softadastra::SoftwareState::Stopped);
    EXPECT_EQ(second->state(), softadastra::SoftwareState::Stopped);
    EXPECT_EQ(second->process_spec().arguments().at(1), "words");
    std::filesystem::remove_all(directory);
  }

  TEST(HostStateFileTest, RejectsCorruptStateWithoutChangingHostState)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-state-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "host-state";
    std::filesystem::create_directories(directory);
    std::ofstream output(path);
    output << "softadastra-registrations 1\n2\n1\na\n1\nx\n0\n1\na\n1\ny\n0\n";
    output.close();

    softadastra::HostState state;
    const softadastra::HostStateFile file(path);

    EXPECT_FALSE(file.load(state));
    EXPECT_EQ(state.software_count(), 0U);
    EXPECT_EQ(file.last_load_error(), softadastra::HostStateLoadError::InvalidContent);
    std::filesystem::remove_all(directory);
  }

  TEST(HostStateFileTest, RejectsMalformedStateFiles)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-state-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "host-state";
    const std::vector<std::string> invalid_content{
        "",
        "softadastra-registrations 1\n1\n1\na\n",
        "softadastra-registrations 1\n1\n1\na\n0\n",
        "softadastra-registrations 1\n1\n0\n\n1\na\n0\n"};

    std::filesystem::create_directories(directory);

    for (const auto &content : invalid_content)
    {
      std::ofstream output(path, std::ios::trunc);
      output << content;
      output.close();

      softadastra::HostState state;
      const softadastra::HostStateFile file(path);

      EXPECT_FALSE(file.load(state));
      EXPECT_TRUE(state.empty());
      EXPECT_EQ(file.last_load_error(), softadastra::HostStateLoadError::InvalidContent);
    }

    std::filesystem::remove_all(directory);
  }

  TEST(HostStateFileTest, KeepsExistingStateWhenFileIsInvalid)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-state-" + std::to_string(
                               std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()));
    const auto path = directory / "host-state";
    std::filesystem::create_directories(directory);
    std::ofstream output(path);
    output << "softadastra-registrations 1\n2\n1\na\n1\nx\n0\n1\na\n1\ny\n0\n";
    output.close();

    softadastra::HostState state;
    ASSERT_TRUE(state.add_software(softadastra::SoftwareEntry(
        softadastra::SoftwareId("existing"),
        softadastra::ProcessSpec("existing"))));
    const softadastra::HostStateFile file(path);

    EXPECT_FALSE(file.load(state));
    ASSERT_EQ(state.software_count(), 1U);
    EXPECT_NE(state.find_software(softadastra::SoftwareId("existing")), nullptr);
    EXPECT_EQ(file.last_load_error(), softadastra::HostStateLoadError::InvalidContent);
    std::filesystem::remove_all(directory);
  }
} // namespace
