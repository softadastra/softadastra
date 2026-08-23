/**
 *
 *  @file ProcessLauncher.hpp
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

#ifndef SOFTADASTRA_PLATFORM_PROCESS_LAUNCHER_HPP
#define SOFTADASTRA_PLATFORM_PROCESS_LAUNCHER_HPP

#include "platform/Process.hpp"
#include "platform/ProcessSpec.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace softadastra
{
  /**
   * @brief Describes a platform-level process launch failure.
   */
  enum class ProcessLaunchError
  {
    ExecutableNotFound,
    PermissionDenied,
    LaunchFailed
  };

  /**
   * @brief Reports the outcome of a process launch attempt.
   */
  class ProcessLaunchResult
  {
  public:
    /**
     * @brief Creates a successful result from a process handle.
     */
    template <typename ProcessType>
    ProcessLaunchResult(std::unique_ptr<ProcessType> process) noexcept
        : process_(std::move(process))
    {
      static_assert(
          std::is_base_of_v<Process, ProcessType>,
          "ProcessType must derive from Process.");
    }

    /**
     * @brief Creates a generic failed launch result.
     */
    ProcessLaunchResult(std::nullptr_t) noexcept
        : error_(ProcessLaunchError::LaunchFailed)
    {
    }

    /**
     * @brief Creates a failed result with a platform launch error.
     */
    ProcessLaunchResult(ProcessLaunchError error) noexcept
        : error_(error)
    {
    }

    /**
     * @brief Returns whether a process was launched.
     */
    [[nodiscard]] bool succeeded() const noexcept
    {
      return process_ != nullptr;
    }

    /**
     * @brief Converts the result to its success state.
     */
    [[nodiscard]] explicit operator bool() const noexcept
    {
      return succeeded();
    }

    /**
     * @brief Returns the launched process handle.
     */
    [[nodiscard]] Process *operator->() const noexcept
    {
      return process_.get();
    }

    /**
     * @brief Returns the launched process.
     */
    [[nodiscard]] Process &operator*() const noexcept
    {
      return *process_;
    }

    /**
     * @brief Compares the result to an absent process handle.
     */
    [[nodiscard]] bool operator==(std::nullptr_t) const noexcept
    {
      return !succeeded();
    }

    /**
     * @brief Returns the launch failure cause.
     */
    [[nodiscard]] std::optional<ProcessLaunchError> error() const noexcept
    {
      return error_;
    }

    /**
     * @brief Moves the launched process handle out of the result.
     */
    [[nodiscard]] std::unique_ptr<Process> take_process() && noexcept
    {
      return std::move(process_);
    }

  private:
    std::unique_ptr<Process> process_;
    std::optional<ProcessLaunchError> error_;
  };

  /**
   * @brief Launches operating-system processes.
   *
   * ProcessLauncher separates process creation from the lifecycle of an
   * individual running process.
   *
   * Platform-specific implementations are responsible for translating a
   * ProcessSpec into a real operating-system process.
   */
  class ProcessLauncher
  {
  public:
    virtual ~ProcessLauncher() = default;

    /**
     * @brief Launches a process from the provided specification.
     *
     * @param spec Infrastructure information required to launch the process.
     *
     * @return A process handle or a stable platform launch failure.
     */
    [[nodiscard]] virtual ProcessLaunchResult launch(
        const ProcessSpec &spec) = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PROCESS_LAUNCHER_HPP
