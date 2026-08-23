/**
 *
 *  @file RemoteControlServer.hpp
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

#ifndef SOFTADASTRA_CONTROL_REMOTE_CONTROL_SERVER_HPP
#define SOFTADASTRA_CONTROL_REMOTE_CONTROL_SERVER_HPP

#include "control/ControlServer.hpp"
#include "control/RemoteAccessConfig.hpp"

#include <atomic>
#include <filesystem>
#include <string>
#include <thread>

namespace softadastra
{
  /** @brief Serves authenticated Host control only when locally enabled. */
  class RemoteControlServer
  {
  public:
    /** @brief Creates a TLS control listener managed by local configuration. */
    RemoteControlServer(ControlServer &server, RemoteAccessConfig &config,
                        std::string secret, std::filesystem::path certificate_directory) noexcept;

    ~RemoteControlServer();
    RemoteControlServer(const RemoteControlServer &) = delete;

    /** @brief Applies the persisted state, starting or stopping the listener. */
    [[nodiscard]] bool apply();

    /** @brief Closes the listener. */
    void stop() noexcept;

    /** @brief Returns whether the configured listener is accepting connections. */
    [[nodiscard]] bool listening() const noexcept;

  private:
    void run(RemoteAccessSettings settings) noexcept;
    [[nodiscard]] bool ensure_certificate() const;

    ControlServer &server_;
    RemoteAccessConfig &config_;
    std::string secret_;
    std::filesystem::path certificate_directory_;
    std::atomic_bool running_{false};
    std::atomic_bool listening_{false};
    int descriptor_{-1};
    std::thread thread_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_REMOTE_CONTROL_SERVER_HPP
