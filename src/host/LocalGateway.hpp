#ifndef SOFTADASTRA_HOST_LOCAL_GATEWAY_HPP
#define SOFTADASTRA_HOST_LOCAL_GATEWAY_HPP

#include <cstdint>
#include <string>

namespace softadastra { class HostService;
enum class LocalGatewayState { Stopped, Running, Failed };
struct LocalGatewayStatus { LocalGatewayState state{LocalGatewayState::Stopped}; std::string address; std::uint16_t port{}; };

class LocalGateway {
public:
  explicit LocalGateway(HostService &host_service) noexcept;
  ~LocalGateway();
  bool start(std::string address, std::uint16_t port);
  // On success this object takes ownership of fd; on failure fd remains owned by the caller.
  bool start_from_socket(int fd);
  void stop() noexcept;
  [[nodiscard]] LocalGatewayStatus status() const;
private:
  void run() noexcept;
  HostService &host_service_; int listener_{-1}; LocalGatewayStatus status_{}; bool stopping_{false};
  class Thread; Thread *thread_{nullptr};
}; }
#endif
