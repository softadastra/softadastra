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
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareManager.hpp"
#include "software/SoftwareState.hpp"

#include <optional>

namespace softadastra
{
  /**
   * @brief Coordinates Host infrastructure capabilities.
   *
   * HostService provides operations over software lifecycle and connectivity
   * while keeping callers independent from the underlying platform details.
   *
   * Process creation remains below this service through ProcessLauncher.
   */
  class HostService
  {
  public:
    /**
     * @brief Creates a Host service.
     *
     * @param host Host coordinated by this service.
     * @param process_launcher Capability used to launch software processes.
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
     *
     * @param id Software identifier.
     * @param process_spec Information required to launch the software.
     *
     * @return true if registration succeeds, otherwise false.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec);

    /**
     * @brief Starts registered software.
     *
     * @param id Software identifier.
     *
     * @return true if the software starts successfully, otherwise false.
     */
    bool start_software(const SoftwareId &id);

    /**
     * @brief Stops running software.
     *
     * @param id Software identifier.
     *
     * @return true if the software stops successfully, otherwise false.
     */
    bool stop_software(const SoftwareId &id);

    /**
     * @brief Returns the lifecycle state of registered software.
     *
     * @param id Software identifier.
     *
     * @return The software state, or std::nullopt if the software is unknown.
     */
    [[nodiscard]] std::optional<SoftwareState> software_state(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns whether network connectivity is available.
     */
    [[nodiscard]] bool connectivity_available() const noexcept;

    /**
     * @brief Returns whether the Host is currently connected.
     */
    [[nodiscard]] bool connected() const noexcept;

  private:
    Host &host_;
    SoftwareManager software_manager_;
    ConnectivityManager connectivity_manager_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_SERVICE_HPP
