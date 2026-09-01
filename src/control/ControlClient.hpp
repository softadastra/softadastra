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

#include <filesystem>
#include <optional>

namespace softadastra
{
  /**
   * @brief Provides access to Host control operations.
   *
   * ControlClient exposes the client-side interface used to inspect and
   * control a Softadastra Host. A client can communicate directly with an
   * existing ControlServer instance or through a local control endpoint.
   *
   * The interface covers software registration and lifecycle operations,
   * log access, project association, local access discovery, and managed
   * network operations.
   */
  class ControlClient
  {
  public:
    /**
     * @brief Constructs a client backed by an existing control server.
     *
     * @param server Control server used to process client operations.
     */
    explicit ControlClient(ControlServer &server) noexcept;

    /**
     * @brief Constructs a client for a local Host control endpoint.
     *
     * @param path Filesystem path identifying the local control endpoint.
     */
    explicit ControlClient(std::filesystem::path path) noexcept;

    /**
     * @brief Checks whether the configured Host control endpoint answers ping.
     *
     * @return true only when the Host answers the expected @c ok response.
     */
    [[nodiscard]] bool host_available() const noexcept;

    /**
     * @brief Registers software with the Host.
     *
     * @param id Identifier assigned to the software.
     * @param process_spec Process configuration used to run the software.
     * @param access_point Optional access point exposed by the software.
     * @param project_identity Optional identity of the associated project.
     * @param name Optional human-readable software name.
     *
     * @return true if the software was registered successfully, otherwise false.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point = std::nullopt,
        std::optional<ProjectIdentity> project_identity = std::nullopt,
        std::string name = {});

    /**
     * @brief Finds software associated with a project identity.
     *
     * @param identity Project identity to look up.
     *
     * @return The matching software entry, or std::nullopt if no matching
     *         software is available.
     */
    [[nodiscard]] std::optional<SoftwareEntry> software_by_project_identity(
        const ProjectIdentity &identity) const noexcept;

    /**
     * @brief Updates the root path associated with a project.
     *
     * @param identity Identity of the project to update.
     * @param root New project root path.
     *
     * @return true if the project root was updated successfully, otherwise false.
     */
    bool update_project_root(
        const ProjectIdentity &identity,
        std::string root);

