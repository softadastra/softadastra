/**
 *
 *  @file ControlServer.hpp
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

#ifndef SOFTADASTRA_CONTROL_CONTROL_SERVER_HPP
#define SOFTADASTRA_CONTROL_CONTROL_SERVER_HPP

#include "host/HostService.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <optional>

namespace softadastra
{
  /**
   * @brief Exposes Host control operations.
   */
  class ControlServer
  {
  public:
    /**
     * @brief Creates a control server.
     */
    explicit ControlServer(HostService &host_service) noexcept;

    /**
     * @brief Registers software with the Host.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point = std::nullopt,
        std::optional<ProjectIdentity> project_identity = std::nullopt);

    [[nodiscard]] std::optional<SoftwareEntry> software_by_project_identity(
        const ProjectIdentity &identity) const noexcept;
    bool update_project_root(const ProjectIdentity &identity, std::string root);
    [[nodiscard]] std::optional<ProjectIdentity> project_identity(const SoftwareId &id) const noexcept;
    [[nodiscard]] std::optional<SoftwareEntry> software(const SoftwareId &id) const noexcept;
    [[nodiscard]] std::vector<SoftwareEntry> software() const;
    [[nodiscard]] bool remove_software(const SoftwareId &id);
    bool link_project(const SoftwareId &id, ProjectIdentity identity, std::string root);
    [[nodiscard]] bool synchronize_software(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point);

    [[nodiscard]] std::optional<AccessPoint> access_point(const SoftwareId &id) const noexcept;

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

    [[nodiscard]] std::optional<LocalAccess> local_access(
        const SoftwareId &id) noexcept;

    [[nodiscard]] NetworkCapability network_capability() const;
    [[nodiscard]] ManagedNetworkStatus managed_network_status() const;
    [[nodiscard]] ManagedNetworkStartResult start_managed_network();
    bool stop_managed_network();

  private:
    HostService &host_service_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_SERVER_HPP
