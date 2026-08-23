/**
 *
 *  @file Network.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NETWORK_HPP
#define SOFTADASTRA_PLATFORM_NETWORK_HPP

namespace softadastra
{
  /**
   * @brief Defines the network capability exposed by a Host platform.
   *
   * Network represents the minimal machine-level network information required
   * by Softadastra. It allows higher-level Host components to determine whether
   * networking is available and whether the machine currently has network
   * connectivity.
   *
   * This interface describes infrastructure provided by the machine. It does not
   * describe application protocols such as HTTP, WebSocket, gRPC, or any
   * protocol used internally by hosted software.
   */
  class Network
  {
  public:
    /**
     * @brief Destroys the network interface.
     */
    virtual ~Network() = default;

    /**
     * @brief Checks whether networking is available on the machine.
     *
     * Availability means that the platform exposes at least one network
     * capability that Softadastra can use.
     *
     * @return true if networking is available, otherwise false.
     */
    [[nodiscard]] virtual bool is_available() const noexcept = 0;

    /**
     * @brief Checks whether the machine currently has network connectivity.
     *
     * This operation reports platform connectivity only. It does not guarantee
     * that Internet access or any particular remote service is reachable.
     *
     * @return true if network connectivity is currently present, otherwise false.
     */
    [[nodiscard]] virtual bool is_connected() const noexcept = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NETWORK_HPP
