#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativeDataDirectory.hpp"
#include "platform/NativePlatform.hpp"
#include "software/ProjectConfig.hpp"
#include "webui/WebUiServer.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <thread>

namespace
{
  std::string request(std::uint16_t port, const std::string &request)
  {
    asio::io_context context;
    asio::ip::tcp::socket socket(context);

    socket.connect(
        {asio::ip::make_address("127.0.0.1"), port});

    asio::write(socket, asio::buffer(request));

    std::array<char, 4096> buffer{};
    std::string response;
    std::error_code error;

    for (;;)
    {
      const auto count = socket.read_some(asio::buffer(buffer), error);
      response.append(buffer.data(), count);

      if (error)
      {
        break;
      }
    }

    return response;
  }

  std::string get(std::uint16_t port, const std::string &path)
  {
    return request(
        port,
        "GET " + path + " HTTP/1.1\r\n"
                        "Host: localhost\r\n"
                        "\r\n");
  }

  std::string post(std::uint16_t port, const std::string &path)
  {
    return request(
        port,
        "POST " + path + " HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "\r\n");
  }

  std::string post(
      std::uint16_t port,
      const std::string &path,
      const std::string &body)
  {
    return request(
        port,
        "POST " + path + " HTTP/1.1\r\n"
                         "Host: localhost\r\n"
                         "Content-Type: application/json\r\n"
                         "Content-Length: " +
            std::to_string(body.size()) +
            "\r\n\r\n" +
            body);
  }

  std::string put(
      std::uint16_t port,
      const std::string &path,
      const std::string &body)
  {
    return request(
        port,
        "PUT " + path + " HTTP/1.1\r\n"
                        "Host: localhost\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: " +
            std::to_string(body.size()) +
            "\r\n\r\n" +
            body);
  }

  std::string remove(std::uint16_t port, const std::string &path)
  {
    return request(
        port,
        "DELETE " + path + " HTTP/1.1\r\n"
                           "Host: localhost\r\n"
                           "\r\n");
  }

  std::string long_running_command()
  {
#if defined(_WIN32)
    return "ping -n 30 127.0.0.1 >NUL";
#else
    return "sleep 30";
#endif
  }

  std::filesystem::path temporary_project()
  {
    return std::filesystem::temp_directory_path() /
           ("softadastra-web-ui-" +
            std::to_string(
                std::chrono::steady_clock::now()
                    .time_since_epoch()
                    .count()));
  }

  TEST(WebUiServerTest, AcceptsLoopbackConnections)
  {
    asio::io_context context;
    asio::ip::tcp::acceptor acceptor(context);
    std::error_code error;

    acceptor.open(asio::ip::tcp::v4(), error);
    ASSERT_FALSE(error);

    acceptor.bind(
        {asio::ip::make_address("127.0.0.1"), 0},
        error);
    ASSERT_FALSE(error);

    acceptor.listen(
        asio::socket_base::max_listen_connections,
        error);
    ASSERT_FALSE(error);

    const auto port = acceptor.local_endpoint().port();

    std::thread server(
        [&]
        {
          asio::ip::tcp::socket socket(context);
          acceptor.accept(socket, error);
        });

    asio::io_context client_context;
    asio::ip::tcp::socket client(client_context);

    client.connect(
        {asio::ip::make_address("127.0.0.1"), port},
        error);

    EXPECT_FALSE(error);

    server.join();
  }

  TEST(WebUiServerTest, ServesWebInterface)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(
        host,
        platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::WebUiServer ui(client);

    ASSERT_TRUE(ui.start());
    ASSERT_NE(ui.port(), 0);

    const auto page = get(ui.port(), "/");

    EXPECT_NE(page.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(page.find("Content-Type: text/html"), std::string::npos);
    EXPECT_NE(page.find("<title>Softadastra</title>"), std::string::npos);
    EXPECT_NE(page.find("<h2>Applications</h2>"), std::string::npos);
    EXPECT_NE(page.find("<h2>Host</h2>"), std::string::npos);
    EXPECT_NE(page.find("/style.css"), std::string::npos);
    EXPECT_NE(page.find("/app.js"), std::string::npos);

    const auto script = get(ui.port(), "/app.js");

    EXPECT_NE(script.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(script.find("async function api"), std::string::npos);
    EXPECT_NE(script.find("access_points"), std::string::npos);

    const auto style = get(ui.port(), "/style.css");

    EXPECT_NE(style.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(style.find("Content-Type: text/css"), std::string::npos);

    ui.stop();
  }

  TEST(WebUiServerTest, ExposesHostAndSoftwareInventory)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(
        host,
        platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::WebUiServer ui(client);

    ASSERT_TRUE(ui.start());

    const auto empty = get(ui.port(), "/api/software");

    EXPECT_NE(empty.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(empty.find("\r\n\r\n[]"), std::string::npos);

    ASSERT_TRUE(
        service.register_software(
            softadastra::SoftwareId("stable-id"),
            softadastra::ProcessSpec("app"),
            softadastra::AccessPoint::create(
                softadastra::AccessProtocol::Http,
                8080),
            std::nullopt,
            "phone-test"));

    const auto host_response = get(ui.port(), "/api/host");

    EXPECT_NE(
        host_response.find("HTTP/1.1 200 OK"),
        std::string::npos);
    EXPECT_NE(
        host_response.find("\"status\":\"running\""),
        std::string::npos);

    const auto software = get(ui.port(), "/api/software");

    EXPECT_NE(
        software.find("\"name\":\"phone-test\""),
        std::string::npos);
    EXPECT_NE(
        software.find("\"access_configured\":\"http:8080\""),
        std::string::npos);

    ui.stop();
  }

  TEST(WebUiServerTest, ReadsProjectConfiguration)
  {
    const auto project = temporary_project();
    const auto command = long_running_command();

    ASSERT_TRUE(std::filesystem::create_directories(project));

    ASSERT_TRUE(
        softadastra::ProjectConfigFile::create(
            project,
            {"Pico",
             command,
             softadastra::AccessPoint::create(
                 softadastra::AccessProtocol::Http,
                 8081),
             {}}));

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(
        host,
        platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);

    softadastra::WebUiServer ui(
        client,
        [project]
        {
          return softadastra::DirectoryChooserResult{
              softadastra::DirectoryChooserStatus::Selected,
              project};
        });

    ASSERT_TRUE(ui.start());

    const auto response = post(
        ui.port(),
        "/api/project-folder");

    EXPECT_NE(response.find("\"name\":\"Pico\""), std::string::npos);
    EXPECT_NE(
        response.find("\"command\":\"" + command + "\""),
        std::string::npos);
    EXPECT_NE(
        response.find("\"access_points\""),
        std::string::npos);

    ui.stop();
    std::filesystem::remove_all(project);
  }

  TEST(WebUiServerTest, ManagesSoftware)
  {
    const auto command = long_running_command();

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(
        host,
        platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::WebUiServer ui(client);

    ASSERT_TRUE(ui.start());

    ASSERT_TRUE(
        service.register_software(
            softadastra::SoftwareId("stable-id"),
            softadastra::ProcessSpec("app"),
            softadastra::AccessPoint::create(
                softadastra::AccessProtocol::Http,
                8080),
            std::nullopt,
            "phone-test"));

    const std::string update_body =
        "{\"name\":\"cloud\","
        "\"project_directory\":\".\","
        "\"command\":\"" +
        command +
        "\","
        "\"access_points\":["
        "{\"protocol\":\"http\",\"port\":\"8080\"},"
        "{\"protocol\":\"ws\",\"port\":\"9090\"}"
        "]}";

    const auto update_response = put(
        ui.port(),
        "/api/software/phone-test",
        update_body);

    EXPECT_NE(
        update_response.find("200 OK"),
        std::string::npos);

    const auto updated = client.software(
        softadastra::SoftwareId("stable-id"));

    ASSERT_TRUE(updated);
    EXPECT_EQ(updated->id().value(), "stable-id");
    EXPECT_EQ(updated->declared_command(), command);
    ASSERT_EQ(updated->access_points().size(), 2U);
    EXPECT_EQ(
        updated->access_points()[1].protocol(),
        softadastra::AccessProtocol::Ws);

    const std::string add_body =
        "{\"name\":\"web-added\","
        "\"project_directory\":\".\","
        "\"command\":\"" +
        command +
        "\","
        "\"access_points\":["
        "{\"protocol\":\"http\",\"port\":\"8081\"},"
        "{\"protocol\":\"ws\",\"port\":\"9091\"}"
        "]}";

    const auto add_response = post(
        ui.port(),
        "/api/software",
        add_body);

    EXPECT_NE(add_response.find("200 OK"), std::string::npos);

    const auto list_response = get(
        ui.port(),
        "/api/software");

    EXPECT_NE(
        list_response.find("\"name\":\"web-added\""),
        std::string::npos);
    EXPECT_NE(
        list_response.find("\"configured\":\"ws:9091\""),
        std::string::npos);

    const auto start_response = post(
        ui.port(),
        "/api/software/web-added/start");

    EXPECT_NE(start_response.find("200 OK"), std::string::npos);

    const auto running_response = get(
        ui.port(),
        "/api/software");

    EXPECT_NE(
        running_response.find(
            "\"name\":\"web-added\",\"state\":\"running\""),
        std::string::npos);

    const auto edit_while_running = put(
        ui.port(),
        "/api/software/web-added",
        add_body);

    EXPECT_NE(
        edit_while_running.find(
            "Stop the application before editing its configuration."),
        std::string::npos);

    EXPECT_NE(
        post(ui.port(), "/api/software/web-added/stop")
            .find("200 OK"),
        std::string::npos);

    EXPECT_NE(
        post(ui.port(), "/api/software/web-added/restart")
            .find("200 OK"),
        std::string::npos);

    EXPECT_NE(
        post(ui.port(), "/api/software/web-added/stop")
            .find("200 OK"),
        std::string::npos);

    EXPECT_NE(
        remove(ui.port(), "/api/software/cloud")
            .find("200 OK"),
        std::string::npos);

    EXPECT_NE(
        remove(ui.port(), "/api/software/web-added")
            .find("200 OK"),
        std::string::npos);

    ui.stop();
  }

  TEST(WebUiServerTest, ServesAndClearsLogs)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(
        host,
        platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::WebUiServer ui(client);

    ASSERT_TRUE(ui.start());

    ASSERT_TRUE(
        service.register_software(
            softadastra::SoftwareId("stable-id"),
            softadastra::ProcessSpec("app"),
            softadastra::AccessPoint::create(
                softadastra::AccessProtocol::Http,
                8080),
            std::nullopt,
            "cloud"));

    const auto log_path =
        softadastra::NativeDataDirectory::path() /
        "logs" /
        "stable-id.log";

    std::filesystem::create_directories(log_path.parent_path());

    {
      std::ofstream output(
          log_path,
          std::ios::trunc | std::ios::binary);
      output << "first\n";
    }

    const auto initial = get(
        ui.port(),
        "/api/software/cloud/logs");

    EXPECT_NE(
        initial.find("\"logs\":\"first\\n\""),
        std::string::npos);
    EXPECT_NE(
        initial.find("\"offset\":6"),
        std::string::npos);

    {
      std::ofstream output(
          log_path,
          std::ios::app | std::ios::binary);
      output << "second\n";
    }

    const auto appended = get(
        ui.port(),
        "/api/software/cloud/logs?offset=6");

    EXPECT_NE(
        appended.find("\"logs\":\"second\\n\""),
        std::string::npos);
    EXPECT_NE(
        appended.find("\"offset\":13"),
        std::string::npos);

    {
      std::ofstream output(
          log_path,
          std::ios::trunc | std::ios::binary);
      output << "new\n";
    }

    const auto truncated = get(
        ui.port(),
        "/api/software/cloud/logs?offset=13");

    EXPECT_NE(
        truncated.find("\"logs\":\"new\\n\""),
        std::string::npos);
    EXPECT_NE(
        truncated.find("\"reset\":true"),
        std::string::npos);

    const auto clear = post(
        ui.port(),
        "/api/software/cloud/logs/clear");

    EXPECT_NE(clear.find("200 OK"), std::string::npos);

    ui.stop();

    std::filesystem::remove(log_path);
  }

  TEST(WebUiServerTest, ReportsUnknownSoftware)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(
        host,
        platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::WebUiServer ui(client);

    ASSERT_TRUE(ui.start());

    const auto response = get(
        ui.port(),
        "/api/software/missing");

    EXPECT_NE(
        response.find("404 Not Found"),
        std::string::npos);

    ui.stop();
  }

  TEST(WebUiServerTest, StopsAndRestartsCleanly)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(
        host,
        platform.process_launcher());
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
