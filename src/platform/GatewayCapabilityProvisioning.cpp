#include "platform/GatewayCapabilityProvisioning.hpp"

namespace softadastra
{
  GatewayCapabilityState GatewayCapabilityProvisioning::status(
      std::string_view output, const std::filesystem::path &executable) noexcept
  {
    if (output.empty()) return GatewayCapabilityState::Absent;
    const std::string expected = executable.string() + " cap_net_bind_service=ep";
    while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) output.remove_suffix(1);
    return output == expected ? GatewayCapabilityState::Present : GatewayCapabilityState::Incorrect;
  }

  std::string GatewayCapabilityProvisioning::command(const std::filesystem::path &executable)
  {
    return "setcap cap_net_bind_service=ep '" + executable.string() + "'";
  }
}
