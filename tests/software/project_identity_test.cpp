#include "software/ProjectIdentity.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>

TEST(ProjectIdentityTest, FindsIdentityFromProjectSubdirectoryAfterMove)
{
  const auto base = std::filesystem::temp_directory_path() /
                    ("softadastra-project-" + std::to_string(
                                                  std::chrono::steady_clock::now().time_since_epoch().count()));
  const auto source = base / "source";
  const auto destination = base / "destination";
  std::filesystem::create_directories(source / "src" / "nested");

  const auto created = softadastra::ProjectIdentity::create(source);
  ASSERT_TRUE(created.has_value());
  std::filesystem::rename(source, destination);

  const auto identity = softadastra::ProjectIdentity::find(destination / "src" / "nested");
  ASSERT_TRUE(identity.has_value());
  EXPECT_EQ(identity->first, destination);
  EXPECT_EQ(identity->second, created.value());
  EXPECT_FALSE(softadastra::ProjectIdentity::create(destination).has_value());
  std::filesystem::remove_all(base);
}
