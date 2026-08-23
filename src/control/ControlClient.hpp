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
     * @brief Registers software with the Host.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec);

    /**
     * @brief Starts registered software.
     */
    bool start_software(const SoftwareId &id);

    /**
     * @brief Stops running software.
     */
    bool stop_software(const SoftwareId &id);

    /**
     * @brief Restarts registered software.
     */
    bool restart_software(const SoftwareId &id);

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
     * @brief Returns whether network connectivity is available.
     */
    [[nodiscard]] bool connectivity_available() const noexcept;

    /**
     * @brief Returns whether the Host is currently connected.
     */
    [[nodiscard]] bool connected() const noexcept;

  private:
    ControlServer &server_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_CLIENT_HPP
