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
#include "software/ProjectIdentity.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareOperation.hpp"
#include "software/SoftwareState.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace softadastra
{
  /**
   * @brief Represents software registered with a Host.
   *
   * SoftwareEntry stores Host-owned information required to identify,
   * launch, access, and track registered software.
   *
   * It does not describe the software language, framework, database,
   * or application architecture.
   */
  class SoftwareEntry
  {
  public:
    /**
     * @brief Creates a software entry with an optional access point.
     *
     * @param id Stable identifier used by the Host.
     * @param process_spec Process specification used to launch the software.
     * @param project_identity Optional identity of the associated project.
     * @param access_point Optional access point exposed by the software.
     * @param declared_command Original command declared for the software.
     * @param name Human-readable software name.
     */
    SoftwareEntry(
        SoftwareId id,
        ProcessSpec process_spec,
        std::optional<ProjectIdentity> project_identity = std::nullopt,
        std::optional<AccessPoint> access_point = std::nullopt,
        std::string declared_command = {},
        std::string name = {})
        : id_(std::move(id)),
          name_(std::move(name)),
          process_spec_(std::move(process_spec)),
          project_identity_(std::move(project_identity)),
          declared_command_(std::move(declared_command))
    {
      if (access_point)
      {
        access_points_.push_back(*access_point);
      }
    }

    /**
     * @brief Creates a software entry with multiple access points.
     *
     * @param id Stable identifier used by the Host.
     * @param process_spec Process specification used to launch the software.
     * @param project_identity Optional identity of the associated project.
     * @param access_points Access points exposed by the software.
     * @param declared_command Original command declared for the software.
     * @param name Human-readable software name.
     */
    SoftwareEntry(
        SoftwareId id,
        ProcessSpec process_spec,
        std::optional<ProjectIdentity> project_identity,
        std::vector<AccessPoint> access_points,
        std::string declared_command = {},
        std::string name = {})
        : id_(std::move(id)),
          name_(std::move(name)),
          process_spec_(std::move(process_spec)),
          project_identity_(std::move(project_identity)),
          access_points_(std::move(access_points)),
          declared_command_(std::move(declared_command))
    {
    }

    /**
     * @brief Creates a software entry with a single optional access point.
     *
     * @param id Stable identifier used by the Host.
     * @param process_spec Process specification used to launch the software.
     * @param access_point Optional access point exposed by the software.
     */
    SoftwareEntry(
        SoftwareId id,
        ProcessSpec process_spec,
        std::optional<AccessPoint> access_point)
        : SoftwareEntry(
              std::move(id),
              std::move(process_spec),
              std::nullopt,
              access_point)
    {
    }

    /**
     * @brief Returns the software identifier.
     *
     * @return Stable software identifier.
     */
    [[nodiscard]] const SoftwareId &id() const noexcept
    {
      return id_;
    }

    /**
     * @brief Returns the software name.
     *
     * @return Human-readable software name.
     */
    [[nodiscard]] const std::string &name() const noexcept
    {
      return name_;
    }

    /**
     * @brief Updates the software name.
     *
     * @param name New human-readable software name.
     */
    void set_name(std::string name)
    {
      name_ = std::move(name);
    }

    /**
     * @brief Returns the process launch specification.
     *
     * @return Process specification associated with the software.
     */
    [[nodiscard]] const ProcessSpec &process_spec() const noexcept
    {
      return process_spec_;
    }

    /**
     * @brief Returns the associated project identity.
     *
     * @return Project identity when available.
     */
    [[nodiscard]] const std::optional<ProjectIdentity> &
    project_identity() const noexcept
    {
      return project_identity_;
    }

    /**
     * @brief Updates the process working directory.
     *
     * @param working_directory New working directory.
     */
    void set_working_directory(std::string working_directory)
    {
      process_spec_ = ProcessSpec(
          process_spec_.executable(),
          process_spec_.arguments(),
          std::move(working_directory));
    }

    /**
     * @brief Associates the software with a project identity.
     *
     * @param identity Project identity to associate with the software.
     */
    void set_project_identity(ProjectIdentity identity)
    {
      project_identity_ = std::move(identity);
    }

    /**
     * @brief Replaces the process launch specification.
     *
     * @param process_spec New process specification.
     */
    void set_process_spec(ProcessSpec process_spec)
    {
      process_spec_ = std::move(process_spec);
    }

    /**
     * @brief Returns the command originally declared for the software.
     *
     * @return Declared command.
     */
    [[nodiscard]] const std::string &declared_command() const noexcept
    {
      return declared_command_;
    }

    /**
     * @brief Updates the declared software command.
     *
     * @param command New declared command.
     */
    void set_declared_command(std::string command)
    {
      declared_command_ = std::move(command);
    }

    /**
     * @brief Returns the process identifier associated with the software.
     *
     * @return Process identifier when available.
     */
    [[nodiscard]] std::optional<long> pid() const noexcept
    {
      return pid_;
    }

    /**
     * @brief Updates the process identifier associated with the software.
     *
     * @param pid Process identifier, or std::nullopt when none is associated.
     */
    void set_pid(std::optional<long> pid) noexcept
    {
      pid_ = pid;
    }

    /**
     * @brief Replaces the current access points with a single optional access point.
     *
     * @param access_point Access point to associate with the software.
     */
    void set_access_point(std::optional<AccessPoint> access_point)
    {
      access_points_.clear();

      if (access_point)
      {
        access_points_.push_back(*access_point);
      }
    }

    /**
     * @brief Replaces the access points associated with the software.
     *
     * @param access_points New access points.
     */
    void set_access_points(std::vector<AccessPoint> access_points)
    {
      access_points_ = std::move(access_points);
    }

    /**
     * @brief Returns all access points associated with the software.
     *
     * @return Access points exposed by the software.
     */
    [[nodiscard]] const std::vector<AccessPoint> &
    access_points() const noexcept
    {
      return access_points_;
    }

    /**
     * @brief Returns the primary access point associated with the software.
     *
     * @return The first access point when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<AccessPoint> access_point() const noexcept
    {
      return access_points_.empty()
                 ? std::nullopt
                 : std::optional<AccessPoint>(access_points_.front());
    }

    /**
     * @brief Returns the current software lifecycle state.
     *
     * @return Current lifecycle state.
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
     * @brief Returns the last terminal lifecycle result.
     *
     * @return Last lifecycle result when available.
     */
    [[nodiscard]] std::optional<SoftwareOperationResult> result() const noexcept
    {
      return result_;
    }

    /**
     * @brief Records a terminal lifecycle result for the software.
     *
     * @param result Lifecycle result to record.
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
    std::vector<AccessPoint> access_points_;
    std::string declared_command_;
    std::optional<long> pid_;
    SoftwareState state_{SoftwareState::Stopped};
    std::optional<SoftwareOperationResult> result_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP
