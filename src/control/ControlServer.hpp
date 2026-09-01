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

#include <cstdint>
#include <optional>

namespace softadastra
{
  /**
   * @brief Represents a chunk of software log output.
   */
  struct LogChunk
  {
    /**
     * @brief Log data contained in the chunk.
     */
    std::string logs;

    /**
     * @brief Offset immediately following the returned log data.
     */
    std::uintmax_t offset{0};

    /**
     * @brief Indicates whether the log stream was reset.
     */
    bool reset{false};
  };

  /**
   * @brief Exposes Host control operations.
   *
   * ControlServer provides operations for registering and managing software,
   * inspecting lifecycle state and logs, discovering local access information,
   * and controlling Host network capabilities.
   */
  class ControlServer
  {
  public:
    /**
     * @brief Creates a control server.
     *
     * @param host_service Host service used to perform control operations.
     */
    explicit ControlServer(HostService &host_service) noexcept;

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
     * @return The matching software entry, or std::nullopt if none is found.
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
     * @return The associated project identity, or std::nullopt if none exists.
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
     * @brief Returns all software registered with the Host.
     *
     * @return Registered software entries.
     */
    [[nodiscard]] std::vector<SoftwareEntry> software() const;

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
     * @return Log contents when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<std::string> logs(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns an initial log tail or data written after an offset.
     *
     * @param id Identifier of the software.
     * @param offset Optional log offset from which to continue reading.
     *
     * @return A log chunk when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<LogChunk> logs_since(
        const SoftwareId &id,
        std::optional<std::uintmax_t> offset) const noexcept;

    /**
     * @brief Clears the logs associated with registered software.
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
     * @param root Root path of the project.
     *
     * @return true if the project was linked successfully, otherwise false.
     */
    bool link_project(
        const SoftwareId &id,
        ProjectIdentity identity,
        std::string root);

    /**
     * @brief Synchronizes a registered software definition.
     *
     * @param id Identifier of the software.
     * @param process_spec Current process configuration.
     * @param access_point Optional access point exposed by the software.
     * @param name Optional human-readable software name.
     *
     * @return true if synchronization succeeded, otherwise false.
     */
    [[nodiscard]] bool synchronize_software(
        const SoftwareId &id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point,
        std::string name = {});

    /**
     * @brief Synchronizes a registered software definition with multiple access points.
     *
     * @param id Identifier of the software.
     * @param process_spec Current process configuration.
     * @param access_points Access points exposed by the software.
     * @param name Optional human-readable software name.
     *
     * @return true if synchronization succeeded, otherwise false.
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
     * @brief Resolves a host name to a local gateway target.
     *
     * @param host Host name to resolve.
     *
     * @return Local gateway target corresponding to the host.
     */
    [[nodiscard]] LocalGatewayTarget local_gateway_target(
        std::string_view host) const;

    /**
     * @brief Returns the current local reachability state.
     *
     * @return Current local reachability state of the Host.
     */
    [[nodiscard]] LocalReachabilityState local_reachability_state() const noexcept;

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
     * @brief Refreshes software lifecycle state from running processes.
     */
    void refresh();

    /**
     * @brief Returns the lifecycle state of registered software.
     *
     * @param id Identifier of the software.
     *
     * @return Current lifecycle state, or std::nullopt if unavailable.
     */
    [[nodiscard]] std::optional<SoftwareState> software_state(
        const SoftwareId &id) noexcept;

    /**
     * @brief Returns the last terminal lifecycle result for registered software.
     *
     * @param id Identifier of the software.
     *
     * @return Last operation result, or std::nullopt if unavailable.
     */
    [[nodiscard]] std::optional<SoftwareOperationResult> software_result(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns whether network connectivity is available.
     *
     * @return true if connectivity is available, otherwise false.
     */
    [[nodiscard]] bool connectivity_available() const noexcept;

    /**
     * @brief Returns whether the Host is currently connected.
     *
     * @return true if the Host is connected, otherwise false.
     */
    [[nodiscard]] bool connected() const noexcept;

    /**
     * @brief Returns infrastructure information for reaching the Host locally.
     *
     * @return Local Host access information.
     */
    [[nodiscard]] LocalHostAccess local_access() const;

    /**
     * @brief Returns local access information for registered software.
     *
     * @param id Identifier of the software.
     *
     * @return Local access information when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<LocalAccess> local_access(
        const SoftwareId &id) noexcept;

    /**
     * @brief Returns the current Host network capability.
     *
     * @return Current network capability.
     */
    [[nodiscard]] NetworkCapability network_capability() const;

    /**
     * @brief Returns the current managed network status.
     *
     * @return Current managed network status.
     */
    [[nodiscard]] ManagedNetworkStatus managed_network_status() const;

    /**
     * @brief Starts the Host-managed network.
     *
     * @return Result of the managed network start operation.
     */
    [[nodiscard]] ManagedNetworkStartResult start_managed_network();

    /**
     * @brief Stops the Host-managed network.
     *
     * @return true if the managed network was stopped successfully, otherwise false.
     */
    bool stop_managed_network();

  private:
    HostService &host_service_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_SERVER_HPP
