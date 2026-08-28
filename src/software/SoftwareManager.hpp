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
#include "software/AccessPoint.hpp"
#include "software/SoftwareId.hpp"
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
   * SoftwareManager maintains the runtime association between registered
   * software and the processes launched for it.
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
     * @param id Identifier assigned to the software.
     * @param process_spec Process specification used to launch the software.
     * @param access_point Optional access point exposed by the software.
     * @param project_identity Optional identity of the associated project.
     * @param name Optional human-readable software name.
     *
     * @return true if the software was registered successfully, otherwise false.
     */
    bool register_software(
        SoftwareId id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point = std::nullopt,
        std::optional<ProjectIdentity> project_identity = std::nullopt,
        std::string name = {});

    /**
     * @brief Finds software associated with a project identity.
     *
     * @param identity Project identity to look up.
     *
     * @return The matching software entry, or std::nullopt if none is found.
     */
    [[nodiscard]] std::optional<SoftwareEntry> software_by_project_identity(
        const ProjectIdentity &identity) const noexcept;

    /**
     * @brief Updates the working directory associated with a project.
     *
     * @param identity Identity of the project to update.
     * @param root New project root directory.
     *
     * @return true if the project was found and updated, otherwise false.
     */
    bool update_project_root(
        const ProjectIdentity &identity,
        std::string root);

    /**
     * @brief Synchronizes a registered software definition.
     *
     * @param id Identifier of the software.
     * @param process_spec Current process specification.
     * @param access_point Optional access point exposed by the software.
     * @param name Optional human-readable software name.
     *
     * @return true if the software definition was updated, otherwise false.
     */
    [[nodiscard]] bool synchronize(
        const SoftwareId &id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point,
        std::string name = {});

    /**
     * @brief Synchronizes a registered software definition with multiple access points.
     *
     * @param id Identifier of the software.
     * @param process_spec Current process specification.
     * @param access_points Access points exposed by the software.
     * @param name Optional human-readable software name.
     *
     * @return true if the software definition was updated, otherwise false.
     */
    [[nodiscard]] bool synchronize(
        const SoftwareId &id,
        ProcessSpec process_spec,
        std::vector<AccessPoint> access_points,
        std::string name = {});

    /**
     * @brief Finds registered software by name.
     *
     * @param name Software name to look up.
     *
     * @return The matching software entry, or std::nullopt if none is found.
     */
    [[nodiscard]] std::optional<SoftwareEntry> find_by_name(
        const std::string &name) const noexcept;

    /**
     * @brief Returns the primary access point associated with software.
     *
     * @param id Identifier of the software.
     *
     * @return The access point when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<AccessPoint> access_point(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns all software registered with the Host.
     *
     * @return Registered software entries.
     */
    [[nodiscard]] std::vector<SoftwareEntry> software() const;

    /**
     * @brief Removes registered software.
     *
     * Running software is not removed.
     *
     * @param id Identifier of the software to remove.
     *
     * @return true if the software was removed successfully, otherwise false.
     */
    [[nodiscard]] bool remove(const SoftwareId &id);

    /**
     * @brief Starts registered software.
     *
     * @param id Identifier of the software to start.
     *
     * @return Result of the start operation.
     */
    [[nodiscard]] SoftwareOperationResult start(
        const SoftwareId &id);

    /**
     * @brief Stops running software.
     *
     * @param id Identifier of the software to stop.
     *
     * @return Result of the stop operation.
     */
    [[nodiscard]] SoftwareOperationResult stop(
        const SoftwareId &id);

    /**
     * @brief Restarts registered software.
     *
     * A running managed process is stopped before a replacement is launched.
     * If no process is managed for the software, a new process is launched
     * directly.
     *
     * @param id Identifier of the software to restart.
     *
     * @return Result of stopping and launching the replacement process.
     */
    [[nodiscard]] SoftwareOperationResult restart(
        const SoftwareId &id);

    /**
     * @brief Stops every process managed by this manager.
     *
     * Registration metadata remains in HostState. Each process that cannot
     * be stopped remains associated with a Failed software entry carrying
     * its stop diagnostic.
     *
     * @return true if every managed process stops successfully, otherwise false.
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
     *
     * @param id Identifier of the software.
     *
     * @return Current lifecycle state, or std::nullopt if the software is unknown.
     */
    [[nodiscard]] std::optional<SoftwareState> state(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Returns the last terminal lifecycle result for registered software.
     *
     * @param id Identifier of the software.
     *
     * @return Last lifecycle result when available, or std::nullopt otherwise.
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
