/**
 *
 *  @file WebUiServer.hpp
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

#ifndef SOFTADASTRA_WEBUI_WEB_UI_SERVER_HPP
#define SOFTADASTRA_WEBUI_WEB_UI_SERVER_HPP

#include "control/ControlClient.hpp"
#include "platform/NativeDirectoryChooser.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

namespace softadastra
{
  /**
   * @brief Provides the loopback-only HTTP interface for the Softadastra Web UI.
   *
   * WebUiServer exposes Web UI operations through ControlClient and does not
   * access Host internals directly.
   */
  class WebUiServer
  {
  public:
    using ProjectDirectoryChooser =
        std::function<DirectoryChooserResult()>;

    /**
     * @brief Creates a Web UI server.
     *
     * @param client Control client used for Host operations.
     * @param directory_chooser Directory chooser used when selecting projects.
     */
    explicit WebUiServer(
        ControlClient &client,
        ProjectDirectoryChooser directory_chooser =
            choose_project_directory) noexcept;

    /**
     * @brief Stops the server and releases its resources.
     */
    ~WebUiServer();

    WebUiServer(const WebUiServer &) = delete;
    WebUiServer &operator=(const WebUiServer &) = delete;

    /**
     * @brief Starts the Web UI server on the loopback interface.
     *
     * @param port Requested TCP port. A value of zero allows the operating
     *        system to select an available port.
     *
     * @return true if the server started successfully, otherwise false.
     */
    [[nodiscard]] bool start(
        std::uint16_t port = 0);

    /**
     * @brief Stops the Web UI server.
     */
    void stop() noexcept;

    /**
     * @brief Returns the TCP port used by the running server.
     *
     * @return Bound TCP port, or zero when the server is not running.
     */
    [[nodiscard]] std::uint16_t port() const noexcept
    {
      return port_.load();
    }

  private:
    void run(
        std::uint16_t requested_port) noexcept;

    void publish_startup(
        bool success,
        std::uint16_t port) noexcept;

    ControlClient &client_;
    ProjectDirectoryChooser directory_chooser_;
    std::atomic_bool stopping_{false};
    std::atomic_uint16_t port_{0};
    std::mutex startup_mutex_;
    std::condition_variable startup_condition_;
    bool startup_complete_{false};
    bool startup_success_{false};
    std::thread worker_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_WEBUI_WEB_UI_SERVER_HPP
