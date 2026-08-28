#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "host/LocalGateway.hpp"
#include "platform/NativePlatform.hpp"
#include "software/AccessPoint.hpp"
#include <gtest/gtest.h>
#include <array>
#include <thread>
#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace
{
#if defined(__linux__)
  int listener(std::uint16_t &port)
  {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (fd < 0 || ::bind(fd, reinterpret_cast<sockaddr *>(&a), sizeof(a)) || ::listen(fd, 1))
    {
      if (fd >= 0)
        ::close(fd);
      return -1;
    }
    socklen_t n = sizeof(a);
    if (::getsockname(fd, reinterpret_cast<sockaddr *>(&a), &n))
    {
      ::close(fd);
      return -1;
    }
    port = ntohs(a.sin_port);
    return fd;
  }
  std::string call(std::uint16_t port, const std::string &q)
  {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (fd < 0 || ::connect(fd, reinterpret_cast<sockaddr *>(&a), sizeof(a)))
      return {};
    ::send(fd, q.data(), q.size(), 0);
    ::shutdown(fd, SHUT_WR);
    std::string r;
    std::array<char, 4096> b{};
    for (;;)
    {
      const auto received = ::recv(fd, b.data(), b.size(), 0);
      if (received <= 0)
        break;
      r.append(b.data(), static_cast<std::size_t>(received));
    }
    ::close(fd);
    return r;
  }
  TEST(LocalGatewayTest, RoutesShortCanonicalAndBodyAndRejectsUnavailable)
  {
    std::uint16_t backend_port{};
    const int backend = listener(backend_port);
    std::string seen;
    std::thread server([&]
                       {int c=::accept(backend,nullptr,nullptr);std::array<char,4096>b{};for(;;){const auto received=::recv(c,b.data(),b.size(),0);if(received<=0)break;seen.append(b.data(),static_cast<std::size_t>(received));}const std::string response="HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nX-Test: ok\r\nContent-Length: 2\r\n\r\nOK";::send(c,response.data(),response.size(),0);::close(c);::close(backend); });
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    const softadastra::SoftwareId id("stable");
    ASSERT_TRUE(service.register_software(id, softadastra::ProcessSpec("app"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, backend_port), std::nullopt, "phone-test"));
    host.state().find_software(id)->set_state(softadastra::SoftwareState::Running);
    softadastra::LocalGateway gateway(service);
    ASSERT_TRUE(gateway.start("127.0.0.1", 0));
    const auto response = call(gateway.status().port, "POST /a?q=1 HTTP/1.1\r\nHost: phone-test.softadastra.home.arpa\r\nConnection: keep-alive, X-Remove\r\nX-Remove: yes\r\nContent-Length: 4\r\n\r\ntest");
    EXPECT_TRUE(response.starts_with("HTTP/1.1 200 OK"));
    EXPECT_NE(response.find("X-Test: ok"), std::string::npos);
    EXPECT_EQ(response.find("keep-alive"), std::string::npos);
    gateway.stop();
    server.join();
    EXPECT_NE(seen.find("POST /a?q=1 HTTP/1.1"), std::string::npos);
    EXPECT_NE(seen.find("Host: phone-test.softadastra.home.arpa"), std::string::npos);
    EXPECT_NE(seen.find("\r\n\r\ntest"), std::string::npos);
    EXPECT_EQ(seen.find("X-Remove"), std::string::npos);
  }
  TEST(LocalGatewayTest, ReturnsNotFoundAndUnavailable)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::LocalGateway gateway(service);
    ASSERT_TRUE(gateway.start("127.0.0.1", 0));
    EXPECT_TRUE(call(gateway.status().port, "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n").starts_with("HTTP/1.1 404"));
    ASSERT_TRUE(service.register_software(softadastra::SoftwareId("stable"), softadastra::ProcessSpec("app"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 1), std::nullopt, "phone-test"));
    EXPECT_TRUE(call(gateway.status().port, "GET / HTTP/1.1\r\nHost: phone-test:18080\r\n\r\n").starts_with("HTTP/1.1 503"));
    gateway.stop();
  }
  TEST(LocalGatewayTest, RejectsInvalidRequestsAndReflectsDynamicState)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::LocalGateway gateway(service);
    ASSERT_TRUE(gateway.start("127.0.0.1", 0));
    const auto port = gateway.status().port;
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\n\r\n").starts_with("HTTP/1.1 400"));
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: phone-test\r\nTransfer-Encoding: chunked\r\n\r\n").starts_with("HTTP/1.1 400"));
    const softadastra::SoftwareId no_access("no-access");
    ASSERT_TRUE(service.register_software(no_access, softadastra::ProcessSpec("app"), std::nullopt, std::nullopt, "no-access"));
    host.state().find_software(no_access)->set_state(softadastra::SoftwareState::Running);
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: no-access\r\n\r\n").starts_with("HTTP/1.1 404"));
    const softadastra::SoftwareId https("https");
    ASSERT_TRUE(service.register_software(https, softadastra::ProcessSpec("app"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Https, 9), std::nullopt, "https"));
    host.state().find_software(https)->set_state(softadastra::SoftwareState::Running);
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: https\r\n\r\n").starts_with("HTTP/1.1 404"));
    const softadastra::SoftwareId absent("absent");
    ASSERT_TRUE(service.register_software(absent, softadastra::ProcessSpec("app"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 1), std::nullopt, "absent"));
    host.state().find_software(absent)->set_state(softadastra::SoftwareState::Running);
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: absent\r\n\r\n").starts_with("HTTP/1.1 502"));
    host.state().find_software(absent)->set_state(softadastra::SoftwareState::Failed);
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: absent\r\n\r\n").starts_with("HTTP/1.1 503"));
    gateway.stop();
  }
  TEST(LocalGatewayTest, ResolvesEachRequestDynamically)
  {
    std::uint16_t first_port{}, second_port{};
    const int first = listener(first_port), second = listener(second_port);
    ASSERT_GE(first, 0);
    ASSERT_GE(second, 0);
    auto backend = [](int fd, const char *body)
    {int c=::accept(fd,nullptr,nullptr);std::array<char,1024>b{};static_cast<void>(::recv(c,b.data(),b.size(),0));const std::string r=std::string("HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\n")+body;::send(c,r.data(),r.size(),0);::close(c);::close(fd); };
    std::thread one(backend, first, "A"), two(backend, second, "B");
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    const softadastra::SoftwareId id("stable");
    ASSERT_TRUE(service.register_software(id, softadastra::ProcessSpec("app"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, first_port), std::nullopt, "phone-test"));
    auto *entry = host.state().find_software(id);
    entry->set_state(softadastra::SoftwareState::Running);
    softadastra::LocalGateway gateway(service);
    ASSERT_TRUE(gateway.start("127.0.0.1", 0));
    const auto port = gateway.status().port;
    EXPECT_NE(call(port, "GET / HTTP/1.1\r\nHost: phone-test\r\n\r\n").find("\r\n\r\nA"), std::string::npos);
    entry->set_access_point(softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, second_port));
    EXPECT_NE(call(port, "GET / HTTP/1.1\r\nHost: phone-test\r\n\r\n").find("\r\n\r\nB"), std::string::npos);
    entry->set_state(softadastra::SoftwareState::Stopped);
    EXPECT_TRUE(host.state().remove_software(id));
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: phone-test\r\n\r\n").starts_with("HTTP/1.1 404"));
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: unknown\r\n\r\n").starts_with("HTTP/1.1 404"));
    gateway.stop();
    one.join();
    two.join();
  }
  TEST(LocalGatewayTest, AdoptsListeningSocketAndClosesItOnStop)
  {
    std::uint16_t port{};
    const int fd = listener(port);
    ASSERT_GE(fd, 0);
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::LocalGateway gateway(service);
    ASSERT_TRUE(gateway.start_from_socket(fd));
    EXPECT_EQ(gateway.status().port, port);
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: unknown\r\n\r\n").starts_with("HTTP/1.1 404"));
    gateway.stop();
    EXPECT_TRUE(call(port, "GET / HTTP/1.1\r\nHost: unknown\r\n\r\n").empty());
  }
  TEST(LocalGatewayTest, RejectsInvalidAndSecondExternalSocketWithoutTakingOwnership)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::LocalGateway gateway(service);
    EXPECT_FALSE(gateway.start_from_socket(-1));
    std::uint16_t first_port{}, second_port{};
    const int first = listener(first_port), second = listener(second_port);
    ASSERT_GE(first, 0);
    ASSERT_GE(second, 0);
    ASSERT_TRUE(gateway.start_from_socket(first));
    EXPECT_FALSE(gateway.start_from_socket(second));
    EXPECT_EQ(::close(second), 0);
    gateway.stop();
  }
#endif
}
