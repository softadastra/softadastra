#ifndef SOFTADASTRA_WEBUI_WEB_UI_SERVER_HPP
#define SOFTADASTRA_WEBUI_WEB_UI_SERVER_HPP

#include "control/ControlClient.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

namespace softadastra
{
  /** Loopback-only HTTP interface backed exclusively by ControlClient. */
  class WebUiServer
  {
  public:
    explicit WebUiServer(ControlClient &client) noexcept;
    ~WebUiServer();
    WebUiServer(const WebUiServer &) = delete;
    WebUiServer &operator=(const WebUiServer &) = delete;

    [[nodiscard]] bool start(std::uint16_t port = 0);
    void stop() noexcept;
    [[nodiscard]] std::uint16_t port() const noexcept { return port_.load(); }

  private:
    void run(std::uint16_t requested_port) noexcept;
    void publish_startup(bool success, std::uint16_t port) noexcept;

    ControlClient &client_;
    std::atomic_bool stopping_{false};
    std::atomic_uint16_t port_{0};
    std::mutex startup_mutex_;
    std::condition_variable startup_condition_;
    bool startup_complete_{false};
    bool startup_success_{false};
    std::thread worker_;
  };
}

#endif // SOFTADASTRA_WEBUI_WEB_UI_SERVER_HPP
