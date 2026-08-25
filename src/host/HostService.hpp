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
#include "platform/Network.hpp"
#include "platform/ManagedNetwork.hpp"
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
    std::string host_name;
    std::string primary_ipv4;
    std::vector<LocalNetworkAddress> addresses;
  };

  enum class LocalGatewayLookup { NotFound, Unavailable, Http };
  struct LocalGatewayTarget { LocalGatewayLookup result{LocalGatewayLookup::NotFound}; std::uint16_t port{}; };

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
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point = std::nullopt,
        std::optional<ProjectIdentity> project_identity = std::nullopt, std::string name = {});

    [[nodiscard]] std::optional<SoftwareEntry> software_by_project_identity(
        const ProjectIdentity &identity) const noexcept;
    bool update_project_root(const ProjectIdentity &identity, std::string root);
    [[nodiscard]] std::optional<ProjectIdentity> project_identity(const SoftwareId &id) const noexcept;
    [[nodiscard]] std::optional<SoftwareEntry> software(const SoftwareId &id) const noexcept;
    [[nodiscard]] std::optional<SoftwareEntry> find_by_name(const std::string &name) const noexcept;
    [[nodiscard]] std::vector<SoftwareEntry> software() const;
    [[nodiscard]] bool remove_software(const SoftwareId &id);
    bool link_project(const SoftwareId &id, ProjectIdentity identity, std::string root);
    [[nodiscard]] bool synchronize_software(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point, std::string name = {});

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

    [[nodiscard]] std::optional<AccessPoint> access_point(
        const SoftwareId &id) const noexcept;
    [[nodiscard]] LocalGatewayTarget local_gateway_target(std::string_view host) const;

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
     * @brief Resolves the current local access for a registered Software.
     *
     * @return No value when the Software has no AccessPoint or is unknown.
     */
    /**
     * @brief Resolves local access, starting the managed network only as a
     * safe fallback for a running Software with an AccessPoint.
     */
    [[nodiscard]] std::optional<LocalAccess> local_access(
        const SoftwareId &id) noexcept;

    /**
     * @brief Returns the Host's detailed, read-only network capability.
     */
    [[nodiscard]] NetworkCapability network_capability() const;
    [[nodiscard]] ManagedNetworkStatus managed_network_status() const;
    [[nodiscard]] ManagedNetworkStartResult start_managed_network();
    bool stop_managed_network();

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
