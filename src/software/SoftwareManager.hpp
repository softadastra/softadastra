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
   * and the processes launched for it.
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
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec);

    /**
     * @brief Starts registered software.
     */
    bool start(const SoftwareId &id);

    /**
     * @brief Stops running software.
     */
    bool stop(const SoftwareId &id);

    /**
     * @brief Restarts registered software.
     *
     * A running managed process is stopped before a replacement is launched.
     * If no process is managed for the software, a new process is launched
     * directly.
     *
     * @return true when the replacement process is running, otherwise false.
     */
    bool restart(const SoftwareId &id);

    /**
     * @brief Refreshes lifecycle state from managed processes.
     *
     * Running software remains Running while its process is active.
     * A process that exits with code 0 becomes Stopped.
     * A process that exits with a non-zero code becomes Failed.
     */
    void refresh();

    /**
     * @brief Returns the lifecycle state of registered software.
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
