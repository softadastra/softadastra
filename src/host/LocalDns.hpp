#ifndef SOFTADASTRA_HOST_LOCAL_DNS_HPP
#define SOFTADASTRA_HOST_LOCAL_DNS_HPP

#include <cstdint>
#include <string>

namespace softadastra
{
  enum class LocalDnsState { Stopped, Running, Failed };
  struct LocalDnsStatus { LocalDnsState state{LocalDnsState::Stopped}; std::string address; std::uint16_t port{}; };

  class LocalDns
  {
  public:
    LocalDns() = default;
    ~LocalDns();
    LocalDns(const LocalDns &) = delete;
    LocalDns &operator=(const LocalDns &) = delete;
    bool start(std::string address, std::uint16_t port);
    void stop() noexcept;
    [[nodiscard]] LocalDnsStatus status() const;
  private:
    void run() noexcept;
    int descriptor_{-1};
    LocalDnsStatus status_{};
    bool stopping_{false};
    class Thread;
    Thread *thread_{nullptr};
  };
}
#endif
