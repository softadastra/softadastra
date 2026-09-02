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
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)

// clang-format off
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <netioapi.h>
// clang-format on

#else

#include <arpa/inet.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined(__linux__)

#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>

#endif

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

    const ULONG first_result =
        GetAdaptersAddresses(
            AF_UNSPEC,
            flags,
            nullptr,
            nullptr,
            &size);

    if (first_result != ERROR_BUFFER_OVERFLOW ||
        size == 0)
    {
      return {};
    }

    std::vector<unsigned char> buffer(
        size);

    auto *addresses =
        reinterpret_cast<IP_ADAPTER_ADDRESSES *>(
            buffer.data());

    const ULONG result =
        GetAdaptersAddresses(
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

  bool is_usable_adapter(
      const IP_ADAPTER_ADDRESSES &adapter) noexcept
  {
    return adapter.IfType !=
           IF_TYPE_SOFTWARE_LOOPBACK;
  }

  bool has_network_address(
      const IP_ADAPTER_ADDRESSES &adapter) noexcept
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

      if (family == AF_INET ||
          family == AF_INET6)
      {
        return true;
      }
    }

    return false;
  }

  std::string address_text(
      const SOCKADDR *address)
  {
    std::array<char, INET6_ADDRSTRLEN> value{};

    if (address->sa_family == AF_INET)
    {
      const auto *ipv4 =
          reinterpret_cast<const SOCKADDR_IN *>(
              address);

      return InetNtopA(
                 AF_INET,
                 &ipv4->sin_addr,
                 value.data(),
                 value.size()) != nullptr
                 ? value.data()
                 : std::string{};
    }

    const auto *ipv6 =
        reinterpret_cast<const SOCKADDR_IN6 *>(
            address);

    return InetNtopA(
               AF_INET6,
               &ipv6->sin6_addr,
               value.data(),
               value.size()) != nullptr
               ? value.data()
               : std::string{};
  }

#else

  bool is_usable_interface(
      const ifaddrs &interface) noexcept
  {
    if (interface.ifa_name == nullptr)
    {
      return false;
    }

    return (interface.ifa_flags &
            IFF_LOOPBACK) == 0;
  }

  bool is_interface_up(
      const ifaddrs &interface) noexcept
  {
    return (interface.ifa_flags &
            IFF_UP) != 0;
  }

  bool has_network_address(
      const ifaddrs &interface) noexcept
  {
    if (interface.ifa_addr == nullptr)
    {
      return false;
    }

    const int family =
        interface.ifa_addr->sa_family;

    return family == AF_INET ||
           family == AF_INET6;
  }

  bool represents_network_interface(
      const ifaddrs &interface) noexcept
  {
    if (interface.ifa_addr == nullptr)
    {
      return false;
    }

#if defined(__APPLE__)

    return interface.ifa_addr->sa_family ==
           AF_LINK;

#else

    return interface.ifa_addr->sa_family ==
           AF_PACKET;

#endif
  }

  bool is_loopback_address(
      const sockaddr &address) noexcept
  {
    if (address.sa_family == AF_INET)
    {
      const auto &ipv4 =
          reinterpret_cast<const sockaddr_in &>(
              address);

      return (ntohl(ipv4.sin_addr.s_addr) >> 24) ==
             127;
    }

    if (address.sa_family == AF_INET6)
    {
      const auto &ipv6 =
          reinterpret_cast<const sockaddr_in6 &>(
              address);

      return IN6_IS_ADDR_LOOPBACK(
                 &ipv6.sin6_addr) != 0;
    }

    return true;
  }

  std::string address_text(
      const sockaddr &address)
  {
    std::array<char, INET6_ADDRSTRLEN> value{};

    if (address.sa_family == AF_INET)
    {
      const auto &ipv4 =
          reinterpret_cast<const sockaddr_in &>(
              address);

      return inet_ntop(
                 AF_INET,
                 &ipv4.sin_addr,
                 value.data(),
                 value.size()) != nullptr
                 ? value.data()
                 : std::string{};
    }

    const auto &ipv6 =
        reinterpret_cast<const sockaddr_in6 &>(
            address);

    return inet_ntop(
               AF_INET6,
               &ipv6.sin6_addr,
               value.data(),
               value.size()) != nullptr
               ? value.data()
               : std::string{};
  }

