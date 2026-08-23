/**
 *
 *  @file Process.hpp
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

#ifndef SOFTADASTRA_PLATFORM_PROCESS_HPP
#define SOFTADASTRA_PLATFORM_PROCESS_HPP

namespace softadastra
{
  /**
   * @brief Represents a running operating-system process.
   *
   * Process is a handle to a process that has already been launched.
   * Process creation belongs to ProcessLauncher.
   *
   * The interface exposes only the lifecycle capabilities currently required
   * by Softadastra.
   */
  class Process
  {
  public:
    virtual ~Process() = default;

    /**
     * @brief Stops the process.
     *
     * @return true if the process was stopped successfully, otherwise false.
     */
    virtual bool stop() = 0;

    /**
     * @brief Returns whether the process is currently running.
     */
    [[nodiscard]] virtual bool is_running() const noexcept = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PROCESS_HPP
