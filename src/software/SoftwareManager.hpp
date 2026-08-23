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
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace softadastra
{
  /**
   * @brief Manages software registered with a Host.
   *
   * SoftwareManager owns the runtime association between registered software
   * and the operating-system processes launched for it.
   *
   * Process creation is delegated to ProcessLauncher so that software
   * management remains independent of the underlying operating system.
   */
  class SoftwareManager
  {
  public:
    /**
     * @brief Creates a software manager.
     *
     * @param state Host-owned software state.
     * @param process_launcher Platform capability used to launch processes.
     */
    SoftwareManager(
        HostState &state,
        ProcessLauncher &process_launcher) noexcept;

    /**
     * @brief Registers software with the Host.
     *
     * @param id Software identifier.
     * @param process_spec Information required to launch the software.
     *
     * @return true if the software was registered, otherwise false.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec);

    /**
     * @brief Starts registered software.
     *
     * @param id Identifier of the software to start.
     *
     * @return true if the software was started, otherwise false.
     */
    bool start(const SoftwareId &id);

    /**
     * @brief Stops running software.
     *
     * @param id Identifier of the software to stop.
     *
     * @return true if the software was stopped, otherwise false.
     */
    bool stop(const SoftwareId &id);

    /**
     * @brief Returns the lifecycle state of registered software.
     *
     * @param id Software identifier.
     *
     * @return The lifecycle state, or std::nullopt when the software is not
     *         registered.
     */
    [[nodiscard]] std::optional<SoftwareState> state(
        const SoftwareId &id) const noexcept;

  private:
    struct ManagedProcess
    {
      SoftwareId id;
      std::unique_ptr<Process> process;
    };

    [[nodiscard]] ManagedProcess *find_process(
        const SoftwareId &id) noexcept;

    [[nodiscard]] const ManagedProcess *find_process(
        const SoftwareId &id) const noexcept;

    HostState &state_;
    ProcessLauncher &process_launcher_;
    std::vector<ManagedProcess> processes_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_MANAGER_HPP
