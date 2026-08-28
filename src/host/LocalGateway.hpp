/**
 *
 *  @file LocalGateway.hpp
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

#ifndef SOFTADASTRA_HOST_LOCAL_GATEWAY_HPP
#define SOFTADASTRA_HOST_LOCAL_GATEWAY_HPP

#include "host/LocalGatewayTargetResolver.hpp"

#include <cstdint>
#include <string>

namespace softadastra
{
  /**
   * @brief Describes the runtime state of the local gateway.
   */
  enum class LocalGatewayState
  {
    Stopped,
    Running,
    Failed
  };

  /**
   * @brief Describes the current local gateway status.
   */
  struct LocalGatewayStatus
  {
    /**
     * @brief Current gateway state.
     */
    LocalGatewayState state{
        LocalGatewayState::Stopped};

    /**
     * @brief IPv4 address on which the gateway is listening.
     */
    std::string address;

    /**
     * @brief TCP port on which the gateway is listening.
     */
    std::uint16_t port{};
  };

  /**
   * @brief Provides the local HTTP gateway for registered software.
   */
  class LocalGateway
  {
  public:
    /**
     * @brief Creates a local gateway.
     *
     * @param resolver Resolver used to locate local software targets.
     */
    explicit LocalGateway(
        LocalGatewayTargetResolver &resolver) noexcept;

    /**
     * @brief Stops the gateway and releases its resources.
     */
    ~LocalGateway();

    /**
     * @brief Starts the gateway on an IPv4 address and TCP port.
     *
     * @param address IPv4 address on which to listen.
     * @param port TCP port on which to listen.
     *
     * @return true if the gateway started successfully, otherwise false.
     */
    bool start(
        std::string address,
        std::uint16_t port);

    /**
     * @brief Starts the gateway from an existing listening socket.
     *
     * On success, the gateway takes ownership of the socket descriptor.
     * On failure, ownership remains with the caller.
     *
     * @param fd Listening socket descriptor.
     *
     * @return true if the gateway started successfully, otherwise false.
     */
    bool start_from_socket(
        int fd);

    /**
     * @brief Stops the local gateway.
     */
    void stop() noexcept;

    /**
     * @brief Returns the current gateway status.
     *
     * @return Current local gateway status.
     */
    [[nodiscard]] LocalGatewayStatus status() const;

  private:
    void run() noexcept;

    LocalGatewayTargetResolver &resolver_;
    int listener_{-1};
    LocalGatewayStatus status_{};
    bool stopping_{false};

    class Thread;
    Thread *thread_{nullptr};
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_LOCAL_GATEWAY_HPP
