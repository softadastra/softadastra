/**
 *
 *  @file SoftwareOperation.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_SOFTWARE_OPERATION_HPP
#define SOFTADASTRA_SOFTWARE_SOFTWARE_OPERATION_HPP

#include <optional>

namespace softadastra
{
  /**
   * @brief Describes a stable failure cause for a software lifecycle operation.
   */
  enum class SoftwareOperationError
  {
    SoftwareUnknown,
    AlreadyRunning,
    NotRunning,
    ExecutableNotFound,
    PermissionDenied,
    LaunchFailed,
    ProcessExitedSuccessfully,
    ProcessExitedWithNonZeroCode,
    StopFailed
  };

  /**
   * @brief Reports the outcome of a software lifecycle operation.
   *
   * The result contains a stable Softadastra failure cause instead of errors
   * from the underlying platform implementation.
   */
  class SoftwareOperationResult
  {
  public:
    /**
     * @brief Creates a successful operation result.
     */
    SoftwareOperationResult() noexcept = default;

    /**
     * @brief Creates a failed operation result.
     *
     * @param error Stable lifecycle failure cause.
     * @param exit_code Process exit code when applicable.
     */
    SoftwareOperationResult(
        SoftwareOperationError error,
        std::optional<int> exit_code = std::nullopt) noexcept
        : error_(error),
          exit_code_(exit_code)
    {
    }

    /**
     * @brief Returns whether the operation succeeded.
     */
    [[nodiscard]] bool succeeded() const noexcept
    {
      return !error_.has_value();
    }

    /**
     * @brief Converts the result to its success state.
     */
    [[nodiscard]] explicit operator bool() const noexcept
    {
      return succeeded();
    }

    /**
     * @brief Returns the failure cause when the operation failed.
     */
    [[nodiscard]] std::optional<SoftwareOperationError> error() const noexcept
    {
      return error_;
    }

    /**
     * @brief Returns the process exit code when available.
     */
    [[nodiscard]] std::optional<int> exit_code() const noexcept
    {
      return exit_code_;
    }

  private:
    std::optional<SoftwareOperationError> error_;
    std::optional<int> exit_code_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_OPERATION_HPP
