#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "gateway/ControlLocalGatewayTargetResolver.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
  TEST(ControlLocalGatewayTargetResolverTest, ResolvesCurrentControlStateWithoutCaching)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::ControlLocalGatewayTargetResolver resolver(client);
    const softadastra::SoftwareId id("stable-id");
    ASSERT_TRUE(service.register_software(
        id, softadastra::ProcessSpec("app"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080),
        std::nullopt, "phone-test"));
    auto *entry = host.state().find_software(id);
    ASSERT_NE(entry, nullptr);
    entry->set_state(softadastra::SoftwareState::Running);

    EXPECT_EQ(resolver.resolve("phone-test").result, softadastra::LocalGatewayLookup::Http);
    EXPECT_EQ(resolver.resolve("phone-test").port, 8080);
    EXPECT_EQ(resolver.resolve("unknown").result, softadastra::LocalGatewayLookup::NotFound);

    entry->set_access_point(softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 9000));
    EXPECT_EQ(resolver.resolve("phone-test").port, 9000);
    entry->set_state(softadastra::SoftwareState::Stopped);
    EXPECT_EQ(resolver.resolve("phone-test").result, softadastra::LocalGatewayLookup::Unavailable);
  }

  TEST(ControlLocalGatewayTargetResolverTest, TreatsUnavailableControlAsUnavailable)
  {
    softadastra::ControlClient client(
        std::filesystem::temp_directory_path() / "softadastra-missing-control.sock");
    softadastra::ControlLocalGatewayTargetResolver resolver(client);
    EXPECT_EQ(resolver.resolve("phone-test").result, softadastra::LocalGatewayLookup::Unavailable);
  }
}
