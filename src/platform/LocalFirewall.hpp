/**
 *
 *  @file LocalFirewall.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Softadastra
 */

#ifndef SOFTADASTRA_PLATFORM_LOCAL_FIREWALL_HPP
#define SOFTADASTRA_PLATFORM_LOCAL_FIREWALL_HPP

#include <cstdint>
#include <string>

namespace softadastra
{
  /**
   * @brief Describes the result of a local firewall operation.
   */
  enum class LocalFirewallResult
  {
    Open,
    Disabled,
    PermissionRequired,
    Unsupported,
    Failed
  };

  /**
   * @brief Describes a Host-owned TCP rule restricted to a local IPv4 subnet.
   */
  struct LocalFirewallRule
  {
    /**
     * @brief Ownership tag identifying the rule.
     */
    std::string owner;

    /**
     * @brief Local IPv4 subnet to which the rule applies.
     */
    std::string subnet;

    /**
     * @brief TCP port controlled by the rule.
     */
    std::uint16_t port{0};
  };

  /**
   * @brief Provides the platform interface for Host-owned local firewall rules.
   */
  class LocalFirewall
  {
  public:
    /**
     * @brief Destroys the local firewall provider.
     */
    virtual ~LocalFirewall() = default;

    /**
     * @brief Ensures that a Host-owned local firewall rule is usable.
     *
     * @param rule Firewall rule to ensure.
     *
     * @return Result of the firewall operation.
     */
    [[nodiscard]] virtual LocalFirewallResult ensure(
        const LocalFirewallRule &rule) = 0;

    /**
     * @brief Returns the current state of a local firewall rule.
     *
     * @param rule Firewall rule to inspect.
     *
     * @return Current firewall result for the rule.
     */
    [[nodiscard]] virtual LocalFirewallResult status(
        const LocalFirewallRule &rule)
    {
      return ensure(rule);
    }

    /**
     * @brief Explicitly enables a Host-owned local firewall rule.
     *
     * @param rule Firewall rule to enable.
     *
     * @return Result of the firewall operation.
     */
    [[nodiscard]] virtual LocalFirewallResult allow(
        const LocalFirewallRule &rule)
    {
      return ensure(rule);
    }

    /**
     * @brief Releases a Host-owned local firewall rule.
     *
     * @param rule Firewall rule to release.
     */
    virtual void release(
        const LocalFirewallRule &rule) noexcept = 0;

    /**
     * @brief Explicitly removes a Host-owned local firewall rule.
     *
     * @param rule Firewall rule to remove.
     *
     * @return Result of the firewall operation.
     */
    [[nodiscard]] virtual LocalFirewallResult deny(
        const LocalFirewallRule &rule)
    {
      release(rule);

      return LocalFirewallResult::Open;
    }
  };

  /**
   * @brief Provides a no-op local firewall implementation.
   */
  class UnavailableLocalFirewall final : public LocalFirewall
  {
  public:
    /**
     * @brief Reports the rule as usable without modifying the firewall.
     *
     * @param rule Firewall rule.
     *
     * @return LocalFirewallResult::Open.
     */
    [[nodiscard]] LocalFirewallResult ensure(
        const LocalFirewallRule &rule) override
    {
      static_cast<void>(rule);

      return LocalFirewallResult::Open;
    }

    /**
     * @brief Performs no operation for the supplied rule.
     *
     * @param rule Firewall rule.
     */
    void release(
        const LocalFirewallRule &rule) noexcept override
    {
      static_cast<void>(rule);
    }
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_LOCAL_FIREWALL_HPP
