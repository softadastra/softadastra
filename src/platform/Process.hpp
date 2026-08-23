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

#include <optional>

namespace softadastra
{

  /**
   * @brief Represents a process launched by the operating system.
   */
  class Process
  {
  public:
    virtual ~Process() = default;

    /**
     * @brief Stops the process.
     *
     * @return true if the process is stopped successfully, otherwise false.
     */
    virtual bool stop() = 0;

    /**
     * @brief Returns whether the process is currently running.
     */
    [[nodiscard]] virtual bool is_running() const noexcept = 0;

    /**
     * @brief Returns the process exit code when available.
     *
     * @return The exit code when the process has terminated, or std::nullopt
     *         while the process is still running.
     */
    [[nodiscard]] virtual std::optional<int> exit_code() noexcept = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PROCESS_HPP
