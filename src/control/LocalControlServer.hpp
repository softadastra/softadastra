/**
 *
 *  @file LocalControlServer.hpp
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

#ifndef SOFTADASTRA_CONTROL_LOCAL_CONTROL_SERVER_HPP
#define SOFTADASTRA_CONTROL_LOCAL_CONTROL_SERVER_HPP

#include "control/ControlServer.hpp"

#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace softadastra
{
  class RemoteAccessConfig;
  class RemoteControlServer;
  /**
   * @brief Serves Host control operations through a local platform channel.
   */
  class LocalControlServer
  {
  public:
    /**
     * @brief Creates a local control server for a Host control surface.
     */
    LocalControlServer(
        ControlServer &server,
        std::filesystem::path path,
        RemoteAccessConfig *remote_config = nullptr,
        RemoteControlServer *remote_server = nullptr) noexcept;

    ~LocalControlServer();

    LocalControlServer(const LocalControlServer &) = delete;
    LocalControlServer &operator=(const LocalControlServer &) = delete;

    /**
     * @brief Opens the local control endpoint.
     */
    [[nodiscard]] bool start();

    /**
     * @brief Serves all currently pending local control requests.
     */
    [[nodiscard]] bool process_pending();

    /**
     * @brief Closes the local control endpoint.
     */
    void stop() noexcept;
    void set_shutdown_handler(std::function<void()> handler) { shutdown_handler_ = std::move(handler); }

    /** Executes the shared Host control protocol. */
    [[nodiscard]] static std::string handle(ControlServer &server, std::string_view request);

  private:
    ControlServer &server_;
    std::filesystem::path path_;
    int descriptor_{-1};
#if defined(_WIN32)
    HANDLE pipe_{INVALID_HANDLE_VALUE};
#endif
    RemoteAccessConfig *remote_config_;
    RemoteControlServer *remote_server_;
    std::function<void()> shutdown_handler_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_LOCAL_CONTROL_SERVER_HPP
