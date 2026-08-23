/**
 *
 *  @file NativeProcess.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_PROCESS_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_PROCESS_HPP

#include "platform/Process.hpp"

#include <cstdint>

namespace softadastra
{
  /**
   * @brief Represents a process running on the native operating system.
   *
   * NativeProcess owns the infrastructure handle needed by Softadastra to
   * inspect and stop a process that has already been launched.
   *
   * Process creation remains the responsibility of ProcessLauncher.
   */
  class NativeProcess final : public Process
  {
  public:
    using Id = std::uint64_t;

    /**
     * @brief Creates a native process handle.
     *
     * @param id Operating-system process identifier.
     */
    explicit NativeProcess(Id id) noexcept;

    /**
     * @brief Requests termination of the process.
     *
     * @return true if the process is already stopped or the termination request
     *         succeeds, otherwise false.
     */
    bool stop() override;

    /**
     * @brief Returns whether the process currently exists.
     */
    [[nodiscard]] bool is_running() const noexcept override;

    /**
     * @brief Returns the operating-system process identifier.
     */
    [[nodiscard]] Id id() const noexcept;

  private:
    Id id_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_PROCESS_HPP
