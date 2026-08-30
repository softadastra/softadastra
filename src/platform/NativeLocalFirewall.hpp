/** @file NativeLocalFirewall.hpp */
#ifndef SOFTADASTRA_PLATFORM_NATIVE_LOCAL_FIREWALL_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_LOCAL_FIREWALL_HPP

#include "platform/LocalFirewall.hpp"

namespace softadastra
{
  /** Linux UFW implementation; unsupported platforms report Unsupported. */
  class NativeLocalFirewall final : public LocalFirewall
  {
  public:
    [[nodiscard]] LocalFirewallResult ensure(
        const LocalFirewallRule &rule) override;
    void release(const LocalFirewallRule &rule) noexcept override;
  };
}
#endif
