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
   *
   * ControlServer defines the Host operations available to control clients.
   *
   * It does not implement transport, serialization, sockets, or IPC. Those
   * concerns can be introduced later without changing the Host service model.
   */
  class ControlServer
  {
  public:
    /**
     * @brief Creates a control server.
     *
     * @param host_service Host service receiving control operations.
     */
    explicit ControlServer(HostService &host_service) noexcept;

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
    HostService &host_service_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_SERVER_HPP
