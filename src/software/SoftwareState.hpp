/**
 *
 *  @file SoftwareState.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_SOFTWARE_STATE_HPP
#define SOFTADASTRA_SOFTWARE_SOFTWARE_STATE_HPP

namespace softadastra
{
  /**
   * @brief Represents the infrastructure lifecycle state of hosted software.
   *
   * SoftwareState describes only the state that a Softadastra Host needs in
   * order to manage software execution. It does not represent application
   * business state or any state defined internally by the hosted software.
   */
  enum class SoftwareState
  {
    /**
     * @brief The software is not currently running.
     */
    Stopped,

    /**
     * @brief The software is currently being started.
     */
    Starting,

    /**
     * @brief The software is running.
     */
    Running,

    /**
     * @brief The software failed to start or stopped because of a failure.
     */
    Failed
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_STATE_HPP
