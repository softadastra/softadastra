/**
 *
 *  @file SoftwareManager.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_SOFTWARE_MANAGER_HPP
#define SOFTADASTRA_SOFTWARE_SOFTWARE_MANAGER_HPP

#include "host/HostState.hpp"
#include "platform/Process.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <optional>

namespace softadastra
{
  /**
   * @brief Manages the infrastructure lifecycle of software known to a Host.
   *
   * SoftwareManager coordinates software registration and lifecycle state with
   * the process capability used to execute the software.
   *
   * It does not know how the software is implemented, which language it uses,
   * which protocols it exposes, or how its internal architecture is organized.
   *
   * A Process supplied to a lifecycle operation represents the execution handle
   * associated with that software. SoftwareManager controls that handle without
   * knowing how the platform created it.
   */
  class SoftwareManager
  {
  public:
    /**
     * @brief Creates a software manager for a Host state.
     *
     * The HostState instance must remain valid for the lifetime of the manager.
     *
     * @param state Host infrastructure state managed by this instance.
     */
    explicit SoftwareManager(HostState &state) noexcept;

    /**
     * @brief Registers software with the Host.
     *
     * Registered software begins in the Stopped state.
     *
     * @param id Identifier of the software to register.
     *
     * @return true if the software was registered, otherwise false if the
     *         identifier is already known to the Host.
     */
    bool register_software(SoftwareId id);

    /**
     * @brief Starts registered software using a process execution handle.
     *
     * The software state changes to Starting before the process start attempt.
     * A successful start changes the state to Running. A failed start changes
     * the state to Failed.
     *
     * @param id Identifier of the software to start.
     * @param process Process execution handle associated with the software.
     *
     * @return true if the software started successfully, otherwise false.
     */
    bool start(const SoftwareId &id, Process &process);

    /**
     * @brief Stops registered software using a process execution handle.
     *
     * A successful stop changes the software state to Stopped. A failed stop
     * changes the software state to Failed.
     *
     * @param id Identifier of the software to stop.
     * @param process Process execution handle associated with the software.
     *
     * @return true if the software stopped successfully, otherwise false.
     */
    bool stop(const SoftwareId &id, Process &process);

    /**
     * @brief Returns the infrastructure lifecycle state of registered software.
     *
     * @param id Identifier of the software to inspect.
     *
     * @return Current software state, or std::nullopt if the identifier is not
     *         known to the Host.
     */
    [[nodiscard]] std::optional<SoftwareState> state(
        const SoftwareId &id) const noexcept;

  private:
    HostState &state_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_MANAGER_HPP
