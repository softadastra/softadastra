/**
 *
 *  @file NativeNetwork.cpp
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

#include "platform/NativeNetwork.hpp"

#include <array>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)

#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <net/if_dl.h>
#else
#include <netpacket/packet.h>
#endif

#endif

namespace
{
#if defined(_WIN32)

  std::vector<unsigned char> adapter_buffer()
  {
    ULONG size = 0;

    const ULONG flags =
        GAA_FLAG_SKIP_ANYCAST |
        GAA_FLAG_SKIP_MULTICAST |
        GAA_FLAG_SKIP_DNS_SERVER;

    const ULONG first_result = GetAdaptersAddresses(
        AF_UNSPEC,
        flags,
        nullptr,
        nullptr,
        &size);

    if (first_result != ERROR_BUFFER_OVERFLOW || size == 0)
    {
      return {};
    }

    std::vector<unsigned char> buffer(size);

    auto *addresses =
        reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());

    const ULONG result = GetAdaptersAddresses(
        AF_UNSPEC,
        flags,
        nullptr,
        addresses,
        &size);

    if (result != NO_ERROR)
    {
      return {};
    }

    return buffer;
  }

  bool is_usable_adapter(const IP_ADAPTER_ADDRESSES &adapter) noexcept
  {
    return adapter.IfType != IF_TYPE_SOFTWARE_LOOPBACK;
  }

  bool has_network_address(const IP_ADAPTER_ADDRESSES &adapter) noexcept
  {
    for (const IP_ADAPTER_UNICAST_ADDRESS *address =
             adapter.FirstUnicastAddress;
         address != nullptr;
         address = address->Next)
    {
      if (address->Address.lpSockaddr == nullptr)
      {
        continue;
      }

      const int family =
          address->Address.lpSockaddr->sa_family;

      if (family == AF_INET || family == AF_INET6)
      {
        return true;
      }
    }

    return false;
  }

  std::string address_text(const SOCKADDR *address)
  {
    std::array<char, INET6_ADDRSTRLEN> value{};

    if (address->sa_family == AF_INET)
    {
      const auto *ipv4 = reinterpret_cast<const SOCKADDR_IN *>(address);
      return InetNtopA(AF_INET, &ipv4->sin_addr, value.data(), value.size()) != nullptr
                 ? value.data()
                 : std::string{};
    }

    const auto *ipv6 = reinterpret_cast<const SOCKADDR_IN6 *>(address);
    return InetNtopA(AF_INET6, &ipv6->sin6_addr, value.data(), value.size()) != nullptr
               ? value.data()
               : std::string{};
  }

#else

  bool is_usable_interface(const ifaddrs &interface) noexcept
  {
    if (interface.ifa_name == nullptr)
    {
      return false;
    }

    return (interface.ifa_flags & IFF_LOOPBACK) == 0;
  }

  bool is_interface_up(const ifaddrs &interface) noexcept
  {
    return (interface.ifa_flags & IFF_UP) != 0;
  }

  bool has_network_address(const ifaddrs &interface) noexcept
  {
    if (interface.ifa_addr == nullptr)
    {
      return false;
    }

    const int family = interface.ifa_addr->sa_family;

    return family == AF_INET || family == AF_INET6;
  }

  bool represents_network_interface(const ifaddrs &interface) noexcept
  {
    if (interface.ifa_addr == nullptr)
    {
      return false;
    }

#if defined(__APPLE__)
    return interface.ifa_addr->sa_family == AF_LINK;
#else
    return interface.ifa_addr->sa_family == AF_PACKET;
#endif
  }

  bool is_loopback_address(const sockaddr &address) noexcept
  {
    if (address.sa_family == AF_INET)
    {
      const auto &ipv4 = reinterpret_cast<const sockaddr_in &>(address);
      return (ntohl(ipv4.sin_addr.s_addr) >> 24) == 127;
    }

    if (address.sa_family == AF_INET6)
    {
      const auto &ipv6 = reinterpret_cast<const sockaddr_in6 &>(address);
      return IN6_IS_ADDR_LOOPBACK(&ipv6.sin6_addr) != 0;
    }

    return true;
  }

  std::string address_text(const sockaddr &address)
  {
    std::array<char, INET6_ADDRSTRLEN> value{};

    if (address.sa_family == AF_INET)
    {
      const auto &ipv4 = reinterpret_cast<const sockaddr_in &>(address);
      return inet_ntop(AF_INET, &ipv4.sin_addr, value.data(), value.size()) != nullptr
                 ? value.data()
                 : std::string{};
    }

    const auto &ipv6 = reinterpret_cast<const sockaddr_in6 &>(address);
    return inet_ntop(AF_INET6, &ipv6.sin6_addr, value.data(), value.size()) != nullptr
               ? value.data()
               : std::string{};
  }

#endif

} // namespace

namespace softadastra
{

  bool NativeNetwork::is_available() const noexcept
  {
#if defined(_WIN32)

    std::vector<unsigned char> buffer = adapter_buffer();

    if (buffer.empty())
    {
      return false;
    }

    const auto *adapter =
        reinterpret_cast<const IP_ADAPTER_ADDRESSES *>(buffer.data());

    for (; adapter != nullptr; adapter = adapter->Next)
    {
      if (is_usable_adapter(*adapter))
      {
        return true;
      }
    }

    return false;

#else

    ifaddrs *interfaces = nullptr;

    if (::getifaddrs(&interfaces) != 0)
    {
      return false;
    }

    bool available = false;

    for (const ifaddrs *interface = interfaces;
         interface != nullptr;
         interface = interface->ifa_next)
    {
      if (!is_usable_interface(*interface))
      {
        continue;
      }

      if (represents_network_interface(*interface))
      {
        available = true;
        break;
      }
    }

    ::freeifaddrs(interfaces);

    return available;

#endif
  }

  bool NativeNetwork::is_connected() const noexcept
  {
#if defined(_WIN32)

    std::vector<unsigned char> buffer = adapter_buffer();

    if (buffer.empty())
    {
      return false;
    }

    const auto *adapter =
        reinterpret_cast<const IP_ADAPTER_ADDRESSES *>(buffer.data());

    for (; adapter != nullptr; adapter = adapter->Next)
    {
      if (!is_usable_adapter(*adapter))
      {
        continue;
      }

      if (adapter->OperStatus != IfOperStatusUp)
      {
        continue;
      }

      if (has_network_address(*adapter))
      {
        return true;
      }
    }

    return false;

#else

    ifaddrs *interfaces = nullptr;

    if (::getifaddrs(&interfaces) != 0)
    {
      return false;
    }

    bool connected = false;

    for (const ifaddrs *interface = interfaces;
         interface != nullptr;
         interface = interface->ifa_next)
    {
      if (!is_usable_interface(*interface))
      {
        continue;
      }

      if (!is_interface_up(*interface))
      {
        continue;
      }

      if (has_network_address(*interface))
      {
        connected = true;
        break;
      }
    }

    ::freeifaddrs(interfaces);

    return connected;

#endif
  }

  std::string NativeNetwork::host_name() const
  {
#if defined(_WIN32)

    std::array<char, MAX_COMPUTERNAME_LENGTH + 1> value{};
    DWORD size = static_cast<DWORD>(value.size());
    return GetComputerNameA(value.data(), &size) != 0
               ? std::string(value.data(), size)
               : std::string{};

#else

    std::array<char, 256> value{};
    return ::gethostname(value.data(), value.size()) == 0
               ? value.data()
               : std::string{};

#endif
  }

  std::vector<LocalNetworkAddress> NativeNetwork::local_addresses() const
  {
    std::vector<LocalNetworkAddress> addresses;

#if defined(_WIN32)

    std::vector<unsigned char> buffer = adapter_buffer();

    if (buffer.empty())
    {
      return addresses;
    }

    const auto *adapter =
        reinterpret_cast<const IP_ADAPTER_ADDRESSES *>(buffer.data());

    for (; adapter != nullptr; adapter = adapter->Next)
    {
      if (!is_usable_adapter(*adapter) ||
          adapter->OperStatus != IfOperStatusUp)
      {
        continue;
      }

      for (const IP_ADAPTER_UNICAST_ADDRESS *address =
               adapter->FirstUnicastAddress;
           address != nullptr;
           address = address->Next)
      {
        if (address->Address.lpSockaddr == nullptr)
        {
          continue;
        }

        const int family = address->Address.lpSockaddr->sa_family;

        if (family != AF_INET && family != AF_INET6)
        {
          continue;
        }

        const std::string value = address_text(address->Address.lpSockaddr);

        if (!value.empty())
        {
          addresses.push_back(
              LocalNetworkAddress{
                  family == AF_INET ? LocalAddressFamily::IPv4
                                    : LocalAddressFamily::IPv6,
                  adapter->AdapterName,
                  value});
        }
      }
    }

#else

    ifaddrs *interfaces = nullptr;

    if (::getifaddrs(&interfaces) != 0)
    {
      return addresses;
    }

    for (const ifaddrs *interface = interfaces;
         interface != nullptr;
         interface = interface->ifa_next)
    {
      if (!is_usable_interface(*interface) ||
          !is_interface_up(*interface) ||
          !has_network_address(*interface) ||
          is_loopback_address(*interface->ifa_addr))
      {
        continue;
      }

      const int family = interface->ifa_addr->sa_family;
      std::string value = address_text(*interface->ifa_addr);

      if (value.empty())
      {
        continue;
      }

      if (family == AF_INET6)
      {
        const auto &ipv6 =
            reinterpret_cast<const sockaddr_in6 &>(*interface->ifa_addr);

        if (IN6_IS_ADDR_LINKLOCAL(&ipv6.sin6_addr) != 0)
        {
          value += "%";
          value += interface->ifa_name;
        }
      }

      addresses.push_back(
          LocalNetworkAddress{
              family == AF_INET ? LocalAddressFamily::IPv4
                                : LocalAddressFamily::IPv6,
              interface->ifa_name,
              std::move(value)});
    }

    ::freeifaddrs(interfaces);

#endif

    return addresses;
  }

} // namespace softadastra
