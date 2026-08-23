/**
 *
 *  @file Service.hpp
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

#ifndef SOFTADASTRA_PLATFORM_SERVICE_HPP
#define SOFTADASTRA_PLATFORM_SERVICE_HPP

namespace softadastra
{
  /**
   * @brief Defines the system service operations required by a Host.
   *
   * Service represents the minimal platform capability needed by Softadastra
   * to control a long-running system service independently of an interactive
   * terminal session.
   *
   * This interface describes infrastructure managed by the Host. It does not
   * represent a service belonging to the internal architecture of hosted
   * software.
   */
  class Service
  {
  public:
    /**
     * @brief Destroys the service interface.
     */
    virtual ~Service() = default;

    /**
     * @brief Starts the service.
     *
     * @return true if the service was started successfully, otherwise false.
     */
    virtual bool start() = 0;

    /**
     * @brief Stops the service.
     *
     * @return true if the service was stopped successfully, otherwise false.
     */
    virtual bool stop() = 0;

    /**
     * @brief Checks whether the service is currently running.
     *
     * @return true if the service is running, otherwise false.
     */
    [[nodiscard]] virtual bool is_running() const noexcept = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_SERVICE_HPP
