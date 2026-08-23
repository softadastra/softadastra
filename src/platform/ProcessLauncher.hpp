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

#include <memory>

namespace softadastra
{
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
     * @return A process handle when launch succeeds, or nullptr when launch
     *         fails.
     */
    [[nodiscard]] virtual std::unique_ptr<Process> launch(
        const ProcessSpec &spec) = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PROCESS_LAUNCHER_HPP
