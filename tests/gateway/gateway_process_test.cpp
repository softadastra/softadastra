#include "control/ControlServer.hpp"
#include "control/LocalControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <thread>

#if defined(__linux__)
#include <arpa/inet.h>
#include <csignal>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
  int listener(std::uint16_t &port)
  {
    const int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (descriptor < 0 || ::bind(descriptor, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0 || ::listen(descriptor, 1) != 0)
    {
      if (descriptor >= 0) ::close(descriptor);
      return -1;
    }
    socklen_t size = sizeof(address);
    if (::getsockname(descriptor, reinterpret_cast<sockaddr *>(&address), &size) != 0)
    {
      ::close(descriptor);
      return -1;
    }
    port = ntohs(address.sin_port);
    return descriptor;
  }

  std::string request(std::uint16_t port, const std::string &message)
  {
    const int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (descriptor < 0 || ::connect(descriptor, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0)
    {
      if (descriptor >= 0) ::close(descriptor);
      return {};
    }
    static_cast<void>(::send(descriptor, message.data(), message.size(), 0));
    static_cast<void>(::shutdown(descriptor, SHUT_WR));
    std::string response;
    std::array<char, 1024> buffer{};
    ssize_t received = 0;
    while ((received = ::recv(descriptor, buffer.data(), buffer.size(), 0)) > 0) response.append(buffer.data(), static_cast<std::size_t>(received));
    ::close(descriptor);
    return response;
  }

  TEST(GatewayProcessTest, RoutesThroughLocalControlAndStopsOnSignal)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer control(service);
    const softadastra::SoftwareId id("stable-id");
    std::uint16_t backend_port{};
    const int backend = listener(backend_port);
    ASSERT_GE(backend, 0);
    ASSERT_TRUE(service.register_software(id, softadastra::ProcessSpec("app"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, backend_port), std::nullopt, "phone-test"));
    host.state().find_software(id)->set_state(softadastra::SoftwareState::Running);

    const auto directory = std::filesystem::temp_directory_path() / ("softadastra-gateway-test-" + std::to_string(::getpid()));
    const auto socket = directory / "control.sock";
    std::filesystem::create_directories(directory);
    softadastra::LocalControlServer local_control(control, socket);
    ASSERT_TRUE(local_control.start());
    std::atomic_bool serving{true};
    std::thread control_thread([&] { while (serving) { static_cast<void>(local_control.process_pending()); std::this_thread::sleep_for(std::chrono::milliseconds(1)); } });

    std::uint16_t gateway_port{};
    const int reserved = listener(gateway_port);
    ASSERT_GE(reserved, 0);
    ASSERT_EQ(::close(reserved), 0);
    const std::string listen = "127.0.0.1:" + std::to_string(gateway_port);
    const pid_t process = ::fork();
    ASSERT_GE(process, 0);
    if (process == 0)
    {
      ::execl(SOFTADASTRA_GATEWAY_EXECUTABLE, SOFTADASTRA_GATEWAY_EXECUTABLE, "--listen", listen.c_str(), "--control", socket.c_str(), nullptr);
      ::_exit(127);
    }

    std::string received;
    std::thread backend_thread([&]
    {
      const int client = ::accept(backend, nullptr, nullptr);
      std::array<char, 1024> buffer{};
      const ssize_t size = ::recv(client, buffer.data(), buffer.size(), 0);
      if (size > 0) received.assign(buffer.data(), static_cast<std::size_t>(size));
      constexpr std::string_view response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
      static_cast<void>(::send(client, response.data(), response.size(), 0));
      ::close(client);
      ::close(backend);
    });

    std::string response;
    for (int attempt = 0; attempt < 100 && response.empty(); ++attempt)
    {
      response = request(gateway_port, "GET /path HTTP/1.1\r\nHost: phone-test\r\n\r\n");
      if (response.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(response.starts_with("HTTP/1.1 200 OK"));
    EXPECT_NE(received.find("GET /path HTTP/1.1"), std::string::npos);
    EXPECT_TRUE(request(gateway_port, "GET / HTTP/1.1\r\nHost: unknown\r\n\r\n").starts_with("HTTP/1.1 404"));

    EXPECT_EQ(::kill(process, SIGTERM), 0);
    int status = 0;
    EXPECT_EQ(::waitpid(process, &status, 0), process);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
    backend_thread.join();
    serving = false;
    control_thread.join();
    local_control.stop();
    std::filesystem::remove_all(directory);
  }
}
#else
TEST(GatewayProcessTest, RequiresLinux)
{
  GTEST_SKIP();
}
#endif
