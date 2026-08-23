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
#include "platform/Process.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <optional>

namespace softadastra
{
  /**
   * @brief Provides the client-side interface to Host control operations.
   *
   * ControlClient represents the control interface used by higher-level
   * components such as the CLI to interact with a Softadastra Host.
   *
   * At this stage, ControlClient communicates directly with ControlServer.
   * This keeps the control contract independent from a specific transport.
   *
   * A future local IPC or other transport can be introduced between the client
   * and server without changing the Host or application architecture.
   */
  class ControlClient
  {
  public:
    /**
     * @brief Creates a control client connected to a control server.
     *
     * The ControlServer instance must remain valid for the lifetime of the
     * ControlClient.
     *
     * @param server Control server used by this client.
     */
    explicit ControlClient(ControlServer &server) noexcept;

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
    ControlServer &server_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_CONTROL_CLIENT_HPP
