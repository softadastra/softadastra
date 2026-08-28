#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/NativeDataDirectory.hpp"
#include "software/ProjectConfig.hpp"
#include "webui/WebUiServer.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <chrono>
#include <fstream>

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

  std::string long_running_command()
  {
#if defined(_WIN32)
    return "ping -n 30 127.0.0.1 >NUL";
#else
    return "sleep 30";
#endif
  }

  TEST(WebUiServerTest, ServesLoopbackControlBackedHostAndSoftwareRoutes)
  {
    const std::string command = long_running_command();
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    const auto project = std::filesystem::temp_directory_path() /
                         ("softadastra-web-ui-project-" + std::to_string(
                              std::chrono::steady_clock::now().time_since_epoch().count()));
    ASSERT_TRUE(std::filesystem::create_directories(project));
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        project, {softadastra::ProjectIdentity("web-project"), "Pico", command,
                  softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8081), {}}));
    softadastra::WebUiServer ui(client, [project] {
      return softadastra::DirectoryChooserResult{softadastra::DirectoryChooserStatus::Selected, project};
    });
    ASSERT_TRUE(ui.start());
    ASSERT_NE(ui.port(), 0);

    const auto page_response = get(ui.port(), "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(page_response.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(page_response.find("Content-Type: text/html"), std::string::npos);
    EXPECT_NE(page_response.find("<!doctype html>"), std::string::npos);
    EXPECT_NE(page_response.find("<title>Softadastra</title>"), std::string::npos);
    EXPECT_NE(page_response.find("<h2>Host</h2>"), std::string::npos);
    EXPECT_NE(page_response.find("<h2>Applications</h2>"), std::string::npos);
    EXPECT_NE(page_response.find("This computer"), std::string::npos);
    EXPECT_EQ(page_response.find("No applications yet."), std::string::npos);
    EXPECT_LT(page_response.find("<h2>Applications</h2>"), page_response.find("<h2>Host</h2>"));
    EXPECT_NE(page_response.find("/style.css"), std::string::npos);
    EXPECT_NE(page_response.find("/app.js"), std::string::npos);

    const auto script_response = get(ui.port(), "GET /app.js HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(script_response.find("200 OK"), std::string::npos);
    EXPECT_NE(script_response.find("async function api"), std::string::npos);
    EXPECT_NE(script_response.find("Edit configuration"), std::string::npos);
    EXPECT_NE(script_response.find("access_points"), std::string::npos);
    // The empty-state wording is a frontend behaviour, not server-rendered HTML.
    EXPECT_NE(script_response.find("No applications yet."), std::string::npos);
    const auto style_response = get(ui.port(), "GET /style.css HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(style_response.find("Content-Type: text/css"), std::string::npos);

    const auto empty_list_response = get(ui.port(), "GET /api/software HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(empty_list_response.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(empty_list_response.find("\r\n\r\n[]"), std::string::npos);

    ASSERT_TRUE(service.register_software(
        softadastra::SoftwareId("stable-id"), softadastra::ProcessSpec("app"),
        softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080),
        std::nullopt, "phone-test"));

    const auto host_response = get(ui.port(), "GET /api/host HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(host_response.find("200 OK"), std::string::npos);
    EXPECT_NE(host_response.find("\"status\":\"running\""), std::string::npos);

    const auto project_response = get(ui.port(), "POST /api/project-folder HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(project_response.find("\"name\":\"Pico\""), std::string::npos);
    EXPECT_NE(project_response.find("\"command\":\"" + command + "\""), std::string::npos);
    EXPECT_NE(project_response.find("\"access_points\""), std::string::npos);

    const auto list_response = get(ui.port(), "GET /api/software HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(list_response.find("\"name\":\"phone-test\""), std::string::npos);
    EXPECT_NE(list_response.find("\"access_configured\":\"http:8080\""), std::string::npos);

    const std::string update_body = "{\"name\":\"cloud\",\"project_directory\":\".\",\"command\":\"" + command + "\",\"access_points\":[{\"protocol\":\"http\",\"port\":\"8080\"},{\"protocol\":\"ws\",\"port\":\"9090\"}]}";
    const auto update_response = get(
        ui.port(), "PUT /api/software/phone-test HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: " +
                       std::to_string(update_body.size()) + "\r\n\r\n" + update_body);
    EXPECT_NE(update_response.find("200 OK"), std::string::npos);
    const auto updated = client.software(softadastra::SoftwareId("stable-id"));
    ASSERT_TRUE(updated);
    EXPECT_EQ(updated->id().value(), "stable-id");
    EXPECT_EQ(updated->declared_command(), command);
    ASSERT_EQ(updated->access_points().size(), 2U);
    EXPECT_EQ(updated->access_points()[1].protocol(), softadastra::AccessProtocol::Ws);
    const auto updated_list = get(ui.port(), "GET /api/software HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(updated_list.find("\"configured\":\"http:8080\""), std::string::npos);
    EXPECT_NE(updated_list.find("\"configured\":\"ws:9090\""), std::string::npos);

    const std::string add_body = "{\"name\":\"web-added\",\"project_directory\":\".\",\"command\":\"" + command + "\",\"access_points\":[{\"protocol\":\"http\",\"port\":\"8081\"},{\"protocol\":\"ws\",\"port\":\"9091\"}]}";
    const auto add_response = get(
        ui.port(), "POST /api/software HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: " +
                       std::to_string(add_body.size()) + "\r\n\r\n" + add_body);
    EXPECT_NE(add_response.find("200 OK"), std::string::npos);
    const auto added_list_response = get(ui.port(), "GET /api/software HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(added_list_response.find("\"name\":\"web-added\""), std::string::npos);
    EXPECT_NE(added_list_response.find("\"project_directory\":\".\""), std::string::npos);
    EXPECT_NE(added_list_response.find("\"configured\":\"ws:9091\""), std::string::npos);

    const auto start_response = get(ui.port(), "POST /api/software/web-added/start HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(start_response.find("200 OK"), std::string::npos);
    const auto running_list_response = get(ui.port(), "GET /api/software HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(running_list_response.find("\"name\":\"web-added\",\"state\":\"running\""), std::string::npos);
    const auto running_update_response = get(
        ui.port(), "PUT /api/software/web-added HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: " +
                       std::to_string(add_body.size()) + "\r\n\r\n" + add_body);
    EXPECT_NE(running_update_response.find("Stop the application before editing its configuration."), std::string::npos);
    const auto log_path = softadastra::NativeDataDirectory::path() / "logs" / "stable-id.log";
    std::filesystem::create_directories(log_path.parent_path());
    { std::ofstream output(log_path, std::ios::trunc | std::ios::binary); output << "first\n"; }
    const auto initial_logs_response = get(ui.port(), "GET /api/software/cloud/logs HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(initial_logs_response.find("\"logs\":\"first\\n\""), std::string::npos);
    EXPECT_NE(initial_logs_response.find("\"offset\":6"), std::string::npos);
    { std::ofstream output(log_path, std::ios::app | std::ios::binary); output << "second\n"; }
    const auto appended_logs_response = get(ui.port(), "GET /api/software/cloud/logs?offset=6 HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(appended_logs_response.find("\"logs\":\"second\\n\""), std::string::npos);
    EXPECT_NE(appended_logs_response.find("\"offset\":13"), std::string::npos);
    { std::ofstream output(log_path, std::ios::trunc | std::ios::binary); output << "new\n"; }
    const auto truncated_logs_response = get(ui.port(), "GET /api/software/cloud/logs?offset=13 HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(truncated_logs_response.find("\"logs\":\"new\\n\""), std::string::npos);
    EXPECT_NE(truncated_logs_response.find("\"reset\":true"), std::string::npos);
    const auto clear_logs_response = get(ui.port(), "POST /api/software/cloud/logs/clear HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(clear_logs_response.find("200 OK"), std::string::npos);
    const auto stop_response = get(ui.port(), "POST /api/software/web-added/stop HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(stop_response.find("200 OK"), std::string::npos);
    const auto restart_response = get(ui.port(), "POST /api/software/web-added/restart HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(restart_response.find("200 OK"), std::string::npos);
    const auto final_stop_response = get(ui.port(), "POST /api/software/web-added/stop HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(final_stop_response.find("200 OK"), std::string::npos);
    const auto stopped_logs_response = get(ui.port(), "GET /api/software/web-added/logs HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(stopped_logs_response.find("\"running\":false"), std::string::npos);

    const auto access_response = get(ui.port(), "GET /api/software/cloud/access HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(access_response.find("200 OK"), std::string::npos);

    const auto unknown_response = get(ui.port(), "GET /api/software/missing HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(unknown_response.find("404 Not Found"), std::string::npos);

    const auto remove_response = get(ui.port(), "DELETE /api/software/cloud HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(remove_response.find("200 OK"), std::string::npos);
    const auto remove_added_response = get(ui.port(), "DELETE /api/software/web-added HTTP/1.1\r\nHost: localhost\r\n\r\n");
    EXPECT_NE(remove_added_response.find("200 OK"), std::string::npos);
    EXPECT_TRUE(std::filesystem::is_directory("."));
    ui.stop();
    std::filesystem::remove(log_path);
    std::filesystem::remove_all(project);
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
