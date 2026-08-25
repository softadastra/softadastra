#include "platform/GatewayCapabilityProvisioning.hpp"

#include <gtest/gtest.h>

namespace
{
  TEST(GatewayCapabilityProvisioningTest, RecognizesOnlyTheExpectedCapability)
  {
    const std::filesystem::path executable("/opt/softadastra/softadastra-gateway");
    EXPECT_EQ(softadastra::GatewayCapabilityProvisioning::status("", executable), softadastra::GatewayCapabilityState::Absent);
    EXPECT_EQ(softadastra::GatewayCapabilityProvisioning::status("/opt/softadastra/softadastra-gateway cap_net_bind_service=ep\n", executable), softadastra::GatewayCapabilityState::Present);
    EXPECT_EQ(softadastra::GatewayCapabilityProvisioning::status("/opt/softadastra/softadastra-gateway cap_net_admin=ep", executable), softadastra::GatewayCapabilityState::Incorrect);
  }

  TEST(GatewayCapabilityProvisioningTest, GeneratesTheMinimalIdempotentProvisioningCommand)
  {
    EXPECT_EQ(softadastra::GatewayCapabilityProvisioning::command("/opt/softadastra/softadastra-gateway"), "setcap cap_net_bind_service=ep '/opt/softadastra/softadastra-gateway'");
  }
}
