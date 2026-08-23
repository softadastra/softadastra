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
#include "platform/Process.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <optional>

namespace softadastra
{
  /**
   * @brief Provides the control boundary for a Softadastra Host.
   *
   * ControlServer exposes Host operations to the control layer while keeping
   * HostService independent from CLI or transport concerns.
   *
   * At this stage, ControlServer represents the control contract only. It does
   * not define sockets, IPC, serialization, or another transport mechanism.
   * Those mechanisms may be introduced later when a real local control channel
   * is required.
   */
  class ControlServer
  {
  public:
    /**
     * @brief Creates a control server for a Host service.
     *
     * The HostService instance must remain valid for the lifetime of the
     * ControlServer.
     *
     * @param host_service Host service controlled by this server.
     */
    explicit ControlServer(HostService &host_service) noexcept;

    /**
     * @brief Registers software with the Host.
     *
     * @param id Identifier of the software to register.
     *
     * @return true if the software was registered, otherwise false.
     */
    bool register_software(SoftwareId id);

    /**
     * @brief Starts registered software.
     *
     * The supplied Process represents the platform execution handle associated
     * with the software.
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
     * @brief Stops registered software.
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
     * @return true if networking is available, otherwise false.
     */
    [[nodiscard]] bool connectivity_available() const noexcept;

    /**
     * @brief Checks whether the Host currently has network connectivity.
     *
     * @return true if the Host is connected, otherwise false.
     */
    [[nodiscard]] bool connected() const noexcept;

  private:
    HostService &host_service_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_SERVER_HPP
