/**
 *
 *  @file GatewayCapabilityProvisioning.hpp
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

#ifndef SOFTADASTRA_PLATFORM_GATEWAY_CAPABILITY_PROVISIONING_HPP
#define SOFTADASTRA_PLATFORM_GATEWAY_CAPABILITY_PROVISIONING_HPP

#include <filesystem>
#include <string>
#include <string_view>

namespace softadastra
{
  /**
   * @brief Describes the provisioning state of the gateway executable capability.
   */
  enum class GatewayCapabilityState
  {
    Present,
    Absent,
    Incorrect
  };

  /**
   * @brief Provides helpers for provisioning the gateway executable capability.
   */
  class GatewayCapabilityProvisioning
  {
  public:
    /**
     * @brief Determines the current gateway capability state.
     *
     * @param getcap_output Output produced by the capability inspection command.
     * @param executable Path to the gateway executable.
     *
     * @return Current capability provisioning state.
     */
    [[nodiscard]] static GatewayCapabilityState status(
        std::string_view getcap_output,
        const std::filesystem::path &executable) noexcept;

    /**
     * @brief Builds the command used to provision the gateway capability.
     *
     * @param executable Path to the gateway executable.
     *
     * @return Command used to configure the required executable capability.
     */
    [[nodiscard]] static std::string command(
        const std::filesystem::path &executable);
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_GATEWAY_CAPABILITY_PROVISIONING_HPP
