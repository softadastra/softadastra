
/**
 *
 *  @file RelayServer.hpp
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

#ifndef SOFTADASTRA_RELAY_RELAY_SERVER_HPP
#define SOFTADASTRA_RELAY_RELAY_SERVER_HPP

#include "host/NativeSocket.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace softadastra
{
  /**
   * @brief Describes one TCP listener used by the relay.
   */
  struct RelayListenEndpoint
  {
    /**
     * @brief Address to bind.
     */
    std::string address{"0.0.0.0"};

    /**
     * @brief TCP port to bind. Zero lets the operating system choose a port.
     */
    std::uint16_t port{0};

    /**
     * @brief Checks whether the endpoint can be used.
     *
     * @return true if the endpoint contains a bind address, otherwise false.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return !address.empty();
    }
  };

  /**
   * @brief Describes the minimal relay server configuration.
   */
  struct RelayServerConfig
  {
    /**
     * @brief Listener used by Hosts to establish outbound relay connections.
     */
    RelayListenEndpoint host;

    /**
     * @brief Listener used by public HTTP clients.
     */
    RelayListenEndpoint http;

    /**
     * @brief Software name used when forwarding public HTTP requests.
     */
    std::string default_software{"app"};

    /**
     * @brief Checks whether the configuration can start a relay.
     *
     * @return true if the configuration is usable, otherwise false.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return host.valid() &&
             http.valid() &&
             !default_software.empty();
    }
  };

  /**
   * @brief Represents the current relay server status.
   */
  struct RelayServerStatus
  {
    /**
     * @brief Indicates whether the relay is running.
     */
    bool running{false};

    /**
     * @brief Indicates whether a Host is currently connected.
     */
    bool host_connected{false};

    /**
     * @brief Effective Host listener port.
     */
    std::uint16_t host_port{0};

    /**
     * @brief Effective public HTTP listener port.
     */
    std::uint16_t http_port{0};

    /**
     * @brief Number of public requests accepted by the relay.
     */
    std::uint64_t requests{0};

    /**
     * @brief Number of public requests that could not be relayed.
     */
    std::uint64_t failures{0};
  };

  /**
   * @brief Provides a minimal HTTP relay for remote Host reachability.
   *
   * RelayServer accepts one outbound Host connection and one public HTTP
   * listener. Public HTTP requests are encoded with RelayProtocol and forwarded
   * to the connected Host.
   */
  class RelayServer
  {
  public:
    /**
     * @brief Creates a relay server.
     *
     * @param config Relay configuration.
     */
    explicit RelayServer(
        RelayServerConfig config) noexcept;

    /**
     * @brief Stops the relay server and releases its resources.
     */
    ~RelayServer();

    RelayServer(const RelayServer &) = delete;
    RelayServer &operator=(const RelayServer &) = delete;

    /**
     * @brief Starts both relay listeners.
     *
     * @return true if both listeners started successfully, otherwise false.
     */
    [[nodiscard]] bool start();

    /**
     * @brief Stops the relay server.
     */
    void stop() noexcept;

    /**
     * @brief Returns whether the relay server is running.
     *
     * @return true if the relay is running, otherwise false.
     */
    [[nodiscard]] bool running() const noexcept
    {
      return running_.load();
    }

    /**
     * @brief Returns whether a Host is currently connected.
     *
     * @return true if a Host is connected, otherwise false.
     */
    [[nodiscard]] bool host_connected() const noexcept
    {
      return host_connected_.load();
    }

    /**
     * @brief Returns the effective Host listener port.
     *
     * @return Bound Host listener port, or zero if stopped.
     */
    [[nodiscard]] std::uint16_t host_port() const noexcept
    {
      return host_port_.load();
    }

    /**
     * @brief Returns the effective public HTTP listener port.
     *
     * @return Bound public HTTP listener port, or zero if stopped.
     */
    [[nodiscard]] std::uint16_t http_port() const noexcept
    {
      return http_port_.load();
    }

    /**
     * @brief Returns the current relay server status.
     *
     * @return Current status snapshot.
     */
    [[nodiscard]] RelayServerStatus status() const noexcept;

  private:
    void accept_hosts() noexcept;

    void accept_http_clients() noexcept;

    void serve_http_client(
        NativeSocket fd) noexcept;

    void replace_host(
        NativeSocket fd) noexcept;

    void clear_host(
        NativeSocket fd) noexcept;

    void register_client(
        NativeSocket fd);

    [[nodiscard]] bool unregister_client(
        NativeSocket fd) noexcept;

    void close_clients() noexcept;

    RelayServerConfig config_;
    std::atomic_bool stopping_{false};
    std::atomic_bool running_{false};
    std::atomic_bool host_connected_{false};
    std::atomic_uint16_t host_port_{0};
    std::atomic_uint16_t http_port_{0};
    std::atomic_uint64_t requests_{0};
    std::atomic_uint64_t failures_{0};
    std::atomic<NativeSocket> host_listener_{InvalidSocket};
    std::atomic<NativeSocket> http_listener_{InvalidSocket};
    std::atomic<NativeSocket> active_host_{InvalidSocket};
    std::mutex host_mutex_;
    std::mutex clients_mutex_;
    std::vector<NativeSocket> clients_;
    std::thread host_thread_;
    std::thread http_thread_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_RELAY_RELAY_SERVER_HPP
