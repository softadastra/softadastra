/**
 *
 *  @file LocalGatewaySocketProvider.hpp
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

#ifndef SOFTADASTRA_PLATFORM_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
#define SOFTADASTRA_PLATFORM_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP

namespace softadastra
{
  /**
   * @brief Describes the state of a local gateway socket.
   */
  enum class LocalGatewaySocketState
  {
    Available,
    Unavailable,
    Invalid
  };

  /**
   * @brief Represents a local gateway socket acquisition result.
   */
  struct LocalGatewaySocket
  {
    /**
     * @brief Current socket state.
     */
    LocalGatewaySocketState state{
        LocalGatewaySocketState::Unavailable};

    /**
     * @brief Native socket file descriptor.
     */
    int fd{-1};
  };

  /**
   * @brief Provides a platform interface for acquiring a local gateway socket.
   */
  class LocalGatewaySocketProvider
  {
  public:
    /**
     * @brief Destroys the local gateway socket provider.
     */
    virtual ~LocalGatewaySocketProvider() = default;

    /**
     * @brief Acquires a local gateway socket.
     *
     * @return Socket acquisition result.
     */
    [[nodiscard]] virtual LocalGatewaySocket acquire() const noexcept = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
