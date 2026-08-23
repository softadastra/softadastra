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
#include "platform/Process.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareManager.hpp"
#include "software/SoftwareState.hpp"

#include <optional>

namespace softadastra
{
  /**
   * @brief Coordinates the main infrastructure responsibilities of a Host.
   *
   * HostService is the central orchestration layer between the Host,
   * SoftwareManager, and ConnectivityManager.
   *
   * It does not implement process execution, networking, application logic, or
   * platform-specific behavior itself. Those responsibilities remain delegated
   * to the corresponding lower-level components.
   */
  class HostService
  {
  public:
    /**
     * @brief Creates a service for a Host.
     *
     * The Host instance must remain valid for the lifetime of HostService.
     *
     * @param host Host coordinated by this service.
     */
    explicit HostService(Host &host) noexcept;

    /**
     * @brief Returns the Host coordinated by this service.
     *
     * @return Reference to the Host.
     */
    [[nodiscard]] Host &host() noexcept;

    /**
     * @brief Returns the Host coordinated by this service.
     *
     * @return Constant reference to the Host.
     */
    [[nodiscard]] const Host &host() const noexcept;

    /**
     * @brief Registers software with the Host.
     *
     * @param id Identifier of the software to register.
     *
     * @return true if the software was registered, otherwise false.
     */
    bool register_software(SoftwareId id);

    /**
     * @brief Starts registered software using its process execution handle.
     *
     * HostService delegates lifecycle management to SoftwareManager.
     *
     * @param id Identifier of the software to start.
     * @param process Process execution handle associated with the software.
     *
     * @return true if the software started successfully, otherwise false.
     */
    bool start_software(
        const SoftwareId &id,
        Process &process);

    /**
     * @brief Stops registered software using its process execution handle.
     *
     * HostService delegates lifecycle management to SoftwareManager.
     *
     * @param id Identifier of the software to stop.
     * @param process Process execution handle associated with the software.
     *
     * @return true if the software stopped successfully, otherwise false.
     */
    bool stop_software(
        const SoftwareId &id,
        Process &process);

    /**
     * @brief Returns the lifecycle state of registered software.
     *
     * @param id Identifier of the software to inspect.
     *
     * @return Current software state, or std::nullopt if the software is not
     *         registered.
     */
    [[nodiscard]] std::optional<SoftwareState> software_state(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Checks whether networking is available to the Host.
     *
     * @return true if a usable network capability is available, otherwise false.
     */
    [[nodiscard]] bool connectivity_available() const noexcept;

    /**
     * @brief Checks whether the Host currently has network connectivity.
     *
     * This does not imply Internet access.
     *
     * @return true if the Host currently has network connectivity, otherwise
     *         false.
     */
    [[nodiscard]] bool connected() const noexcept;

  private:
    Host &host_;
    SoftwareManager software_manager_;
    ConnectivityManager connectivity_manager_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_SERVICE_HPP
