/**
 *
 *  @file ControlClient.hpp
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

#ifndef SOFTADASTRA_CONTROL_CONTROL_CLIENT_HPP
#define SOFTADASTRA_CONTROL_CONTROL_CLIENT_HPP

#include "control/ControlServer.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <optional>
#include <filesystem>

namespace softadastra
{
  /**
   * @brief Sends Host control operations to a ControlServer.
   */
  class ControlClient
  {
  public:
    /**
     * @brief Creates a control client.
     */
    explicit ControlClient(ControlServer &server) noexcept;

    /**
     * @brief Creates a client for a local Host control endpoint.
     */
    explicit ControlClient(std::filesystem::path path) noexcept;

    /**
     * @brief Returns whether a local Host control endpoint is reachable.
     */
    [[nodiscard]] bool host_available() const noexcept;

    /**
     * @brief Registers software with the Host.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point = std::nullopt,
        std::optional<ProjectIdentity> project_identity = std::nullopt, std::string name = {});

    [[nodiscard]] std::optional<SoftwareEntry> software_by_project_identity(
        const ProjectIdentity &identity) const noexcept;
    bool update_project_root(const ProjectIdentity &identity, std::string root);
    [[nodiscard]] std::optional<ProjectIdentity> project_identity(const SoftwareId &id) const noexcept;
    [[nodiscard]] std::optional<SoftwareEntry> software(const SoftwareId &id) const noexcept;
    [[nodiscard]] std::vector<SoftwareEntry> software() const noexcept;
    [[nodiscard]] bool remove_software(const SoftwareId &id);
    bool link_project(const SoftwareId &id, ProjectIdentity identity, std::string root);
    [[nodiscard]] bool synchronize_software(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point, std::string name = {});

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
     *
     * @return Local Host information when the Host control endpoint responds.
     */
    [[nodiscard]] std::optional<LocalHostAccess> local_access() const noexcept;

    [[nodiscard]] std::optional<LocalAccess> local_access(
        const SoftwareId &id) noexcept;

    /**
     * @brief Returns detailed Host network capability when the endpoint responds.
     */
    [[nodiscard]] std::optional<NetworkCapability> network_capability() const noexcept;
    [[nodiscard]] std::optional<ManagedNetworkStatus> managed_network_status() const noexcept;
    [[nodiscard]] std::optional<ManagedNetworkStartResult> start_managed_network() const noexcept;
    [[nodiscard]] std::optional<bool> stop_managed_network() const noexcept;

  public:
    [[nodiscard]] std::optional<std::string> request(
        const std::string &message) const noexcept;

    ControlServer *server_{nullptr};
    std::filesystem::path path_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_CLIENT_HPP
