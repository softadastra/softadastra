/**
 *
 *  @file FirewallHelper.hpp
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

#ifndef SOFTADASTRA_PLATFORM_FIREWALL_HELPER_HPP
#define SOFTADASTRA_PLATFORM_FIREWALL_HELPER_HPP

#include "platform/LocalFirewall.hpp"
#include "platform/Network.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Describes the operation performed by the firewall helper.
   */
  enum class FirewallHelperCommand
  {
    Status,
    Modify
  };

  /**
   * @brief Validates and executes local firewall helper operations.
   */
  class FirewallHelper final
  {
  public:
    /**
     * @brief Creates a firewall helper.
     *
     * @param network Network provider used to validate local network rules.
     * @param firewall Firewall provider used to inspect or modify rules.
     */
    FirewallHelper(
        const Network &network,
        LocalFirewall &firewall) noexcept;

    /**
     * @brief Executes a firewall helper command.
     *
     * @param command Operation to perform.
     * @param arguments Command arguments.
     * @param output Stream receiving command output.
     *
     * @return 0 on success, 1 when a firewall operation fails, or 2 when the
     *         supplied arguments are invalid.
     */
    [[nodiscard]] int run(
        FirewallHelperCommand command,
        const std::vector<std::string> &arguments,
        std::ostream &output) const;

  private:
    const Network &network_;
    LocalFirewall &firewall_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_FIREWALL_HELPER_HPP
