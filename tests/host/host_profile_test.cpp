#include "host/HostProfile.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(HostProfileTest, NewHostIsStandardThenProvisioningPersistsBoxProfile)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-profile";
    const auto path = directory / "host-profile";
    std::filesystem::remove_all(directory);

    softadastra::HostProfileStore first(path);
    EXPECT_TRUE(first.load("host-id"));
    EXPECT_EQ(first.profile(), softadastra::HostProfile::Standard);
    EXPECT_EQ(
        softadastra::box_state(first.profile(), true, {}),
        softadastra::BoxState::NotProvisioned);
    ASSERT_TRUE(first.provision_box("host-id"));
    EXPECT_EQ(first.profile(), softadastra::HostProfile::Box);
#if !defined(_WIN32)
    EXPECT_EQ(
        std::filesystem::status(path).permissions() & std::filesystem::perms::all,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write);
#endif
    EXPECT_EQ(
        softadastra::box_state(
            first.profile(), true,
            {softadastra::ManagedNetworkCapability::Available,
             softadastra::ManagedNetworkState::Running, "wlan1", "10.42.0.1", "test"}),
        softadastra::BoxState::Ready);
    EXPECT_EQ(
        softadastra::box_state(
            first.profile(), true,
            {softadastra::ManagedNetworkCapability::Unavailable,
             softadastra::ManagedNetworkState::Stopped,
             {},
             {},
             {}}),
        softadastra::BoxState::Degraded);
    EXPECT_EQ(
        softadastra::box_state(first.profile(), false, {}),
        softadastra::BoxState::Stopped);
    EXPECT_EQ(
        softadastra::box_state(
            first.profile(), true,
            {softadastra::ManagedNetworkCapability::Available,
             softadastra::ManagedNetworkState::Stopped,
             {},
             {},
             {}}),
        softadastra::BoxState::Degraded);

    softadastra::HostProfileStore restored(path);
    EXPECT_TRUE(restored.load("host-id"));
    EXPECT_EQ(restored.profile(), softadastra::HostProfile::Box);
    EXPECT_FALSE(restored.load("other-host-id"));
    EXPECT_TRUE(restored.unprovision());
    EXPECT_EQ(restored.profile(), softadastra::HostProfile::Standard);
    EXPECT_TRUE(softadastra::HostProfileStore(path).load("host-id"));
    std::filesystem::remove_all(directory);
  }
}
