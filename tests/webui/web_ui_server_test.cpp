#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"
#include "webui/WebUiServer.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>

#include <gtest/gtest.h>

namespace
{
  TEST(WebUiServerTest, AsioAcceptLifetimeBaseline)
  {
    asio::io_context context;
    asio::ip::tcp::acceptor acceptor(context);
    std::error_code error;
    acceptor.open(asio::ip::tcp::v4(), error);
    ASSERT_FALSE(error);
    acceptor.bind({asio::ip::make_address("127.0.0.1"), 0}, error);
    ASSERT_FALSE(error);
    acceptor.listen(asio::socket_base::max_listen_connections, error);
    ASSERT_FALSE(error);
    const auto port = acceptor.local_endpoint().port();
    std::thread server([&] {
      asio::ip::tcp::socket socket(context);
      acceptor.accept(socket, error);
    });
    asio::io_context client_context;
    asio::ip::tcp::socket client(client_context);
    client.connect({asio::ip::make_address("127.0.0.1"), port}, error);
    EXPECT_FALSE(error);
    server.join();
  }

  std::string get(std::uint16_t port, const std::string &request)
  {
    asio::io_context context;
    asio::ip::tcp::socket socket(context);
    socket.connect({asio::ip::make_address("127.0.0.1"), port});
    asio::write(socket, asio::buffer(request));
    std::string response;
    std::array<char, 4096> buffer{};
    std::error_code error;
    for (;;)
    {
      const auto count = socket.read_some(asio::buffer(buffer), error);
      response.append(buffer.data(), count);
      if (error) break;
    }
    return response;
  }

  TEST(WebUiServerTest, ServesLoopbackControlBackedHostAndSoftwareRoutes)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    ASSERT_TRUE(service.register_software(
        softadastra::SoftwareId("stable-id"), softadastra::ProcessSpec("app"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080),
        std::nullopt, "phone-test"));

    softadastra::WebUiServer ui(client);
    ASSERT_TRUE(ui.start());
    ASSERT_NE(ui.port(), 0);

    const auto host_response = get(ui.port(), "GET /api/host HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(host_response.find("200 OK"), std::string::npos);
    EXPECT_NE(host_response.find("\"status\":\"running\""), std::string::npos);

    const auto list_response = get(ui.port(), "GET /api/software HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(list_response.find("\"name\":\"phone-test\""), std::string::npos);
    EXPECT_NE(list_response.find("http:8080"), std::string::npos);

    const auto access_response = get(ui.port(), "GET /api/software/phone-test/access HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(access_response.find("200 OK"), std::string::npos);

    const auto unknown_response = get(ui.port(), "GET /api/software/missing HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(unknown_response.find("404 Not Found"), std::string::npos);

    const auto remove_response = get(ui.port(), "DELETE /api/software/phone-test HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(remove_response.find("200 OK"), std::string::npos);
    ui.stop();
  }

  TEST(WebUiServerTest, StopsIdempotentlyAndRestartsWithoutRequests)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::WebUiServer ui(client);

    for (int cycle = 0; cycle < 50; ++cycle)
    {
      ASSERT_TRUE(ui.start());
      EXPECT_NE(ui.port(), 0);
      ui.stop();
      ui.stop();
      EXPECT_EQ(ui.port(), 0);
    }
  }
}
