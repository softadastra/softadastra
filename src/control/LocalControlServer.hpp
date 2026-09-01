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
#include <utility>

namespace softadastra
{
  class RemoteAccessConfig;
  class RemoteReachability;

  /**
   * @brief Serves Host control operations through a local platform channel.
   *
   * LocalControlServer exposes a ControlServer through a platform-specific
   * local endpoint and processes requests using the shared Host control
   * protocol.
   */
  class LocalControlServer
  {
  public:
    /**
     * @brief Creates a local control server for a Host control surface.
     *
     * @param server Control server that handles Host operations.
     * @param path Filesystem path identifying the local control endpoint.
     * @param remote_config Optional remote access configuration.
     * @param remote_reachability Optional remote reachability service.
     */
    LocalControlServer(
        ControlServer &server,
        std::filesystem::path path,
        RemoteAccessConfig *remote_config = nullptr,
        RemoteReachability *remote_reachability = nullptr) noexcept;

    /**
     * @brief Destroys the local control server.
     */
    ~LocalControlServer();

    LocalControlServer(const LocalControlServer &) = delete;
    LocalControlServer &operator=(const LocalControlServer &) = delete;

    /**
     * @brief Opens the local control endpoint.
     *
     * @return true if the endpoint was opened successfully, otherwise false.
     */
    [[nodiscard]] bool start();

    /**
     * @brief Processes all currently pending local control requests.
     *
     * @return true if pending requests were processed successfully, otherwise false.
     */
    [[nodiscard]] bool process_pending();

    /**
     * @brief Closes the local control endpoint.
     */
    void stop() noexcept;

    /**
     * @brief Sets the callback invoked when a shutdown request is received.
     *
     * @param handler Callback to invoke for a shutdown request.
     */
    void set_shutdown_handler(std::function<void()> handler)
    {
      shutdown_handler_ = std::move(handler);
    }

    /**
     * @brief Sets the callback that checkpoints Host state before a lifecycle
     *        response is returned to a client.
     */
    void set_state_persistence_handler(std::function<bool()> handler)
    {
      state_persistence_handler_ = std::move(handler);
    }

    /**
     * @brief Executes a request using the shared Host control protocol.
     *
     * @param server Control server that handles the request.
     * @param request Protocol request to execute.
     *
     * @return Protocol response produced by the control server.
     */
    [[nodiscard]] static std::string handle(
        ControlServer &server,
        std::string_view request);

  private:
    ControlServer &server_;
    std::filesystem::path path_;
    int descriptor_{-1};

#if defined(_WIN32)
    void *pipe_{nullptr};
#endif

    RemoteAccessConfig *remote_config_;
    RemoteReachability *remote_reachability_;
    std::function<void()> shutdown_handler_;
    std::function<bool()> state_persistence_handler_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_LOCAL_CONTROL_SERVER_HPP
