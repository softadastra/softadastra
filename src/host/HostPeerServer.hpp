/**
 *
 *  @file HostPeerServer.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_PEER_SERVER_HPP
#define SOFTADASTRA_HOST_HOST_PEER_SERVER_HPP

#include "host/HostIdentity.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <string>
#include <thread>

namespace softadastra
{
  /** @brief Serves the minimal pinned TLS Host-to-Host protocol. */
  class HostPeerServer
  {
  public:
    /** @brief Creates a Host peer listener with identity-derived TLS material. */
    HostPeerServer(
        const HostIdentity &identity,
        std::filesystem::path certificate_directory,
        std::string address,
        std::uint16_t port,
        std::string infrastructure_info) noexcept;

    ~HostPeerServer();
    HostPeerServer(const HostPeerServer &) = delete;

    /** @brief Starts the TLS 1.3 listener. */
    [[nodiscard]] bool start();

    /** @brief Stops the listener and all future peer requests. */
    void stop() noexcept;

    /** @brief Returns whether the listener is ready to accept connections. */
    [[nodiscard]] bool listening() const noexcept;

    /** @brief Returns the effective TCP port, including when port zero was used. */
    [[nodiscard]] std::uint16_t port() const noexcept;

  private:
    void run() noexcept;

    const HostIdentity &identity_;
    std::filesystem::path certificate_directory_;
    std::string address_;
    std::uint16_t port_;
    std::string infrastructure_info_;
    std::atomic_bool running_{false};
    std::atomic_bool listening_{false};
    std::atomic_int descriptor_{-1};
    std::thread thread_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_PEER_SERVER_HPP