#endif

#if defined(__linux__)

  std::string ipv4_subnet(
      const std::string &interface_name,
      const std::string &ipv4)
  {
    ifaddrs *interfaces = nullptr;

    if (::getifaddrs(&interfaces) != 0)
    {
      return {};
    }

    std::string result;

    for (const ifaddrs *interface = interfaces;
         interface != nullptr;
         interface = interface->ifa_next)
    {
      if (interface->ifa_name == nullptr ||
          interface->ifa_addr == nullptr ||
          interface->ifa_netmask == nullptr ||
          interface_name != interface->ifa_name ||
          interface->ifa_addr->sa_family != AF_INET)
      {
        continue;
      }

      const auto &address =
          reinterpret_cast<const sockaddr_in &>(
              *interface->ifa_addr);

      const auto &mask =
          reinterpret_cast<const sockaddr_in &>(
              *interface->ifa_netmask);

      std::array<char, INET_ADDRSTRLEN> text{};

      if (::inet_ntop(
              AF_INET,
              &address.sin_addr,
              text.data(),
              text.size()) == nullptr ||
          ipv4 != text.data())
      {
        continue;
      }

      const std::uint32_t value =
          ntohl(
              address.sin_addr.s_addr);

      const std::uint32_t netmask =
          ntohl(
              mask.sin_addr.s_addr);

      unsigned int prefix = 0;

      for (std::uint32_t bit = 0x80000000U;
           bit != 0U;
           bit >>= 1U)
      {
        prefix +=
            (netmask & bit) != 0U
                ? 1U
                : 0U;
      }

      in_addr network{};

      network.s_addr =
          htonl(
              value & netmask);

      if (::inet_ntop(
              AF_INET,
              &network,
              text.data(),
              text.size()) != nullptr)
      {
        result =
            std::string(text.data()) +
            "/" +
            std::to_string(prefix);
      }

      break;
    }

    ::freeifaddrs(
        interfaces);

    return result;
  }

  bool attribute_ok(
      const nlattr *attribute,
      int length) noexcept
  {
    return length >=
               static_cast<int>(
                   sizeof(nlattr)) &&
           attribute->nla_len >=
               sizeof(nlattr) &&
           attribute->nla_len <=
               length;
  }

  bool message_ok(
      const nlmsghdr *message,
      int length) noexcept
  {
    return length >=
               static_cast<int>(
                   sizeof(nlmsghdr)) &&
           message->nlmsg_len >=
               sizeof(nlmsghdr) &&
           message->nlmsg_len <=
               static_cast<decltype(message->nlmsg_len)>(
                   length);
  }

  const nlmsghdr *next_message(
      const nlmsghdr *message,
      int &length) noexcept
  {
    const auto message_length =
        static_cast<std::size_t>(
            message->nlmsg_len);

    const auto aligned_length =
        (message_length +
         NLMSG_ALIGNTO -
         1U) &
        ~(NLMSG_ALIGNTO - 1U);

    if (aligned_length >
        static_cast<std::size_t>(length))
    {
      length = 0;
      return message;
    }

    length -=
        static_cast<int>(
            aligned_length);

    return reinterpret_cast<const nlmsghdr *>(
        reinterpret_cast<const char *>(message) +
        aligned_length);
  }

  const nlattr *next_attribute(
      const nlattr *attribute,
      int &length) noexcept
  {
    const int aligned_length =
        NLA_ALIGN(
            attribute->nla_len);

    length -=
        aligned_length;

    return reinterpret_cast<const nlattr *>(
        reinterpret_cast<const char *>(attribute) +
        aligned_length);
  }

  void *attribute_data(
      nlattr *attribute) noexcept
  {
    return reinterpret_cast<char *>(attribute) +
           sizeof(nlattr);
  }

  const void *attribute_data(
      const nlattr *attribute) noexcept
  {
    return reinterpret_cast<const char *>(attribute) +
           sizeof(nlattr);
  }

  int attribute_type(
      const nlattr *attribute) noexcept
  {
    return attribute->nla_type &
           NLA_TYPE_MASK;
  }

  bool has_attribute(
      const nlattr *attributes,
      int length,
      int wanted) noexcept
  {
    for (const nlattr *attribute = attributes;
         attribute_ok(
             attribute,
             length);
         attribute =
             next_attribute(
                 attribute,
                 length))
    {
      if (attribute_type(attribute) ==
          wanted)
      {
        return true;
      }
    }

    return false;
  }

  std::optional<std::uint16_t>
  generic_netlink_family(
      int descriptor,
      std::uint32_t sequence)
  {
    std::array<char, 256> buffer{};

    auto *message =
        reinterpret_cast<nlmsghdr *>(
            buffer.data());

    message->nlmsg_len =
        NLMSG_LENGTH(
            GENL_HDRLEN);

    message->nlmsg_type =
        GENL_ID_CTRL;

    message->nlmsg_flags =
        NLM_F_REQUEST;

    message->nlmsg_seq =
        sequence;

    auto *header =
        reinterpret_cast<genlmsghdr *>(
            NLMSG_DATA(message));

    header->cmd =
        CTRL_CMD_GETFAMILY;

    header->version = 1;

    auto *attribute =
        reinterpret_cast<nlattr *>(
            reinterpret_cast<char *>(header) +
            GENL_HDRLEN);

    constexpr char family_name[] =
        "nl80211";

    attribute->nla_type =
        CTRL_ATTR_FAMILY_NAME;

    constexpr auto attribute_size =
        sizeof(nlattr) +
        sizeof(family_name);

    static_assert(
        attribute_size <=
        std::numeric_limits<
            decltype(attribute->nla_len)>::max());

    attribute->nla_len =
        static_cast<decltype(attribute->nla_len)>(
            attribute_size);

    std::memcpy(
        attribute_data(attribute),
        family_name,
        sizeof(family_name));

    message->nlmsg_len +=
        NLA_ALIGN(
            attribute->nla_len);

    if (::send(
            descriptor,
            message,
            message->nlmsg_len,
            0) < 0)
    {
      return std::nullopt;
    }

    const ssize_t received =
        ::recv(
            descriptor,
            buffer.data(),
            buffer.size(),
            0);

    if (received <
        static_cast<ssize_t>(
            NLMSG_LENGTH(GENL_HDRLEN)))
    {
      return std::nullopt;
    }

    int remaining =
        static_cast<int>(
            received);

    for (const auto *reply =
             reinterpret_cast<const nlmsghdr *>(
                 buffer.data());
         message_ok(
             reply,
             remaining);
         reply =
             next_message(
                 reply,
                 remaining))
    {
      if (reply->nlmsg_type ==
          NLMSG_ERROR)
      {
        return std::nullopt;
      }

      auto *reply_header =
          reinterpret_cast<genlmsghdr *>(
              NLMSG_DATA(reply));

      const auto header_length =
          static_cast<decltype(reply->nlmsg_len)>(
              NLMSG_LENGTH(
                  GENL_HDRLEN));

      if (reply->nlmsg_len <
              header_length ||
          reply->nlmsg_len -
                  header_length >
              static_cast<decltype(reply->nlmsg_len)>(
                  std::numeric_limits<int>::max()))
      {
        continue;
      }

      int attribute_length =
          static_cast<int>(
              reply->nlmsg_len -
              header_length);

      const auto *attributes =
          reinterpret_cast<const nlattr *>(
              reinterpret_cast<const char *>(
                  reply_header) +
              GENL_HDRLEN);

      for (const nlattr *current = attributes;
           attribute_ok(
               current,
               attribute_length);
           current =
               next_attribute(
                   current,
                   attribute_length))
      {
        if (attribute_type(current) ==
                CTRL_ATTR_FAMILY_ID &&
            current->nla_len >=
                sizeof(nlattr) +
                    sizeof(std::uint16_t))
        {
          std::uint16_t id = 0;

          std::memcpy(
              &id,
              attribute_data(current),
              sizeof(id));

          return id;
        }
      }
    }

    return std::nullopt;
  }

  bool wifi_hotspot_supported() noexcept
  {
    const int descriptor =
        ::socket(
            AF_NETLINK,
            SOCK_RAW,
            NETLINK_GENERIC);

    if (descriptor < 0)
    {
      return false;
    }

    sockaddr_nl address{};
    address.nl_family =
        AF_NETLINK;

    if (::bind(
            descriptor,
            reinterpret_cast<const sockaddr *>(
                &address),
            sizeof(address)) != 0)
    {
      ::close(descriptor);
      return false;
    }

    constexpr std::uint32_t sequence = 1;

    const auto family =
        generic_netlink_family(
            descriptor,
            sequence);

    if (!family.has_value())
    {
      ::close(descriptor);
      return false;
    }

    std::array<char,
               NLMSG_SPACE(GENL_HDRLEN)>
        request{};

    auto *message =
        reinterpret_cast<nlmsghdr *>(
            request.data());

    message->nlmsg_len =
        NLMSG_LENGTH(
            GENL_HDRLEN);

    message->nlmsg_type =
        family.value();

    message->nlmsg_flags =
        NLM_F_REQUEST |
        NLM_F_DUMP;

    message->nlmsg_seq =
        sequence + 1;

    auto *header =
        reinterpret_cast<genlmsghdr *>(
            NLMSG_DATA(message));

    header->cmd =
        NL80211_CMD_GET_WIPHY;

    header->version = 0;

    if (::send(
            descriptor,
            message,
            message->nlmsg_len,
            0) < 0)
    {
      ::close(descriptor);
      return false;
    }

    bool supported = false;

    std::array<char, 8192> buffer{};

    while (true)
    {
      ssize_t received =
          ::recv(
              descriptor,
              buffer.data(),
              buffer.size(),
              0);

      if (received <= 0)
      {
        break;
      }

      int remaining =
          static_cast<int>(
              received);

      for (const auto *reply =
               reinterpret_cast<const nlmsghdr *>(
                   buffer.data());
           message_ok(
               reply,
               remaining);
           reply =
               next_message(
                   reply,
                   remaining))
      {
        if (reply->nlmsg_type ==
                NLMSG_DONE ||
            reply->nlmsg_type ==
                NLMSG_ERROR)
        {
          ::close(descriptor);

          return supported;
        }

        const auto *header =
            reinterpret_cast<const genlmsghdr *>(
                NLMSG_DATA(reply));

        const auto header_length =
            static_cast<decltype(reply->nlmsg_len)>(
                NLMSG_LENGTH(
                    GENL_HDRLEN));

        if (reply->nlmsg_len <
                header_length ||
            reply->nlmsg_len -
                    header_length >
                static_cast<decltype(reply->nlmsg_len)>(
                    std::numeric_limits<int>::max()))
        {
          continue;
        }

        int attribute_length =
            static_cast<int>(
                reply->nlmsg_len -
                header_length);

        const auto *attributes =
            reinterpret_cast<const nlattr *>(
                reinterpret_cast<const char *>(header) +
                GENL_HDRLEN);

        for (const nlattr *attribute = attributes;
             attribute_ok(
                 attribute,
                 attribute_length);
             attribute =
                 next_attribute(
                     attribute,
                     attribute_length))
        {
          if (attribute_type(attribute) !=
              NL80211_ATTR_SUPPORTED_IFTYPES)
          {
            continue;
          }

          if (attribute->nla_len <
              sizeof(nlattr))
          {
            continue;
          }

          const auto payload_length =
              attribute->nla_len -
              sizeof(nlattr);

          const int types_length =
              static_cast<int>(
                  payload_length);

          const auto *types =
              reinterpret_cast<const nlattr *>(
                  attribute_data(attribute));

          if (has_attribute(
                  types,
                  types_length,
                  NL80211_IFTYPE_AP))
          {
            supported = true;
          }
        }
      }
    }

    ::close(descriptor);

    return supported;
  }

