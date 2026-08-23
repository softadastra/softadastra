/**
 *
 *  @file HostService.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_SERVICE_HPP
#define SOFTADASTRA_HOST_HOST_SERVICE_HPP

#include "connectivity/ConnectivityManager.hpp"
#include "host/Host.hpp"
#include "platform/Network.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareManager.hpp"
#include "software/SoftwareState.hpp"

#include <optional>
#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Describes infrastructure information for reaching a Host locally.
   */
  struct LocalHostAccess
  {
    std::string host_name;
    std::vector<LocalNetworkAddress> addresses;
  };

  /**
   * @brief Coordinates Host infrastructure capabilities.
   */
  class HostService
  {
  public:
    /**
     * @brief Creates a Host service.
     */
    HostService(
        Host &host,
        ProcessLauncher &process_launcher) noexcept;

    /**
     * @brief Returns the managed Host.
     */
    [[nodiscard]] Host &host() noexcept;

    /**
     * @brief Returns the managed Host.
     */
    [[nodiscard]] const Host &host() const noexcept;

    /**
     * @brief Registers software with the Host.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec);

    /**
     * @brief Starts registered software.
     */
    [[nodiscard]] SoftwareOperationResult start_software(const SoftwareId &id);

    /**
     * @brief Stops running software.
     */
    [[nodiscard]] SoftwareOperationResult stop_software(const SoftwareId &id);

    /**
     * @brief Restarts registered software.
     */
    [[nodiscard]] SoftwareOperationResult restart_software(const SoftwareId &id);

    /**
     * @brief Stops every process managed by the Host.
     *
     * @return true when all managed processes stop successfully.
     */
    [[nodiscard]] bool shutdown();

    /**
     * @brief Refreshes software lifecycle state from running processes.
     */
    void refresh();

    /**
     * @brief Returns the lifecycle state of registered software.
     */
    [[nodiscard]] std::optional<SoftwareState> software_state(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns the last terminal lifecycle result for registered software.
     */
    [[nodiscard]] std::optional<SoftwareOperationResult> software_result(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns whether network connectivity is available.
     */
    [[nodiscard]] bool connectivity_available() const noexcept;

    /**
     * @brief Returns whether the Host is currently connected.
     */
    [[nodiscard]] bool connected() const noexcept;

    /**
     * @brief Returns infrastructure information for reaching the Host locally.
     */
    [[nodiscard]] LocalHostAccess local_access() const;

    /**
     * @brief Returns the current primary local IPv4 address when available.
     */
    [[nodiscard]] std::string primary_ipv4() const;

  private:
    Host &host_;
    SoftwareManager software_manager_;
    ConnectivityManager connectivity_manager_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_SERVICE_HPP
