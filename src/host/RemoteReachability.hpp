/**
 *
 *  @file RemoteReachability.hpp
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

#ifndef SOFTADASTRA_HOST_REMOTE_REACHABILITY_HPP
#define SOFTADASTRA_HOST_REMOTE_REACHABILITY_HPP

#include "host/LocalGatewayTargetResolver.hpp"
#include "host/NativeSocket.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

namespace softadastra
{
  /**
   * @brief Describes the remote relay endpoint used by the Host.
   */
  struct RemoteEndpoint
  {
    /**
     * @brief Relay address.
     */
    std::string address;

    /**
     * @brief Relay TCP port.
     */
    std::uint16_t port{0};

    /**
     * @brief Checks whether the endpoint contains a usable address and port.
     *
     * @return true if the endpoint is valid, otherwise false.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return !address.empty() && port != 0;
    }
  };

  /**
   * @brief Describes the state of remote reachability.
   */
  enum class RemoteReachabilityState
  {
    Disabled,
    Connecting,
    Ready,
    Degraded
  };

  /**
   * @brief Maintains an outbound connection to a remote relay.
   *
   * RemoteReachability only initiates outbound connections. It does not bind
   * or listen for incoming network connections.
   */
  class RemoteReachability
  {
  public:
    /**
     * @brief Creates a remote reachability service.
     *
     * @param resolver Resolver used to locate local software targets.
     */
    explicit RemoteReachability(
        LocalGatewayTargetResolver &resolver) noexcept;

    /**
     * @brief Disables remote reachability and releases its resources.
     */
    ~RemoteReachability();

    RemoteReachability(const RemoteReachability &) = delete;

    /**
     * @brief Configures and starts remote reachability.
     *
     * @param endpoint Remote relay endpoint.
     */
    void configure(
        RemoteEndpoint endpoint);

    /**
     * @brief Disables remote reachability.
     */
    void disable() noexcept;

    /**
     * @brief Returns the current remote reachability state.
     *
     * @return Current remote reachability state.
     */
    [[nodiscard]] RemoteReachabilityState state() const noexcept;

    /**
     * @brief Returns the currently configured relay endpoint.
     *
     * @return Configured remote endpoint.
     */
    [[nodiscard]] RemoteEndpoint endpoint() const;

  private:
    void run() noexcept;

    LocalGatewayTargetResolver &resolver_;
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    RemoteEndpoint endpoint_;
    RemoteReachabilityState state_{
        RemoteReachabilityState::Disabled};
    std::atomic_bool stopping_{false};
    std::atomic<NativeSocket> socket_{
        InvalidSocket};
    std::thread thread_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_REMOTE_REACHABILITY_HPP
