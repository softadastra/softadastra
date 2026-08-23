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

#include <filesystem>
#include <string>
#include <string_view>

namespace softadastra
{
  /**
   * @brief Controls the native Softadastra system service.
   *
   * NativeService represents the operating-system service responsible for
   * running the Softadastra Host independently from an interactive terminal.
   *
   * On Linux, it manages the systemd unit used to run the Host.
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

#if defined(__linux__)
    /**
     * @brief Returns the systemd unit path used by Softadastra.
     */
    [[nodiscard]] static std::filesystem::path unit_file_path();

    /**
     * @brief Generates the systemd unit for a Host executable.
     *
     * @param executable Path to the Softadastra executable.
     *
     * @return Complete systemd unit content.
     */
    [[nodiscard]] static std::string unit_file_content(
        const std::filesystem::path &executable);

    /**
     * @brief Installs the systemd unit for a Host executable.
     *
     * This operation writes to the native system service directory and may
     * require administrative privileges.
     *
     * @return true when the unit is installed and systemd reloads it.
     */
    [[nodiscard]] bool install(
        const std::filesystem::path &executable);

    /**
     * @brief Enables automatic startup through systemd.
     *
     * @return true when systemd enables the installed service, otherwise
     *         false.
     */
    [[nodiscard]] bool enable_auto_start();
#endif

    /**
     * @brief Returns whether the native Softadastra service is installed.
     */
    [[nodiscard]] bool is_installed() const noexcept;

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
