#include "webui/WebUiServer.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <asio/write.hpp>

#include <istream>
#include <system_error>

#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace
{
  using Tcp = asio::ip::tcp;

  std::string response(int status, const std::string &body, const char *type = "application/json")
  {
    const char *reason = status == 200 ? "OK" : status == 404 ? "Not Found" :
                         status == 503 ? "Service Unavailable" : "Bad Request";
    return "HTTP/1.1 " + std::to_string(status) + " " + reason + "\r\nContent-Type: " +
           type + "\r\nContent-Length: " + std::to_string(body.size()) +
           "\r\nConnection: close\r\n\r\n" + body;
  }

  std::string escape_json(const std::string &value)
  {
    std::string escaped;
    for (char character : value)
    {
      if (character == '"' || character == '\\') escaped += '\\';
      if (character == '\n') escaped += "\\n";
      else if (character != '\r' && static_cast<unsigned char>(character) >= 0x20) escaped += character;
    }
    return escaped;
  }

  const char *state_name(softadastra::SoftwareState state)
  {
    using softadastra::SoftwareState;
    if (state == SoftwareState::Running) return "running";
    if (state == SoftwareState::Starting) return "starting";
    if (state == SoftwareState::Failed) return "failed";
    return "stopped";
  }
}

namespace softadastra
{
  WebUiServer::WebUiServer(ControlClient &client) noexcept : client_(client) {}
  WebUiServer::~WebUiServer() { stop(); }

  void WebUiServer::publish_startup(bool success, std::uint16_t port) noexcept
  {
    std::lock_guard lock(startup_mutex_);
    startup_success_ = success;
    startup_complete_ = true;
    if (success) port_.store(port);
    startup_condition_.notify_one();
  }

  bool WebUiServer::start(std::uint16_t requested_port)
  {
    if (worker_.joinable()) return false;
    stopping_.store(false);
    port_.store(0);
    {
      std::lock_guard lock(startup_mutex_);
      startup_complete_ = false;
      startup_success_ = false;
    }
    try { worker_ = std::thread([this, requested_port] { run(requested_port); }); }
    catch (...) { return false; }

    std::unique_lock lock(startup_mutex_);
    startup_condition_.wait(lock, [this] { return startup_complete_; });
    const bool success = startup_success_;
    lock.unlock();
    if (!success) { worker_.join(); return false; }
    return true;
  }

  void WebUiServer::stop() noexcept
  {
    stopping_.store(true);
    const auto port = port_.load();
    if (worker_.joinable() && port != 0)
    {
      // This temporary connection only wakes accept(); the worker owns all
      // listener objects and will close the accepted socket itself.
#if defined(__linux__)
      const int wake = ::socket(AF_INET, SOCK_STREAM, 0);
      if (wake >= 0)
      {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        static_cast<void>(::connect(wake, reinterpret_cast<const sockaddr *>(&address), sizeof(address)));
        ::close(wake);
      }
#endif
    }
    if (worker_.joinable()) worker_.join();
    port_.store(0);
  }

  void WebUiServer::run(std::uint16_t requested_port) noexcept
  {
    try
    {
      asio::io_context context;
      Tcp::acceptor acceptor(context);
      std::error_code error;
      const auto address = asio::ip::make_address("127.0.0.1", error);
      acceptor.open(Tcp::v4(), error);
      acceptor.set_option(asio::socket_base::reuse_address(true), error);
      acceptor.bind(Tcp::endpoint(address, requested_port), error);
      acceptor.listen(asio::socket_base::max_listen_connections, error);
      if (error) { publish_startup(false, 0); return; }
      publish_startup(true, acceptor.local_endpoint().port());

      while (!stopping_.load())
      {
        Tcp::socket socket(context);
        acceptor.accept(socket, error);
        if (error) continue;
        if (stopping_.load()) break;

        asio::streambuf request;
        asio::read_until(socket, request, "\r\n\r\n", error);
        if (error) continue;
        std::istream input(&request);
        std::string method, path, version;
        input >> method >> path >> version;
        int status = 200;
        std::string body;
        const char *type = "application/json";
        if (method.empty() || path.empty() || version.rfind("HTTP/", 0) != 0)
        { status = 400; body = "{\"error\":\"malformed request\"}"; }
        else if (path == "/") { body = "<!doctype html><title>Softadastra</title><h1>Softadastra</h1>"; type = "text/html; charset=utf-8"; }
        else if (path == "/api/host")
        { status = client_.host_available() ? 200 : 503; body = status == 200 ? "{\"status\":\"running\"}" : "{\"error\":\"host unavailable\"}"; }
        else if (path == "/api/software")
        {
          body = "["; bool first = true;
          for (const auto &entry : client_.software())
          {
            if (!first) body += ",";
            first = false;
            body += "{\"name\":\"" + escape_json(entry.name()) + "\",\"state\":\"" + state_name(entry.state()) + "\",\"access\":\"";
            if (const auto access = entry.access_point())
              body += std::string(AccessPoint::name(access->protocol())) + ":" + std::to_string(access->port());
            body += "\"}";
          }
          body += "]";
        }
        else if (path.rfind("/api/software/", 0) == 0)
        {
          const auto tail = path.substr(14);
          const auto slash = tail.find('/');
          const auto name = tail.substr(0, slash);
          bool found = false;
          for (const auto &entry : client_.software()) if (entry.name() == name) { found = true; break; }
          if (!found) { status = 404; body = "{\"error\":\"application not found\"}"; }
          else if (slash != std::string::npos && tail.substr(slash + 1) == "access")
          {
            for (const auto &entry : client_.software()) if (entry.name() == name)
            {
              const auto access = client_.local_access(entry.id());
              body = access ? "{\"url\":\"" + escape_json(access->url) + "\",\"ipv4\":\"" + escape_json(access->ipv4) + "\"}" : "{}";
              break;
            }
          }
          else body = "{}";
        }
        else { status = 404; body = "{\"error\":\"not found\"}"; }
        static_cast<void>(asio::write(socket, asio::buffer(response(status, body, type)), error));
      }
    }
    catch (...) { publish_startup(false, 0); }
  }
}
