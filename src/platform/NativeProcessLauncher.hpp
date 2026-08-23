/**
 *
 *  @file NativeProcessLauncher.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_PROCESS_LAUNCHER_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_PROCESS_LAUNCHER_HPP

#include "platform/ProcessLauncher.hpp"

namespace softadastra
{
  /**
   * @brief Launches processes on the native operating system.
   *
   * NativeProcessLauncher translates Softadastra ProcessSpec values into
   * Vix.cpp process commands and launches them through the Vix process module.
   *
   * Hosted software remains independent from Vix.cpp. Vix.cpp is used only as
   * an internal Softadastra platform implementation detail.
   */
  class NativeProcessLauncher final : public ProcessLauncher
  {
  public:
    /**
     * @brief Launches a native process.
     *
     * @param spec Infrastructure information required to launch the process.
     *
     * @return A NativeProcess handle or a stable platform launch failure.
     */
    [[nodiscard]] ProcessLaunchResult launch(
        const ProcessSpec &spec) override;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_PROCESS_LAUNCHER_HPP