#endif

} // namespace

namespace softadastra
{
  const char *network_state_name(
      NetworkState value) noexcept
  {
    return value == NetworkState::Available
               ? "available"
               : "unavailable";
  }

  const char *network_interface_type_name(
      NetworkInterfaceType value) noexcept
  {
    switch (value)
    {
    case NetworkInterfaceType::Wifi:
      return "wifi";

    case NetworkInterfaceType::Ethernet:
      return "ethernet";

    case NetworkInterfaceType::Loopback:
      return "loopback";

    case NetworkInterfaceType::Other:
      return "other";

    case NetworkInterfaceType::Unknown:
      return "unknown";
    }

    return "unknown";
  }

  const char *local_network_state_name(
      LocalNetworkState value) noexcept
  {
    return value == LocalNetworkState::Existing
               ? "existing"
               : "unavailable";
  }

  const char *managed_network_capability_name(
      ManagedNetworkCapability value) noexcept
  {
    return value ==
                   ManagedNetworkCapability::Available
               ? "available"
               : "unavailable";
  }

  bool NativeNetwork::is_available() const noexcept
  {
#if defined(_WIN32)

    std::vector<unsigned char> buffer =
        adapter_buffer();

    if (buffer.empty())
    {
      return false;
    }

    const auto *adapter =
        reinterpret_cast<
            const IP_ADAPTER_ADDRESSES *>(
            buffer.data());

    for (;
         adapter != nullptr;
         adapter = adapter->Next)
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

      if (represents_network_interface(
              *interface))
      {
        available = true;
        break;
      }
    }

    ::freeifaddrs(
        interfaces);

    return available;

#endif
  }

  bool NativeNetwork::is_connected() const noexcept
  {
#if defined(_WIN32)

    std::vector<unsigned char> buffer =
        adapter_buffer();

    if (buffer.empty())
    {
      return false;
    }

    const auto *adapter =
        reinterpret_cast<
            const IP_ADAPTER_ADDRESSES *>(
            buffer.data());

    for (;
         adapter != nullptr;
         adapter = adapter->Next)
    {
      if (!is_usable_adapter(*adapter))
      {
        continue;
      }

      if (adapter->OperStatus !=
          IfOperStatusUp)
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

    ::freeifaddrs(
        interfaces);

    return connected;

#endif
  }

  std::string NativeNetwork::host_name() const
  {
#if defined(_WIN32)

    std::array<
        char,
        MAX_COMPUTERNAME_LENGTH + 1>
        value{};

    DWORD size =
        static_cast<DWORD>(
            value.size());

    return GetComputerNameA(
               value.data(),
               &size) != 0
               ? std::string(
                     value.data(),
                     size)
               : std::string{};

#else

    std::array<char, 256> value{};

    return ::gethostname(
               value.data(),
               value.size()) == 0
               ? value.data()
               : std::string{};

#endif
  }

  std::vector<LocalNetworkAddress>
  NativeNetwork::local_addresses() const
  {
    std::vector<LocalNetworkAddress> addresses;

#if defined(_WIN32)

    std::vector<unsigned char> buffer =
        adapter_buffer();

    if (buffer.empty())
    {
      return addresses;
    }

    const auto *adapter =
        reinterpret_cast<
            const IP_ADAPTER_ADDRESSES *>(
            buffer.data());

    for (;
         adapter != nullptr;
         adapter = adapter->Next)
    {
      if (!is_usable_adapter(*adapter) ||
          adapter->OperStatus !=
              IfOperStatusUp)
      {
        continue;
      }

      for (const IP_ADAPTER_UNICAST_ADDRESS *address =
               adapter->FirstUnicastAddress;
           address != nullptr;
           address = address->Next)
      {
        if (address->Address.lpSockaddr ==
            nullptr)
        {
          continue;
        }

        const int family =
            address->Address.lpSockaddr->sa_family;

        if (family != AF_INET &&
            family != AF_INET6)
        {
          continue;
        }

        const std::string value =
            address_text(
                address->Address.lpSockaddr);

        if (!value.empty())
        {
          addresses.push_back(
              LocalNetworkAddress{
                  family == AF_INET
                      ? LocalAddressFamily::IPv4
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
          is_loopback_address(
              *interface->ifa_addr))
      {
        continue;
      }

      const int family =
          interface->ifa_addr->sa_family;

      std::string value =
          address_text(
              *interface->ifa_addr);

      if (value.empty())
      {
        continue;
      }

      if (family == AF_INET6)
      {
        const auto &ipv6 =
            reinterpret_cast<
                const sockaddr_in6 &>(
                *interface->ifa_addr);

        if (IN6_IS_ADDR_LINKLOCAL(
                &ipv6.sin6_addr) != 0)
        {
          value += "%";
          value += interface->ifa_name;
        }
      }

      addresses.push_back(
          LocalNetworkAddress{
              family == AF_INET
                  ? LocalAddressFamily::IPv4
                  : LocalAddressFamily::IPv6,
              interface->ifa_name,
              std::move(value)});
    }

    ::freeifaddrs(
        interfaces);

#endif

    return addresses;
  }

  std::string NativeNetwork::primary_ipv4() const
  {
#if defined(__linux__)

    const int descriptor =
        ::socket(
            AF_INET,
            SOCK_DGRAM,
            0);

    if (descriptor >= 0)
    {
      sockaddr_in destination{};

      destination.sin_family =
          AF_INET;

      destination.sin_port =
          htons(53);

      const bool connected =
          ::inet_pton(
              AF_INET,
              "1.1.1.1",
              &destination.sin_addr) == 1 &&
          ::connect(
              descriptor,
              reinterpret_cast<const sockaddr *>(
                  &destination),
              sizeof(destination)) == 0;

      sockaddr_in source{};

      socklen_t source_size =
          sizeof(source);

      std::array<char, INET_ADDRSTRLEN> value{};

      if (connected &&
          ::getsockname(
              descriptor,
              reinterpret_cast<sockaddr *>(
                  &source),
              &source_size) == 0 &&
          ::inet_ntop(
              AF_INET,
              &source.sin_addr,
              value.data(),
              value.size()) != nullptr)
      {
        ::close(descriptor);

        return value.data();
      }

      ::close(descriptor);
    }

#endif

    return Network::primary_ipv4();
  }

  NetworkCapability
  NativeNetwork::network_capability() const
  {
    NetworkCapability capability =
        Network::network_capability();

#if defined(__linux__)

    capability.managed_network_capability =
        wifi_hotspot_supported()
            ? ManagedNetworkCapability::Available
            : ManagedNetworkCapability::Unavailable;

#endif

    if (capability.state ==
        NetworkState::Unavailable)
    {
      return capability;
    }

#if defined(__linux__)

    const std::filesystem::path interface_path =
        std::filesystem::path(
            "/sys/class/net") /
        capability.primary_interface;

    if (std::filesystem::exists(
            interface_path / "wireless"))
    {
      capability.interface_type =
          NetworkInterfaceType::Wifi;
    }
    else
    {
      std::ifstream type_file(
          interface_path / "type");

      unsigned int hardware_type = 0;

      type_file >>
          hardware_type;

      if (hardware_type == 1)
      {
        capability.interface_type =
            NetworkInterfaceType::Ethernet;
      }
      else if (hardware_type == 772)
      {
        capability.interface_type =
            NetworkInterfaceType::Loopback;
      }
      else if (type_file)
      {
        capability.interface_type =
            NetworkInterfaceType::Other;
      }
    }

    capability.local_subnet =
        ipv4_subnet(
            capability.primary_interface,
            capability.primary_ipv4);

#endif

    return capability;
  }

  bool NativeNetwork::tcp_listener(
      const std::string &ipv4,
      const std::uint16_t port) const noexcept
  {
#if defined(_WIN32)

    static_cast<void>(ipv4);
    static_cast<void>(port);

    return false;

#else

    const int descriptor =
        ::socket(
            AF_INET,
            SOCK_STREAM,
            0);

    if (descriptor < 0)
    {
      return false;
    }

    const int flags =
        ::fcntl(
            descriptor,
            F_GETFL,
            0);

    if (flags < 0 ||
        ::fcntl(
            descriptor,
            F_SETFL,
            flags | O_NONBLOCK) != 0)
    {
      ::close(descriptor);

      return false;
    }

    sockaddr_in address{};

    address.sin_family =
        AF_INET;

    address.sin_port =
        htons(port);

    if (::inet_pton(
            AF_INET,
            ipv4.c_str(),
            &address.sin_addr) != 1)
    {
      ::close(descriptor);

      return false;
    }

    const int connected =
        ::connect(
            descriptor,
            reinterpret_cast<const sockaddr *>(
                &address),
            sizeof(address));

    bool ready =
        connected == 0;

    if (!ready &&
        errno == EINPROGRESS)
    {
      pollfd poll_descriptor{
          descriptor,
          POLLOUT,
          0};

      if (::poll(
              &poll_descriptor,
              1,
              200) > 0)
      {
        int error = 0;

        socklen_t size =
            sizeof(error);

        ready =
            ::getsockopt(
                descriptor,
                SOL_SOCKET,
                SO_ERROR,
                &error,
                &size) == 0 &&
            error == 0;
      }
    }

    ::close(descriptor);

    return ready;

#endif
  }

} // namespace softadastra
