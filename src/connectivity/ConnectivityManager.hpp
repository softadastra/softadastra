/**
 *
 *  @file ConnectivityManager.hpp
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

#ifndef SOFTADASTRA_CONNECTIVITY_CONNECTIVITY_MANAGER_HPP
#define SOFTADASTRA_CONNECTIVITY_CONNECTIVITY_MANAGER_HPP

#include "platform/Network.hpp"

namespace softadastra
{
  /**
   * @brief Provides Host-level access to platform connectivity information.
   *
   * ConnectivityManager translates machine-level network capability into the
   * connectivity information needed by higher-level Host components.
   *
   * It does not interpret application protocols, inspect hosted software, or
   * assume that network connectivity implies Internet access.
   */
  class ConnectivityManager
  {
  public:
    /**
     * @brief Creates a connectivity manager for a platform network capability.
     *
     * The Network instance must remain valid for the lifetime of the manager.
     *
     * @param network Platform network capability used by this manager.
     */
    explicit ConnectivityManager(Network &network) noexcept;

    /**
     * @brief Checks whether usable networking is available to the Host.
     *
     * This reports whether the platform exposes a network capability that
     * Softadastra can use.
     *
     * @return true if networking is available, otherwise false.
     */
    [[nodiscard]] bool is_available() const noexcept;

    /**
     * @brief Checks whether the Host currently has network connectivity.
     *
     * Connectivity does not imply Internet access or reachability of any
     * particular remote service.
     *
     * @return true if the underlying platform reports connectivity, otherwise
     *         false.
     */
    [[nodiscard]] bool is_connected() const noexcept;

  private:
    Network &network_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONNECTIVITY_CONNECTIVITY_MANAGER_HPP