    /**
     * @brief Returns the project identity associated with software.
     *
     * @param id Identifier of the software.
     *
     * @return The associated project identity, or std::nullopt if no project
     *         identity is associated with the software.
     */
    [[nodiscard]] std::optional<ProjectIdentity> project_identity(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns a registered software entry.
     *
     * @param id Identifier of the software.
     *
     * @return The matching software entry, or std::nullopt if it is not found.
     */
    [[nodiscard]] std::optional<SoftwareEntry> software(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns all software known to the Host.
     *
     * @return Registered software entries.
     */
    [[nodiscard]] std::vector<SoftwareEntry> software() const noexcept;

    /**
     * @brief Removes software from the Host.
     *
     * @param id Identifier of the software to remove.
     *
     * @return true if the software was removed successfully, otherwise false.
     */
    [[nodiscard]] bool remove_software(const SoftwareId &id);

    /**
     * @brief Returns the available logs for registered software.
     *
     * @param id Identifier of the software.
     *
     * @return Log contents when available, or std::nullopt if logs cannot be
     *         retrieved.
     */
    [[nodiscard]] std::optional<std::string> logs(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns software logs beginning at an optional byte offset.
     *
     * @param id Identifier of the software.
     * @param offset Optional offset from which log data should be returned.
     *
     * @return A log chunk when available, or std::nullopt if log data cannot
     *         be retrieved.
     */
    [[nodiscard]] std::optional<LogChunk> logs_since(
        const SoftwareId &id,
        std::optional<std::uintmax_t> offset) const noexcept;

    /**
     * @brief Clears the stored logs for registered software.
     *
     * @param id Identifier of the software.
     *
     * @return true if the logs were cleared successfully, otherwise false.
     */
    [[nodiscard]] bool clear_logs(const SoftwareId &id) const noexcept;

    /**
     * @brief Associates registered software with a project.
     *
     * @param id Identifier of the software.
     * @param identity Project identity to associate with the software.
     * @param root Root path of the associated project.
     *
     * @return true if the project was linked successfully, otherwise false.
     */
    bool link_project(
        const SoftwareId &id,
        ProjectIdentity identity,
        std::string root);

    /**
     * @brief Synchronizes the Host definition of registered software.
     *
     * @param id Identifier of the software.
     * @param process_spec Current process configuration.
     * @param access_point Optional access point exposed by the software.
     * @param name Optional human-readable software name.
     *
     * @return true if the software definition was synchronized successfully,
     *         otherwise false.
     */
    [[nodiscard]] bool synchronize_software(
        const SoftwareId &id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point,
        std::string name = {});

    /**
     * @brief Synchronizes the Host definition of software with multiple access points.
     *
     * @param id Identifier of the software.
     * @param process_spec Current process configuration.
     * @param access_points Access points exposed by the software.
     * @param name Optional human-readable software name.
     *
     * @return true if the software definition was synchronized successfully,
     *         otherwise false.
     */
    [[nodiscard]] bool synchronize_software(
        const SoftwareId &id,
        ProcessSpec process_spec,
        std::vector<AccessPoint> access_points,
        std::string name = {});

    /**
     * @brief Returns the primary access point associated with software.
     *
     * @param id Identifier of the software.
     *
     * @return The access point when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<AccessPoint> access_point(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Resolves a host name to its local gateway target.
     *
     * @param host Host name to resolve.
     *
     * @return Local gateway target corresponding to the supplied host name.
     */
    [[nodiscard]] LocalGatewayTarget local_gateway_target(
        std::string_view host) const noexcept;

    /**
     * @brief Returns the current local reachability state of the Host.
     *
     * @return The local reachability state when available, or std::nullopt
     *         when it cannot be determined.
     */
    [[nodiscard]] std::optional<LocalReachabilityState>
    local_reachability_state() const noexcept;

    /**
     * @brief Starts registered software.
     *
     * @param id Identifier of the software to start.
     *
     * @return Result of the start operation.
     */
    [[nodiscard]] SoftwareOperationResult start_software(
        const SoftwareId &id);

    /**
     * @brief Stops running software.
     *
     * @param id Identifier of the software to stop.
     *
     * @return Result of the stop operation.
     */
    [[nodiscard]] SoftwareOperationResult stop_software(
        const SoftwareId &id);

    /**
     * @brief Restarts registered software.
     *
     * @param id Identifier of the software to restart.
     *
     * @return Result of the restart operation.
     */
    [[nodiscard]] SoftwareOperationResult restart_software(
        const SoftwareId &id);

    /**
     * @brief Refreshes software lifecycle information from running processes.
     *
     * Updates the Host's view of registered software to reflect the current
     * state of the corresponding processes.
     */
    void refresh();

    /**
     * @brief Returns the current lifecycle state of registered software.
     *
     * @param id Identifier of the software.
     *
     * @return The current lifecycle state, or std::nullopt if the state is not
     *         available.
     */
    [[nodiscard]] std::optional<SoftwareState> software_state(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns the most recent terminal lifecycle result for software.
     *
     * @param id Identifier of the software.
     *
     * @return The most recent operation result when available, or std::nullopt
     *         otherwise.
     */
    [[nodiscard]] std::optional<SoftwareOperationResult> software_result(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Checks whether network connectivity is available to the Host.
     *
     * @return true if connectivity is available, otherwise false.
     */
    [[nodiscard]] bool connectivity_available() const noexcept;

    /**
     * @brief Checks whether the Host is currently connected.
     *
     * @return true if the Host is connected, otherwise false.
     */
    [[nodiscard]] bool connected() const noexcept;

    /**
     * @brief Returns information required to reach the Host locally.
     *
     * @return Local Host access information when available, or std::nullopt
     *         when the Host cannot provide it.
     */
    [[nodiscard]] std::optional<LocalHostAccess> local_access() const noexcept;

    /**
     * @brief Returns local access information for registered software.
     *
     * @param id Identifier of the software.
     *
     * @return Local software access information when available, or std::nullopt
     *         otherwise.
     */
    [[nodiscard]] std::optional<LocalAccess> local_access(
        const SoftwareId &id) noexcept;

    /**
     * @brief Returns the Host's current network capability.
     *
     * @return Network capability information when available, or std::nullopt
     *         when it cannot be retrieved.
     */
    [[nodiscard]] std::optional<NetworkCapability>
    network_capability() const noexcept;

    /**
     * @brief Returns the current status of the Host-managed network.
     *
     * @return Managed network status when available, or std::nullopt when the
     *         status cannot be retrieved.
     */
    [[nodiscard]] std::optional<ManagedNetworkStatus>
    managed_network_status() const noexcept;

    /**
     * @brief Requests startup of the Host-managed network.
     *
     * @return The managed network startup result when the request can be
     *         processed, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<ManagedNetworkStartResult>
    start_managed_network() const noexcept;

    /**
     * @brief Requests shutdown of the Host-managed network.
     *
     * @return The result of the stop operation when available, or std::nullopt
     *         when the request cannot be processed.
     */
    [[nodiscard]] std::optional<bool> stop_managed_network() const noexcept;

  public:
    /**
     * @brief Sends a raw request to the configured control endpoint.
     *
     * @param message Request payload to send.
     *
     * @return The response payload when a response is available, or
     *         std::nullopt if the request cannot be completed.
     */
    [[nodiscard]] std::optional<std::string> request(
        const std::string &message) const noexcept;

    ControlServer *server_{nullptr};
    std::filesystem::path path_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_CLIENT_HPP
