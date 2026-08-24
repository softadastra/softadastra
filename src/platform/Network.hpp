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

#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Identifies the address family of a local network address.
   */
  enum class LocalAddressFamily
  {
    IPv4,
    IPv6
  };

  /**
   * @brief Describes an active non-loopback address of the Host machine.
   */
  struct LocalNetworkAddress
  {
    LocalAddressFamily family;
    std::string interface_name;
    std::string value;
  };

  enum class NetworkState
  {
    Available,
    Unavailable
  };

  enum class NetworkInterfaceType
  {
    Wifi,
    Ethernet,
    Loopback,
    Other,
    Unknown
  };

  enum class LocalNetworkState
  {
    Existing,
    Unavailable
  };

  enum class ManagedNetworkCapability
  {
    Available,
    Unavailable
  };

  /**
   * @brief Describes the Host's currently observable network capability.
   *
   * This describes interfaces and locally observable capabilities only. It
   * does not assert that another device can reach the Host.
   */
  struct NetworkCapability
  {
    NetworkState state{NetworkState::Unavailable};
    std::string primary_ipv4;
    std::string primary_interface;
    NetworkInterfaceType interface_type{NetworkInterfaceType::Unknown};
    LocalNetworkState local_network_state{LocalNetworkState::Unavailable};
    ManagedNetworkCapability managed_network_capability{
        ManagedNetworkCapability::Unavailable};
  };

  [[nodiscard]] const char *network_state_name(NetworkState value) noexcept;
  [[nodiscard]] const char *network_interface_type_name(
      NetworkInterfaceType value) noexcept;
  [[nodiscard]] const char *local_network_state_name(
      LocalNetworkState value) noexcept;
  [[nodiscard]] const char *managed_network_capability_name(
      ManagedNetworkCapability value) noexcept;

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

    /**
     * @brief Returns the native hostname of the Host machine when available.
     *
     * The hostname is informational only. It is not a promise that a local
     * name is resolvable by another device.
     */
    [[nodiscard]] virtual std::string host_name() const
    {
      return {};
    }

    /**
     * @brief Returns active non-loopback local addresses of the Host.
     *
     * Returned addresses describe the existing local network only. They do
     * not expose a hosted software port or guarantee Internet access.
     */
    [[nodiscard]] virtual std::vector<LocalNetworkAddress> local_addresses() const
    {
      return {};
    }

    /**
     * @brief Returns the current primary local IPv4 address when available.
     *
     * The default implementation selects the first reported local IPv4
     * address. Native platforms can use their routing state to provide a more
     * precise primary address.
     */
    [[nodiscard]] virtual std::string primary_ipv4() const
    {
      for (const auto &address : local_addresses())
      {
        if (address.family == LocalAddressFamily::IPv4)
        {
          return address.value;
        }
      }

      return {};
    }

    /**
     * @brief Returns a detailed, read-only view of Host network capability.
     *
     * Platforms that can identify an interface from routing state or inspect
     * its physical kind should override this method. The default is a safe
     * unsupported-platform fallback.
     */
    [[nodiscard]] virtual NetworkCapability network_capability() const
    {
      const std::string primary = primary_ipv4();

      for (const auto &address : local_addresses())
      {
        if (address.family == LocalAddressFamily::IPv4 &&
            address.value == primary && !address.value.starts_with("127."))
        {
          return NetworkCapability{
              NetworkState::Available,
              primary,
              address.interface_name,
              NetworkInterfaceType::Unknown,
              LocalNetworkState::Existing,
              ManagedNetworkCapability::Unavailable};
        }
      }

      return {};
    }
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NETWORK_HPP
