/** @file FirewallHelper.hpp */
#ifndef SOFTADASTRA_PLATFORM_FIREWALL_HELPER_HPP
#define SOFTADASTRA_PLATFORM_FIREWALL_HELPER_HPP

#include "platform/LocalFirewall.hpp"
#include "platform/Network.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace softadastra
{
  enum class FirewallHelperCommand { Status, Modify };

  class FirewallHelper final
  {
  public:
    FirewallHelper(const Network &network, LocalFirewall &firewall) noexcept;

    [[nodiscard]] int run(
        FirewallHelperCommand command,
        const std::vector<std::string> &arguments,
        std::ostream &output) const;

  private:
    const Network &network_;
    LocalFirewall &firewall_;
  };
}
#endif
