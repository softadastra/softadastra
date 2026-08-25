#ifndef SOFTADASTRA_HOST_REMOTE_REACHABILITY_HPP
#define SOFTADASTRA_HOST_REMOTE_REACHABILITY_HPP
#include "host/LocalGatewayTargetResolver.hpp"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include "host/NativeSocket.hpp"
namespace softadastra {
struct RemoteEndpoint { std::string address; std::uint16_t port{0}; [[nodiscard]] bool valid() const noexcept { return !address.empty() && port != 0; } };
enum class RemoteReachabilityState { Disabled, Connecting, Ready, Degraded };
/** Maintains one outbound TCP connection to a relay. It never binds or listens. */
class RemoteReachability {
public:
  explicit RemoteReachability(LocalGatewayTargetResolver &) noexcept;
  ~RemoteReachability();
  RemoteReachability(const RemoteReachability &) = delete;
  void configure(RemoteEndpoint endpoint);
  void disable() noexcept;
  [[nodiscard]] RemoteReachabilityState state() const noexcept;
  [[nodiscard]] RemoteEndpoint endpoint() const;
private:
  void run() noexcept;
  LocalGatewayTargetResolver &resolver_;
  mutable std::mutex mutex_;
  std::condition_variable wake_;
  RemoteEndpoint endpoint_;
  RemoteReachabilityState state_{RemoteReachabilityState::Disabled};
  std::atomic_bool stopping_{false};
  std::atomic<NativeSocket> socket_{InvalidSocket};
  std::thread thread_;
};
}
#endif
