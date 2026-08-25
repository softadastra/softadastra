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
#include "platform/NativeManagedNetwork.hpp"
#include "platform/NativeProcessLauncher.hpp"
#include "platform/NativeService.hpp"
#include "platform/Platform.hpp"

#include <string_view>

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
     * @brief Returns whether the complete native Host is supported.
     *
     * Linux is the first platform with a validated service model and local
     * control channel. Other platform primitives may compile independently,
     * but do not represent a supported Host until their complete lifecycle is
     * validated.
     */
    [[nodiscard]] static constexpr bool host_supported() noexcept
    {
#if defined(__linux__) || defined(_WIN32)
      return true;
#else
      return false;
#endif
    }

    /**
     * @brief Returns the diagnostic for an unsupported native Host.
     */
    [[nodiscard]] static constexpr std::string_view
    host_support_diagnostic() noexcept
    {
      return "Softadastra Host is unsupported on this platform";
    }

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
    [[nodiscard]] ManagedNetwork &managed_network() noexcept override;
    [[nodiscard]] const ManagedNetwork &managed_network() const noexcept override;

  private:
    NativeProcessLauncher process_launcher_;
    NativeService service_;
    NativeNetwork network_;
    NativeManagedNetwork managed_network_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_PLATFORM_HPP
