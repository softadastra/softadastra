/**
 *
 *  @file Platform.hpp
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

#ifndef SOFTADASTRA_PLATFORM_PLATFORM_HPP
#define SOFTADASTRA_PLATFORM_PLATFORM_HPP

#include "platform/Network.hpp"
#include "platform/Process.hpp"
#include "platform/Service.hpp"

namespace softadastra
{
  /**
   * @brief Defines the machine-level capabilities available to a Host.
   *
   * Platform provides a single abstraction boundary between Softadastra and
   * platform-specific infrastructure. Higher-level Host components use this
   * interface instead of depending directly on a particular operating system.
   *
   * Operating-system support is an implementation capability. The Platform
   * abstraction itself does not identify a Host as Linux, Windows, macOS, or
   * any other specific system.
   */
  class Platform
  {
  public:
    /**
     * @brief Destroys the platform interface.
     */
    virtual ~Platform() = default;

    /**
     * @brief Returns the process capability provided by the platform.
     *
     * @return Reference to the platform process capability.
     */
    [[nodiscard]] virtual Process &process() noexcept = 0;

    /**
     * @brief Returns the process capability provided by the platform.
     *
     * @return Constant reference to the platform process capability.
     */
    [[nodiscard]] virtual const Process &process() const noexcept = 0;

    /**
     * @brief Returns the system service capability provided by the platform.
     *
     * @return Reference to the platform service capability.
     */
    [[nodiscard]] virtual Service &service() noexcept = 0;

    /**
     * @brief Returns the system service capability provided by the platform.
     *
     * @return Constant reference to the platform service capability.
     */
    [[nodiscard]] virtual const Service &service() const noexcept = 0;

    /**
     * @brief Returns the network capability provided by the platform.
     *
     * @return Reference to the platform network capability.
     */
    [[nodiscard]] virtual Network &network() noexcept = 0;

    /**
     * @brief Returns the network capability provided by the platform.
     *
     * @return Constant reference to the platform network capability.
     */
    [[nodiscard]] virtual const Network &network() const noexcept = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PLATFORM_HPP
