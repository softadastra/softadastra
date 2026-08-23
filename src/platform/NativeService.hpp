/**
 *
 *  @file NativeService.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_SERVICE_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_SERVICE_HPP

#include "platform/Service.hpp"

#include <string_view>

namespace softadastra
{
  /**
   * @brief Controls the native Softadastra system service.
   *
   * NativeService represents the operating-system service responsible for
   * running the Softadastra Host independently from an interactive terminal.
   *
   * This class controls an existing service installation. Service installation,
   * removal, and startup policy are intentionally outside the current contract.
   */
  class NativeService final : public Service
  {
  public:
    /**
     * @brief Returns the canonical native service name.
     */
    [[nodiscard]] static constexpr std::string_view name() noexcept
    {
      return "softadastra";
    }

    /**
     * @brief Requests startup of the Softadastra system service.
     *
     * @return true when the service is already running or the operating system
     *         accepts the startup request, otherwise false.
     */
    bool start() override;

    /**
     * @brief Requests shutdown of the Softadastra system service.
     *
     * @return true when the service is already stopped or the operating system
     *         accepts the shutdown request, otherwise false.
     */
    bool stop() override;

    /**
     * @brief Returns whether the Softadastra system service is running.
     */
    [[nodiscard]] bool is_running() const noexcept override;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_SERVICE_HPP
