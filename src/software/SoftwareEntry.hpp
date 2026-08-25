/**
 *
 *  @file SoftwareEntry.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP
#define SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP

#include "platform/ProcessSpec.hpp"
#include "software/AccessPoint.hpp"
#include "software/SoftwareId.hpp"
#include "software/ProjectIdentity.hpp"
#include "software/SoftwareOperation.hpp"
#include "software/SoftwareState.hpp"

#include <optional>
#include <string>
#include <utility>

namespace softadastra
{
  /**
   * @brief Represents software registered with a Host.
   *
   * SoftwareEntry stores only Host-owned infrastructure information required
   * to identify, launch, and track software.
   *
   * It does not describe the software language, framework, protocol, database,
   * or application architecture.
   */
  class SoftwareEntry
  {
  public:
    /**
     * @brief Creates a software entry.
     *
     * @param id Stable identifier used by the Host.
     * @param process_spec Information required to launch the software process.
     */
    SoftwareEntry(SoftwareId id, ProcessSpec process_spec,
                  std::optional<ProjectIdentity> project_identity = std::nullopt,
                  std::optional<AccessPoint> access_point = std::nullopt,
                  std::string declared_command = {}, std::string name = {})
        : id_(std::move(id)),
          name_(std::move(name)),
          process_spec_(std::move(process_spec)), project_identity_(std::move(project_identity)),
          access_point_(access_point), declared_command_(std::move(declared_command))
    {
    }

    SoftwareEntry(SoftwareId id, ProcessSpec process_spec, std::optional<AccessPoint> access_point)
        : SoftwareEntry(std::move(id), std::move(process_spec), std::nullopt, access_point) {}

    /**
     * @brief Returns the software identifier.
     */
    [[nodiscard]] const SoftwareId &id() const noexcept
    {
      return id_;
    }

    [[nodiscard]] const std::string &name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    /**
     * @brief Returns the process launch specification.
     */
    [[nodiscard]] const ProcessSpec &process_spec() const noexcept
    {
      return process_spec_;
    }

    [[nodiscard]] const std::optional<ProjectIdentity> &project_identity() const noexcept
    { return project_identity_; }

    void set_working_directory(std::string working_directory)
    { process_spec_ = ProcessSpec(process_spec_.executable(), process_spec_.arguments(), std::move(working_directory)); }

    void set_project_identity(ProjectIdentity identity)
    { project_identity_ = std::move(identity); }

    void set_process_spec(ProcessSpec process_spec) { process_spec_ = std::move(process_spec); }
    [[nodiscard]] const std::string &declared_command() const noexcept { return declared_command_; }
    void set_declared_command(std::string command) { declared_command_ = std::move(command); }
    [[nodiscard]] std::optional<long> pid() const noexcept { return pid_; }
    void set_pid(std::optional<long> pid) noexcept { pid_ = pid; }
    void set_access_point(std::optional<AccessPoint> access_point) { access_point_ = access_point; }

    [[nodiscard]] std::optional<AccessPoint> access_point() const noexcept
    {
      return access_point_;
    }

    /**
     * @brief Returns the current software lifecycle state.
     */
    [[nodiscard]] SoftwareState state() const noexcept
    {
      return state_;
    }

    /**
     * @brief Updates the software lifecycle state.
     *
     * @param state New lifecycle state.
     */
    void set_state(SoftwareState state) noexcept
    {
      state_ = state;
    }

    /**
     * @brief Returns the last terminal lifecycle result, when available.
     */
    [[nodiscard]] std::optional<SoftwareOperationResult> result() const noexcept
    {
      return result_;
    }

    /**
     * @brief Records a terminal lifecycle result for the software.
     */
    void set_result(SoftwareOperationResult result) noexcept
    {
      result_ = result;
    }

    /**
     * @brief Clears the last terminal lifecycle result.
     */
    void clear_result() noexcept
    {
      result_.reset();
    }

  private:
    SoftwareId id_;
    std::string name_;
    ProcessSpec process_spec_;
    std::optional<ProjectIdentity> project_identity_;
    std::optional<AccessPoint> access_point_;
    std::string declared_command_;
    std::optional<long> pid_;
    SoftwareState state_{SoftwareState::Stopped};
    std::optional<SoftwareOperationResult> result_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP
