/** @file FirewallHelper.cpp */
#include "platform/FirewallHelper.hpp"

#include <charconv>
#include <optional>
#include <regex>

#if !defined(_WIN32)
#include <arpa/inet.h>
#endif

namespace
{
  std::optional<softadastra::LocalFirewallRule> valid_rule(
      const softadastra::Network &network,
      const std::vector<std::string> &arguments,
      const std::size_t offset)
  {
#if defined(_WIN32)
    static_cast<void>(network); static_cast<void>(arguments); static_cast<void>(offset);
    return std::nullopt;
#else
    if (arguments.size() != offset + 3) return std::nullopt;
    unsigned int port{};
    const auto &port_text = arguments[offset];
    const auto converted = std::from_chars(port_text.data(), port_text.data() + port_text.size(), port);
    if (converted.ec != std::errc{} || converted.ptr != port_text.data() + port_text.size() || port == 0 || port > 65535) return std::nullopt;
    const auto &subnet = arguments[offset + 1]; const auto slash = subnet.find('/'); in_addr address{};
    if (slash == std::string::npos || inet_pton(AF_INET, subnet.substr(0, slash).c_str(), &address) != 1) return std::nullopt;
    unsigned int prefix{}; const auto prefix_text = subnet.substr(slash + 1);
    const auto prefix_result = std::from_chars(prefix_text.data(), prefix_text.data() + prefix_text.size(), prefix);
    if (prefix_result.ec != std::errc{} || prefix_result.ptr != prefix_text.data() + prefix_text.size() || prefix > 32) return std::nullopt;
    const auto &tag = arguments[offset + 2];
    if (!std::regex_match(tag, std::regex("softadastra:[0-9a-f]+"))) return std::nullopt;
    const auto capability = network.network_capability();
    if (capability.local_network_state != softadastra::LocalNetworkState::Existing || capability.local_subnet != subnet) return std::nullopt;
    return softadastra::LocalFirewallRule{tag, subnet, static_cast<std::uint16_t>(port)};
#endif
  }

  const char *status_name(const softadastra::LocalFirewallResult result)
  {
    switch (result)
    {
    case softadastra::LocalFirewallResult::Open: return "allowed";
    case softadastra::LocalFirewallResult::PermissionRequired: return "blocked";
    case softadastra::LocalFirewallResult::Disabled: return "disabled";
    case softadastra::LocalFirewallResult::Unsupported: return "unsupported";
    case softadastra::LocalFirewallResult::Failed: return "error";
    }
    return "error";
  }
}

namespace softadastra
{
  FirewallHelper::FirewallHelper(const Network &network, LocalFirewall &firewall) noexcept
      : network_(network), firewall_(firewall) {}

  int FirewallHelper::run(
      const FirewallHelperCommand command,
      const std::vector<std::string> &arguments,
      std::ostream &output) const
  {
    if (command == FirewallHelperCommand::Status)
    {
      const auto rule = valid_rule(network_, arguments, 0);
      if (!rule) return 2;
      output << status_name(firewall_.status(*rule)) << '\n';
      return 0;
    }
    if (arguments.size() != 4 || (arguments[0] != "allow-local-tcp" && arguments[0] != "deny-owned-local-tcp")) return 2;
    const auto rule = valid_rule(network_, arguments, 1);
    if (!rule) return 2;
    const auto result = arguments[0] == "allow-local-tcp" ? firewall_.allow(*rule) : firewall_.deny(*rule);
    return result == LocalFirewallResult::Open || result == LocalFirewallResult::Disabled ? 0 : 1;
  }
}
