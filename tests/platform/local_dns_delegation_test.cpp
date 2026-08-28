#include "platform/LocalDnsConfiguration.hpp"
#include "platform/LocalDnsDelegation.hpp"
#include "platform/NativeLocalDnsDelegation.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
namespace
{
  std::filesystem::path directory()
  {
    auto p = std::filesystem::temp_directory_path() / ("softadastra-dns-delegation-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(p);
    return p;
  }
  TEST(LocalDnsDelegationTest, IsUnavailableWithoutFragment)
  {
    const auto p = directory();
    EXPECT_EQ(softadastra::NativeLocalDnsDelegation(p).status(), softadastra::LocalDnsDelegationState::Unavailable);
    std::filesystem::remove_all(p);
  }
  TEST(LocalDnsDelegationTest, RecognizesExactExpectedFragment)
  {
    const auto p = directory();
    std::ofstream(p / "softadastra.conf") << softadastra::LocalDnsDelegation::configuration();
    EXPECT_EQ(softadastra::NativeLocalDnsDelegation(p).status(), softadastra::LocalDnsDelegationState::Available);
    std::filesystem::remove_all(p);
  }
  TEST(LocalDnsDelegationTest, RejectsWrongPortAndMalformedContent)
  {
    const auto p = directory();
    std::ofstream(p / "softadastra.conf") << "server=/softadastra.home.arpa/127.0.0.1#53536\n";
    EXPECT_EQ(softadastra::NativeLocalDnsDelegation(p).status(), softadastra::LocalDnsDelegationState::Misconfigured);
    std::ofstream(p / "softadastra.conf", std::ios::trunc) << "server=/softadastra.home.arpa/\n";
    EXPECT_EQ(softadastra::NativeLocalDnsDelegation(p).status(), softadastra::LocalDnsDelegationState::Misconfigured);
    std::filesystem::remove_all(p);
  }
  TEST(LocalDnsDelegationTest, RejectsOtherZoneAndCentralizesPort)
  {
    const auto p = directory();
    std::ofstream(p / "softadastra.conf") << "server=/example.com/127.0.0.1#53535\n";
    EXPECT_EQ(softadastra::NativeLocalDnsDelegation(p).status(), softadastra::LocalDnsDelegationState::Unavailable);
    EXPECT_EQ(softadastra::local_dns_port, 53535);
    EXPECT_EQ(softadastra::LocalDnsDelegation::configuration(), "server=/softadastra.home.arpa/127.0.0.1#53535\n");
    std::filesystem::remove_all(p);
  }
}
