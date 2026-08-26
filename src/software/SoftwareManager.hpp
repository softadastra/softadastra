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
#include "software/AccessPoint.hpp"
#include "software/SoftwareOperation.hpp"
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
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point = std::nullopt,
        std::optional<ProjectIdentity> project_identity = std::nullopt, std::string name = {});

    [[nodiscard]] std::optional<SoftwareEntry> software_by_project_identity(
        const ProjectIdentity &identity) const noexcept;
    bool update_project_root(const ProjectIdentity &identity, std::string root);
    [[nodiscard]] bool synchronize(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point, std::string name = {});
    [[nodiscard]] bool synchronize(const SoftwareId &id, ProcessSpec process_spec, std::vector<AccessPoint> access_points, std::string name = {});
    [[nodiscard]] std::optional<SoftwareEntry> find_by_name(const std::string &name) const noexcept;

    [[nodiscard]] std::optional<AccessPoint> access_point(const SoftwareId &id) const noexcept;
    [[nodiscard]] std::vector<SoftwareEntry> software() const;
    [[nodiscard]] bool remove(const SoftwareId &id);

    /**
     * @brief Starts registered software.
     */
    [[nodiscard]] SoftwareOperationResult start(const SoftwareId &id);

    /**
     * @brief Stops running software.
     */
    [[nodiscard]] SoftwareOperationResult stop(const SoftwareId &id);

    /**
     * @brief Restarts registered software.
     *
     * A running managed process is stopped before a replacement is launched.
     * If no process is managed for the software, a new process is launched
     * directly.
     *
     * @return The result of stopping and launching the replacement process.
     */
    [[nodiscard]] SoftwareOperationResult restart(const SoftwareId &id);

    /**
     * @brief Stops every process managed by this manager.
     *
     * Registration metadata remains in HostState. Each process that cannot be
     * stopped remains associated with a Failed software entry carrying its
     * stop diagnostic.
     *
     * @return true when every managed process stops successfully.
     */
    [[nodiscard]] bool stop_all();

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

    /**
     * @brief Returns the last terminal lifecycle result for registered software.
     */
    [[nodiscard]] std::optional<SoftwareOperationResult> result(
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
