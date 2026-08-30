/**
 *
 *  @file LocalFirewall.hpp
 *  @brief Platform capability for Host-owned local firewall rules.
 */

#ifndef SOFTADASTRA_PLATFORM_LOCAL_FIREWALL_HPP
#define SOFTADASTRA_PLATFORM_LOCAL_FIREWALL_HPP

#include <cstdint>
#include <string>

namespace softadastra
{
  enum class LocalFirewallResult
  {
    Open,
    PermissionRequired,
    Unsupported,
    Failed
  };

  /** A TCP rule restricted to one local IPv4 subnet and owned by the Host. */
  struct LocalFirewallRule
  {
    std::string owner;
    std::string subnet;
    std::uint16_t port{0};
  };

  class LocalFirewall
  {
  public:
    virtual ~LocalFirewall() = default;

    /** Ensures this local-only rule is usable, without modifying user rules. */
    [[nodiscard]] virtual LocalFirewallResult ensure(
        const LocalFirewallRule &rule) = 0;

    /** Releases only a rule previously identified as Host-owned. */
    virtual void release(const LocalFirewallRule &rule) noexcept = 0;
  };

  class UnavailableLocalFirewall final : public LocalFirewall
  {
  public:
    [[nodiscard]] LocalFirewallResult ensure(
        const LocalFirewallRule &) override
    {
      return LocalFirewallResult::Open;
    }

    void release(const LocalFirewallRule &) noexcept override {}
  };
}

#endif
