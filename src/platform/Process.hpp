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
   * @brief Defines the process lifecycle operations required by a Host.
   *
   * Process represents the minimal operating-system process capability needed
   * by Softadastra. It allows higher-level Host components to start, stop, and
   * inspect a process without depending directly on a particular operating
   * system or process implementation.
   *
   * This interface does not describe the architecture, language, protocol, or
   * internal behavior of the software executed by the process.
   */
  class Process
  {
  public:
    /**
     * @brief Destroys the process interface.
     */
    virtual ~Process() = default;

    /**
     * @brief Starts the process.
     *
     * @return true if the process was started successfully, otherwise false.
     */
    virtual bool start() = 0;

    /**
     * @brief Stops the process.
     *
     * @return true if the process was stopped successfully, otherwise false.
     */
    virtual bool stop() = 0;

    /**
     * @brief Checks whether the process is currently running.
     *
     * @return true if the process is running, otherwise false.
     */
    [[nodiscard]] virtual bool is_running() const noexcept = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PROCESS_HPP
