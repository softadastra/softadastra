#ifndef SOFTADASTRA_PLATFORM_MANAGED_NETWORK_HPP
#define SOFTADASTRA_PLATFORM_MANAGED_NETWORK_HPP

#include "platform/Network.hpp"

#include <string>

namespace softadastra
{
  enum class ManagedNetworkState { Stopped, Running };
  enum class ManagedNetworkStartResult { Started, AlreadyRunning, Unavailable, WouldDisruptConnection, Failed };
  struct ManagedNetworkStatus
  {
    ManagedNetworkCapability capability{ManagedNetworkCapability::Unavailable};
    ManagedNetworkState state{ManagedNetworkState::Stopped};
    std::string interface_name;
    std::string ipv4;
    std::string ssid;
  };
  class ManagedNetwork
  {
  public:
    virtual ~ManagedNetwork() = default;
    [[nodiscard]] virtual ManagedNetworkStatus status() const = 0;
    [[nodiscard]] virtual ManagedNetworkStartResult start() = 0;
    virtual bool stop() = 0;
  };
  class UnavailableManagedNetwork final : public ManagedNetwork
  {
  public:
    [[nodiscard]] ManagedNetworkStatus status() const override { return {}; }
    [[nodiscard]] ManagedNetworkStartResult start() override { return ManagedNetworkStartResult::Unavailable; }
    bool stop() override { return false; }
  };
  [[nodiscard]] const char *managed_network_state_name(ManagedNetworkState state) noexcept;
}
#endif
