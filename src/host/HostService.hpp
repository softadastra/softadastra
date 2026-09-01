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
#include "host/LocalAccess.hpp"
#include "host/LocalGatewayTargetResolver.hpp"
#include "host/LocalReachability.hpp"
#include "platform/ManagedNetwork.hpp"
#include "platform/LocalFirewall.hpp"
#include "platform/Network.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareManager.hpp"
#include "software/SoftwareState.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace softadastra
{
  /**
   * @brief Describes infrastructure information for reaching a Host locally.
   */
  struct LocalHostAccess
  {
    /**
     * @brief Host name advertised on the local network.
     */
    std::string host_name;

    /**
     * @brief Primary local IPv4 address.
     */
    std::string primary_ipv4;

    /**
     * @brief Local network addresses associated with the Host.
     */
    std::vector<LocalNetworkAddress> addresses;
  };

  /**
   * @brief Coordinates Host infrastructure and software lifecycle capabilities.
   */
  class HostService : public LocalGatewayTargetResolver
  {
  public:
    /**
     * @brief Creates a Host service.
     *
     * @param host Host managed by this service.
     * @param process_launcher Process launcher used for managed software.
     */
    HostService(
        Host &host,
        ProcessLauncher &process_launcher) noexcept;

    /**
     * @brief Returns the managed Host.
     *
     * @return Mutable reference to the managed Host.
     */
    [[nodiscard]] Host &host() noexcept;

    /**
     * @brief Returns the managed Host.
     *
     * @return Const reference to the managed Host.
     */
    [[nodiscard]] const Host &host() const noexcept;

    /**
     * @brief Registers software with the Host.
     *
     * @param id Software identifier.
     * @param process_spec Process specification used to launch the software.
     * @param access_point Optional access point exposed by the software.
     * @param project_identity Optional associated project identity.
     * @param name Optional software name.
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
     * @return Matching software entry, or std::nullopt if none is found.
     */
    [[nodiscard]] std::optional<SoftwareEntry>
    software_by_project_identity(
        const ProjectIdentity &identity) const noexcept;

    /**
     * @brief Updates the project root associated with a project identity.
     *
     * @param identity Project identity to update.
     * @param root New project root.
     *
     * @return true if the project root was updated successfully, otherwise false.
     */
    bool update_project_root(
        const ProjectIdentity &identity,
        std::string root);

    /**
     * @brief Returns the project identity associated with registered software.
     *
     * @param id Software identifier.
     *
     * @return Associated project identity, or std::nullopt when unavailable.
     */
    [[nodiscard]] std::optional<ProjectIdentity> project_identity(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns registered software by identifier.
     *
     * @param id Software identifier.
     *
     * @return Matching software entry, or std::nullopt if none is found.
     */
    [[nodiscard]] std::optional<SoftwareEntry> software(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Finds registered software by name.
     *
     * @param name Software name to look up.
     *
     * @return Matching software entry, or std::nullopt if none is found.
     */
    [[nodiscard]] std::optional<SoftwareEntry> find_by_name(
        const std::string &name) const noexcept;

    /**
     * @brief Returns all registered software.
     *
     * @return Registered software entries.
     */
    [[nodiscard]] std::vector<SoftwareEntry> software() const;

    /**
     * @brief Removes registered software.
     *
     * @param id Software identifier.
     *
     * @return true if the software was removed successfully, otherwise false.
     */
    [[nodiscard]] bool remove_software(
        const SoftwareId &id);

    /**
     * @brief Associates registered software with a project.
     *
     * @param id Software identifier.
     * @param identity Project identity to associate.
     * @param root Project root to associate.
     *
     * @return true if the project was linked successfully, otherwise false.
     */
    bool link_project(
        const SoftwareId &id,
        ProjectIdentity identity,
        std::string root);

    /**
     * @brief Synchronizes registered software with a process specification.
     *
     * @param id Software identifier.
     * @param process_spec Updated process specification.
     * @param access_point Optional access point.
     * @param name Optional software name.
     *
     * @return true if the software was synchronized successfully, otherwise false.
     */
    [[nodiscard]] bool synchronize_software(
        const SoftwareId &id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point,
        std::string name = {});

    /**
     * @brief Synchronizes registered software with multiple access points.
     *
     * @param id Software identifier.
     * @param process_spec Updated process specification.
     * @param access_points Access points exposed by the software.
     * @param name Optional software name.
     *
     * @return true if the software was synchronized successfully, otherwise false.
     */
    [[nodiscard]] bool synchronize_software(
        const SoftwareId &id,
        ProcessSpec process_spec,
        std::vector<AccessPoint> access_points,
        std::string name = {});

    /**
     * @brief Starts registered software.
     *
     * @param id Software identifier.
     *
     * @return Result of the start operation.
     */
    [[nodiscard]] SoftwareOperationResult start_software(
        const SoftwareId &id);

    /**
     * @brief Stops running software.
     *
     * @param id Software identifier.
     *
     * @return Result of the stop operation.
     */
    [[nodiscard]] SoftwareOperationResult stop_software(
        const SoftwareId &id);

    /**
     * @brief Restarts registered software.
     *
     * @param id Software identifier.
     *
     * @return Result of the restart operation.
     */
    [[nodiscard]] SoftwareOperationResult restart_software(
        const SoftwareId &id);

    /**
     * @brief Restores software that was persistently requested to run.
     *
     * A failed restoration does not prevent other registrations from being
     * restored.
     */
    void restore_desired_software();

    /**
     * @brief Stops every process and managed network owned by the Host.
     *
     * @return true when shutdown completes successfully, otherwise false.
     */
    [[nodiscard]] bool shutdown();

    /**
     * @brief Refreshes software lifecycle state from running processes.
     */
    void refresh();

    /**
     * @brief Returns the lifecycle state of registered software.
     *
     * @param id Software identifier.
     *
     * @return Current software state, or std::nullopt if the software is unknown.
     */
    [[nodiscard]] std::optional<SoftwareState> software_state(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns the last terminal lifecycle result for registered software.
     *
     * @param id Software identifier.
     *
     * @return Last operation result, or std::nullopt when unavailable.
     */
    [[nodiscard]] std::optional<SoftwareOperationResult> software_result(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns the access point associated with registered software.
     *
     * @param id Software identifier.
     *
     * @return Access point when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<AccessPoint> access_point(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Resolves a local gateway host name to a gateway target.
     *
     * @param host Host name to resolve.
     *
     * @return Resolved local gateway target.
     */
    [[nodiscard]] LocalGatewayTarget resolve(
        std::string_view host) const override;

    /**
     * @brief Sets the local reachability provider used by the Host service.
     *
     * @param reachability Local reachability provider.
     */
    void set_local_reachability(
        LocalReachability *reachability) noexcept
    {
      local_reachability_ = reachability;
    }

    /**
     * @brief Returns the current local reachability state.
     *
     * @return Current local reachability state.
     */
    [[nodiscard]] LocalReachabilityState
    local_reachability_state() const noexcept;

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
     * @brief Resolves local access for registered software.
     *
     * A managed network may be started as a fallback when the software is
     * running, exposes an AccessPoint, and no usable local network is available.
     *
     * @param id Software identifier.
     *
     * @return Local access information, or std::nullopt if the software is
     *         unknown or has no AccessPoint.
     */
    [[nodiscard]] std::optional<LocalAccess> local_access(
        const SoftwareId &id) noexcept;

    /** Resolves every declared local access endpoint for software. */
    [[nodiscard]] std::optional<std::vector<LocalAccess>> local_accesses(
        const SoftwareId &id) noexcept;

    /**
     * @brief Returns the Host's detailed network capability.
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
     * @brief Starts the managed network.
     *
     * @return Result of the start operation.
     */
    [[nodiscard]] ManagedNetworkStartResult start_managed_network();

    /**
     * @brief Stops the managed network.
     *
     * @return true if the managed network was stopped successfully,
     *         otherwise false.
     */
    bool stop_managed_network();

    /**
     * @brief Returns the current primary local IPv4 address when available.
     *
     * @return Primary local IPv4 address.
     */
    [[nodiscard]] std::string primary_ipv4() const;

  private:
    Host &host_;
    SoftwareManager software_manager_;
    ConnectivityManager connectivity_manager_;
    LocalReachability *local_reachability_{nullptr};
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_SERVICE_HPP
