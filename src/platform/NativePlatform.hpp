/**
 *
 *  @file NativePlatform.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_PLATFORM_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_PLATFORM_HPP

#include "platform/NativeNetwork.hpp"
#include "platform/NativeProcessLauncher.hpp"
#include "platform/NativeService.hpp"
#include "platform/Platform.hpp"

namespace softadastra
{
  /**
   * @brief Provides Softadastra infrastructure capabilities for the native
   * operating system.
   *
   * NativePlatform composes the concrete process, service, and network
   * implementations used by a Host running on the current machine.
   *
   * The Host depends only on the Platform contract and remains independent from
   * these native implementation details.
   */
  class NativePlatform final : public Platform
  {
  public:
    /**
     * @brief Returns the native process launching capability.
     */
    [[nodiscard]] ProcessLauncher &process_launcher() noexcept override;

    /**
     * @brief Returns the native process launching capability.
     */
    [[nodiscard]] const ProcessLauncher &process_launcher()
        const noexcept override;

    /**
     * @brief Returns the native system service capability.
     */
    [[nodiscard]] Service &service() noexcept override;

    /**
     * @brief Returns the native system service capability.
     */
    [[nodiscard]] const Service &service() const noexcept override;

    /**
     * @brief Returns the native network capability.
     */
    [[nodiscard]] Network &network() noexcept override;

    /**
     * @brief Returns the native network capability.
     */
    [[nodiscard]] const Network &network() const noexcept override;

  private:
    NativeProcessLauncher process_launcher_;
    NativeService service_;
    NativeNetwork network_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_PLATFORM_HPP
