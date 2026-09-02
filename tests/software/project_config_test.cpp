#include "software/ProjectConfig.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

namespace
{
  class ProjectConfigTest : public testing::Test
  {
  protected:
    void SetUp() override
    {
      root_ =
          std::filesystem::temp_directory_path() /
          ("softadastra-project-config-" +
           std::to_string(
               std::chrono::steady_clock::now()
                   .time_since_epoch()
                   .count()));
      ASSERT_TRUE(std::filesystem::create_directories(root_));
    }

    void TearDown() override
    {
      std::filesystem::remove_all(root_);
    }

    void write(const std::string &contents)
    {
      std::ofstream output(root_ / "softadastra.toml");
      ASSERT_TRUE(output);
      output << contents;
    }

    std::filesystem::path root_;
  };

  TEST_F(ProjectConfigTest, ReadsMinimalConfigurationAndEmptyCommandSkeleton)
  {
    write("name = \"app\"\ncommand = \"./app\"\n");
    const auto minimal = softadastra::ProjectConfigFile::find(root_);
    ASSERT_TRUE(minimal);
    EXPECT_EQ(minimal->second.name, "app");
    EXPECT_EQ(minimal->second.command, "./app");
    EXPECT_TRUE(minimal->second.access_points.empty());

    write("name = \"skeleton\"\ncommand = \"\"\n");
    const auto skeleton = softadastra::ProjectConfigFile::find(root_);
    ASSERT_TRUE(skeleton);
    EXPECT_TRUE(skeleton->second.command.empty());
  }

  TEST_F(ProjectConfigTest, RejectsMissingOrEmptyRequiredFields)
  {
    write("name = \"\"\ncommand = \"./app\"\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write("name = \"app\"\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));
  }

  TEST_F(ProjectConfigTest, ReadsSingleAndMultipleAccessPoints)
  {
    write("name = \"app\"\ncommand = \"./app\"\naccess = \"http:8080\"\n");
    const auto single = softadastra::ProjectConfigFile::find(root_);
    ASSERT_TRUE(single);
    ASSERT_TRUE(single->second.access);
    EXPECT_EQ(single->second.access->port(), 8080);

    write(
        "name = \"app\"\ncommand = \"./app\"\n"
        "[[access]]\nprotocol = \"http\"\nport = 8080\n"
        "[[access]]\nprotocol = \"ws\"\nport = 9000\n");
    const auto multiple = softadastra::ProjectConfigFile::find(root_);
    ASSERT_TRUE(multiple);
    ASSERT_EQ(multiple->second.access_points.size(), 2U);
    EXPECT_EQ(multiple->second.access_points[0].port(), 8080);
    EXPECT_EQ(multiple->second.access_points[1].port(), 9000);
  }

  TEST_F(ProjectConfigTest, ReadsLegacyQuotedAccessPort)
  {
    write(
        "name = \"app\"\ncommand = \"./app\"\n"
        "[[access]]\nprotocol = \"http\"\nport = \"8080\"\n");
    const auto config = softadastra::ProjectConfigFile::find(root_);
    ASSERT_TRUE(config);
    ASSERT_EQ(config->second.access_points.size(), 1U);
    EXPECT_EQ(config->second.access_points.front().port(), 8080);
  }

  TEST_F(ProjectConfigTest, RejectsMixedAccessFormsDuplicatesAndInvalidPorts)
  {
    write(
        "name = \"app\"\ncommand = \"./app\"\naccess = \"http:8080\"\n"
        "[[access]]\nprotocol = \"ws\"\nport = 9000\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write("name = \"app\"\nname = \"other\"\ncommand = \"./app\"\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write("name = \"app\"\ncommand = \"./app\"\ncommand = \"other\"\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write(
        "name = \"app\"\ncommand = \"./app\"\n"
        "access = \"http:8080\"\naccess = \"https:8443\"\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write(
        "name = \"app\"\ncommand = \"./app\"\n"
        "[[access]]\nprotocol = \"http\"\nprotocol = \"ws\"\nport = 8080\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write(
        "name = \"app\"\ncommand = \"./app\"\n"
        "[[access]]\nprotocol = \"http\"\nport = 8080\nport = 8081\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write("name = \"app\"\ncommand = \"./app\"\naccess = \"http:8080abc\"\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));

    write(
        "name = \"app\"\ncommand = \"./app\"\n"
        "[[access]]\nprotocol = \"http\"\nport = 8080abc\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));
  }

  TEST_F(ProjectConfigTest, RejectsGlobalKeysWithinAccessTable)
  {
    write(
        "name = \"app\"\ncommand = \"./app\"\n"
        "[[access]]\nname = \"not-global\"\nprotocol = \"http\"\nport = 8080\n");
    EXPECT_FALSE(softadastra::ProjectConfigFile::find(root_));
  }

  TEST_F(ProjectConfigTest, IgnoresLegacyIdAndFindsNearestParentConfiguration)
  {
    write("id = \"legacy\"\nname = \"app\"\ncommand = \"./app\"\n");
    const auto legacy = softadastra::ProjectConfigFile::find(root_);
    ASSERT_TRUE(legacy);
    EXPECT_EQ(legacy->second.name, "app");

    const auto child = root_ / "src" / "nested";
    ASSERT_TRUE(std::filesystem::create_directories(child));
    const auto discovered = softadastra::ProjectConfigFile::find(child);
    ASSERT_TRUE(discovered);
    EXPECT_TRUE(std::filesystem::equivalent(discovered->first, root_));
  }

  TEST_F(ProjectConfigTest, WritesCanonicalIntegerPortsAndRoundTrips)
  {
    const auto http = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Http, 8080);
    const auto ws = softadastra::AccessPoint::create(
        softadastra::AccessProtocol::Ws, 9000);
    ASSERT_TRUE(http);
    ASSERT_TRUE(ws);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        root_, {"app", "./app", std::nullopt, {*http, *ws}}));

    std::ifstream input(root_ / "softadastra.toml");
    const std::string contents{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};
    EXPECT_NE(contents.find("port = 8080"), std::string::npos);
    EXPECT_EQ(contents.find("port = \"8080\""), std::string::npos);

    const auto round_trip = softadastra::ProjectConfigFile::find(root_);
    ASSERT_TRUE(round_trip);
    ASSERT_EQ(round_trip->second.access_points.size(), 2U);
    EXPECT_EQ(round_trip->second.access_points[1].port(), 9000);
  }
} // namespace
