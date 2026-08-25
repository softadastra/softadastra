#ifndef SOFTADASTRA_PLATFORM_GATEWAY_CAPABILITY_PROVISIONING_HPP
#define SOFTADASTRA_PLATFORM_GATEWAY_CAPABILITY_PROVISIONING_HPP

#include <filesystem>
#include <string>
#include <string_view>

namespace softadastra
{
  enum class GatewayCapabilityState { Present, Absent, Incorrect };

  class GatewayCapabilityProvisioning
  {
  public:
    [[nodiscard]] static GatewayCapabilityState status(
        std::string_view getcap_output, const std::filesystem::path &executable) noexcept;
    [[nodiscard]] static std::string command(const std::filesystem::path &executable);
  };
}

#endif
