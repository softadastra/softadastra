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

#if defined(_WIN32)

#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <vector>

#else

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>

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

} // namespace softadastra
