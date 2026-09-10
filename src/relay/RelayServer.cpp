
/**
 *
 *  @file RelayServer.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Softadastra
 */

#include "RelayServer.hpp"
#include "RelayProtocol.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <limits>
#include <optional>
#include <string_view>

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

namespace
{
  constexpr int listen_backlog = 64;

#if defined(_WIN32)

  using SocketLength = int;

#else

  using SocketLength = socklen_t;

#endif

  [[nodiscard]] bool initialize_sockets() noexcept
  {
#if defined(_WIN32)

    static bool ready = false;
    static std::once_flag once;

    std::call_once(
        once,
        []
        {
          WSADATA data{};

          ready =
              WSAStartup(
                  MAKEWORD(2, 2),
                  &data) == 0;
        });

    return ready;

#else

    return true;

#endif
  }

  int close_socket(
      softadastra::NativeSocket fd)
  {
#if defined(_WIN32)

    return closesocket(fd);

#else

    return ::close(fd);

#endif
  }

  void shutdown_socket(
      softadastra::NativeSocket fd) noexcept
  {
    if (fd == softadastra::InvalidSocket)
    {
      return;
    }

#if defined(_WIN32)

    ::shutdown(
        fd,
        SD_BOTH);

#else

    ::shutdown(
        fd,
        SHUT_RDWR);

#endif
  }

  bool send_all(
      softadastra::NativeSocket fd,
      std::string_view value)
  {
    for (std::size_t sent = 0;
         sent < value.size();)
    {
      const std::size_t remaining =
          value.size() - sent;

#if defined(_WIN32)

      const int length =
          static_cast<int>(
              std::min(
                  remaining,
                  static_cast<std::size_t>(
                      std::numeric_limits<int>::max())));

#else

      const std::size_t length =
          remaining;

#endif

      const auto result =
          ::send(
              fd,
              value.data() + sent,
              length,
              0);

      if (result <= 0)
      {
        return false;
      }

      sent +=
          static_cast<std::size_t>(result);
    }

    return true;
  }

  bool receive_all(
      softadastra::NativeSocket fd,
      char *output,
      std::size_t size)
  {
    for (std::size_t received = 0;
         received < size;)
    {
      const std::size_t remaining =
          size - received;

#if defined(_WIN32)

      const int length =
          static_cast<int>(
              std::min(
                  remaining,
                  static_cast<std::size_t>(
                      std::numeric_limits<int>::max())));

#else

      const std::size_t length =
          remaining;

#endif

      const auto result =
          ::recv(
              fd,
              output + received,
              length,
              0);

      if (result <= 0)
      {
        return false;
      }

      received +=
          static_cast<std::size_t>(result);
    }

    return true;
  }

  bool read_frame(
      softadastra::NativeSocket fd,
      std::string &frame)
  {
    std::array<char, softadastra::RelayProtocol::frame_header_size> header{};

    if (!receive_all(
            fd,
            header.data(),
            header.size()))
    {
      return false;
    }

    const auto size =
        softadastra::RelayProtocol::decode_frame_header(
            std::string_view(
                header.data(),
                header.size()));

    if (!size)
    {
      return false;
    }

    frame.resize(*size);

    return receive_all(
        fd,
        frame.data(),
        *size);
  }

  bool write_frame(
      softadastra::NativeSocket fd,
      const std::string &frame)
  {
    const auto header =
        softadastra::RelayProtocol::encode_frame_header(
            frame.size());

    if (!header)
    {
      return false;
    }

    return send_all(fd, *header) &&
           send_all(fd, frame);
  }

  [[nodiscard]] std::uint16_t effective_port(
      softadastra::NativeSocket fd) noexcept
  {
    sockaddr_storage address{};
    SocketLength length =
        static_cast<SocketLength>(sizeof(address));

    if (::getsockname(
            fd,
            reinterpret_cast<sockaddr *>(&address),
            &length) != 0)
    {
      return 0;
    }

    if (address.ss_family == AF_INET)
    {
      const auto *ipv4 =
          reinterpret_cast<const sockaddr_in *>(&address);

      return ntohs(ipv4->sin_port);
    }

    if (address.ss_family == AF_INET6)
    {
      const auto *ipv6 =
          reinterpret_cast<const sockaddr_in6 *>(&address);

      return ntohs(ipv6->sin6_port);
    }

    return 0;
  }

  [[nodiscard]] softadastra::NativeSocket open_listener(
      const softadastra::RelayListenEndpoint &endpoint,
      std::uint16_t &port) noexcept
  {
    port = 0;

    if (!initialize_sockets() ||
        !endpoint.valid())
    {
      return softadastra::InvalidSocket;
    }

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo *found{};

    const auto service =
        std::to_string(endpoint.port);

    if (::getaddrinfo(
            endpoint.address.c_str(),
            service.c_str(),
            &hints,
            &found) != 0)
    {
      return softadastra::InvalidSocket;
    }

    softadastra::NativeSocket listener =
        softadastra::InvalidSocket;

    for (auto *item = found;
         item;
         item = item->ai_next)
    {
      listener =
          ::socket(
              item->ai_family,
              item->ai_socktype,
              item->ai_protocol);

      if (listener == softadastra::InvalidSocket)
      {
        continue;
      }

      int enabled = 1;

      static_cast<void>(
          ::setsockopt(
              listener,
              SOL_SOCKET,
              SO_REUSEADDR,
              reinterpret_cast<const char *>(&enabled),
              sizeof(enabled)));

      if (::bind(
              listener,
              item->ai_addr,
#if defined(_WIN32)
              static_cast<int>(item->ai_addrlen)) == 0 &&
#else
              item->ai_addrlen) == 0 &&
#endif
          ::listen(
              listener,
              listen_backlog) == 0)
      {
        port =
            effective_port(listener);
        break;
      }

      close_socket(listener);

      listener =
          softadastra::InvalidSocket;
    }

    ::freeaddrinfo(found);

    return listener;
  }

  [[nodiscard]] bool receive_some(
      softadastra::NativeSocket fd,
      std::string &buffer)
  {
    std::array<char, 4096> chunk{};

#if defined(_WIN32)

    const int length =
        static_cast<int>(chunk.size());

#else

    const std::size_t length =
        chunk.size();

#endif

    const auto received =
        ::recv(
            fd,
            chunk.data(),
            length,
            0);

    if (received <= 0)
    {
      return false;
    }

    buffer.append(
        chunk.data(),
        static_cast<std::size_t>(received));

    return true;
  }

  [[nodiscard]] std::string_view trim(
      std::string_view value) noexcept
  {
    while (!value.empty() &&
           (value.front() == ' ' ||
            value.front() == '\t'))
    {
      value.remove_prefix(1);
    }

    while (!value.empty() &&
           (value.back() == ' ' ||
            value.back() == '\t'))
    {
      value.remove_suffix(1);
    }

    return value;
  }

  [[nodiscard]] bool equals_ignore_case(
      std::string_view left,
      std::string_view right) noexcept
  {
    if (left.size() != right.size())
    {
      return false;
    }

    for (std::size_t index = 0;
         index < left.size();
         ++index)
    {
      const auto a =
          static_cast<unsigned char>(left[index]);
      const auto b =
          static_cast<unsigned char>(right[index]);

      if (std::tolower(a) !=
          std::tolower(b))
      {
        return false;
      }
    }

    return true;
  }

  [[nodiscard]] std::optional<std::size_t> parse_size(
      std::string_view value) noexcept
  {
    value =
        trim(value);

    if (value.empty())
    {
      return std::nullopt;
    }

    std::size_t output = 0;

    const auto result =
        std::from_chars(
            value.data(),
            value.data() + value.size(),
            output);

    if (result.ec != std::errc{} ||
        result.ptr != value.data() + value.size())
    {
      return std::nullopt;
    }

    return output;
  }

  [[nodiscard]] bool parse_http_header_block(
      std::string_view block,
      softadastra::RelayHttpRequest &request,
      std::size_t &content_length)
  {
    const auto line_end =
        block.find("\r\n");

    if (line_end == std::string_view::npos)
    {
      return false;
    }

    const auto request_line =
        block.substr(0, line_end);

    const auto method_end =
        request_line.find(' ');

    if (method_end == std::string_view::npos)
    {
      return false;
    }

    const auto target_start =
        method_end + 1;

    const auto target_end =
        request_line.find(' ', target_start);

    if (target_end == std::string_view::npos ||
        target_end == target_start)
    {
      return false;
    }

    const auto version =
        request_line.substr(target_end + 1);

    if (version != "HTTP/1.0" &&
        version != "HTTP/1.1")
    {
      return false;
    }

    request.method =
        std::string(request_line.substr(0, method_end));
    request.target =
        std::string(
            request_line.substr(
                target_start,
                target_end - target_start));

    std::size_t position =
        line_end + 2;

    content_length = 0;

    while (position < block.size())
    {
      const auto next =
          block.find(
              "\r\n",
              position);

      if (next == std::string_view::npos)
      {
        return false;
      }

      const auto line =
          block.substr(
              position,
              next - position);

      position =
          next + 2;

      if (line.empty())
      {
        continue;
      }

      const auto colon =
          line.find(':');

      if (colon == std::string_view::npos ||
          colon == 0)
      {
        return false;
      }

      const auto key =
          line.substr(0, colon);
      const auto value =
          trim(line.substr(colon + 1));

      if (equals_ignore_case(
              key,
              "Content-Length"))
      {
        const auto parsed =
            parse_size(value);

        if (!parsed)
        {
          return false;
        }

        content_length =
            *parsed;
      }

      request.headers.emplace_back(
          std::string(key),
          std::string(value));
    }

    return true;
  }

  [[nodiscard]] std::optional<softadastra::RelayHttpRequest> read_http_request(
      softadastra::NativeSocket fd,
      std::string software)
  {
    std::string buffer;

    buffer.reserve(4096);

    std::size_t header_end =
        std::string::npos;

    while (header_end == std::string::npos)
    {
      if (buffer.size() >
          softadastra::RelayProtocol::maximum_frame_size)
      {
        return std::nullopt;
      }

      if (!receive_some(fd, buffer))
      {
        return std::nullopt;
      }

      header_end =
          buffer.find("\r\n\r\n");
    }

    softadastra::RelayHttpRequest request;

    request.software =
        std::move(software);

    std::size_t content_length = 0;

    if (!parse_http_header_block(
            std::string_view(
                buffer.data(),
                header_end + 2),
            request,
            content_length) ||
        content_length >
            softadastra::RelayProtocol::maximum_frame_size)
    {
      return std::nullopt;
    }

    const auto body_start =
        header_end + 4;

    if (body_start >
            softadastra::RelayProtocol::maximum_frame_size ||
        content_length >
            softadastra::RelayProtocol::maximum_frame_size - body_start)
    {
      return std::nullopt;
    }

    const auto expected_size =
        body_start + content_length;

    while (buffer.size() < expected_size)
    {
      if (!receive_some(fd, buffer) ||
          buffer.size() >
              softadastra::RelayProtocol::maximum_frame_size)
      {
        return std::nullopt;
      }
    }

    request.body.assign(
        buffer.data() + body_start,
        content_length);

    return request;
  }

  [[nodiscard]] std::string http_response(
      int status,
      std::string_view reason,
      std::string_view body)
  {
    std::string response;

    response.reserve(
        body.size() + 128);

    response +=
        "HTTP/1.1 ";
    response +=
        std::to_string(status);
    response +=
        " ";
    response +=
        reason;
    response +=
        "\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: ";
    response +=
        std::to_string(body.size());
    response +=
        "\r\nConnection: close\r\n\r\n";
    response.append(
        body.data(),
        body.size());

    return response;
  }

  void send_http_response(
      softadastra::NativeSocket fd,
      int status,
      std::string_view reason,
      std::string_view body)
  {
    static_cast<void>(
        send_all(
            fd,
            http_response(
                status,
                reason,
                body)));
  }

} // namespace

namespace softadastra
{
  RelayServer::RelayServer(
      RelayServerConfig config) noexcept
      : config_(std::move(config))
  {
  }

  RelayServer::~RelayServer()
  {
    stop();
  }

  bool RelayServer::start()
  {
    if (running_.load() ||
        !config_.valid())
    {
      return false;
    }

    std::uint16_t bound_host_port = 0;
    std::uint16_t bound_http_port = 0;

    const auto host_listener =
        open_listener(
            config_.host,
            bound_host_port);

    if (host_listener == InvalidSocket)
    {
      return false;
    }

    const auto http_listener =
        open_listener(
            config_.http,
            bound_http_port);

    if (http_listener == InvalidSocket)
    {
      close_socket(host_listener);

      return false;
    }

    stopping_ = false;
    host_listener_ = host_listener;
    http_listener_ = http_listener;
    host_port_ = bound_host_port;
    http_port_ = bound_http_port;
    requests_ = 0;
    failures_ = 0;
    running_ = true;

    host_thread_ =
        std::thread(
            &RelayServer::accept_hosts,
            this);

    http_thread_ =
        std::thread(
            &RelayServer::accept_http_clients,
            this);

    return true;
  }

  void RelayServer::stop() noexcept
  {
    stopping_ = true;

    const auto host_listener =
        host_listener_.exchange(InvalidSocket);

    if (host_listener != InvalidSocket)
    {
      shutdown_socket(host_listener);
      close_socket(host_listener);
    }

    const auto http_listener =
        http_listener_.exchange(InvalidSocket);

    if (http_listener != InvalidSocket)
    {
      shutdown_socket(http_listener);
      close_socket(http_listener);
    }

    const auto host =
        active_host_.exchange(InvalidSocket);

    if (host != InvalidSocket)
    {
      shutdown_socket(host);
      close_socket(host);
    }

    host_connected_ = false;

    close_clients();

    if (host_thread_.joinable())
    {
      host_thread_.join();
    }

    if (http_thread_.joinable())
    {
      http_thread_.join();
    }

    running_ = false;
    host_port_ = 0;
    http_port_ = 0;
  }

  RelayServerStatus RelayServer::status() const noexcept
  {
    RelayServerStatus result;

    result.running =
        running_.load();
    result.host_connected =
        host_connected_.load();
    result.host_port =
        host_port_.load();
    result.http_port =
        http_port_.load();
    result.requests =
        requests_.load();
    result.failures =
        failures_.load();

    return result;
  }

  void RelayServer::accept_hosts() noexcept
  {
    while (!stopping_)
    {
      const auto listener =
          host_listener_.load();

      if (listener == InvalidSocket)
      {
        break;
      }

      const auto fd =
          ::accept(
              listener,
              nullptr,
              nullptr);

      if (fd == InvalidSocket)
      {
        if (!stopping_)
        {
          continue;
        }

        break;
      }

      replace_host(fd);
    }
  }

  void RelayServer::accept_http_clients() noexcept
  {
    while (!stopping_)
    {
      const auto listener =
          http_listener_.load();

      if (listener == InvalidSocket)
      {
        break;
      }

      const auto fd =
          ::accept(
              listener,
              nullptr,
              nullptr);

      if (fd == InvalidSocket)
      {
        if (!stopping_)
        {
          continue;
        }

        break;
      }

      serve_http_client(fd);
    }
  }

  void RelayServer::serve_http_client(
      NativeSocket fd) noexcept
  {
    register_client(fd);

    requests_.fetch_add(1);

    const auto request =
        read_http_request(
            fd,
            config_.default_software);

    if (!request)
    {
      failures_.fetch_add(1);

      send_http_response(
          fd,
          400,
          "Bad Request",
          "invalid HTTP request\n");

      if (unregister_client(fd))
      {
        close_socket(fd);
      }

      return;
    }

    const auto payload =
        RelayProtocol::encode_request(*request);

    if (!payload)
    {
      failures_.fetch_add(1);

      send_http_response(
          fd,
          400,
          "Bad Request",
          "request is too large for the relay protocol\n");

      if (unregister_client(fd))
      {
        close_socket(fd);
      }

      return;
    }

    std::optional<std::string> response;
    NativeSocket failed_host =
        InvalidSocket;
    bool missing_host = false;

    {
      std::lock_guard lock(host_mutex_);

      const auto host =
          active_host_.load();

      if (host == InvalidSocket)
      {
        missing_host = true;
      }
      else
      {
        std::string answer;

        const bool ok =
            write_frame(
                host,
                *payload) &&
            read_frame(
                host,
                answer);

        if (ok)
        {
          response =
              RelayProtocol::decode_response(answer);
        }

        if (!ok ||
            !response)
        {
          failed_host =
              host;
        }
      }
    }

    if (missing_host)
    {
      failures_.fetch_add(1);

      send_http_response(
          fd,
          503,
          "Service Unavailable",
          "no Softadastra Host is connected to this relay\n");
    }
    else if (failed_host != InvalidSocket)
    {
      failures_.fetch_add(1);

      clear_host(failed_host);

      send_http_response(
          fd,
          502,
          "Bad Gateway",
          "the connected Softadastra Host did not answer correctly\n");
    }
    else
    {
      static_cast<void>(
          send_all(
              fd,
              response.value()));
    }

    if (unregister_client(fd))
    {
      close_socket(fd);
    }
  }

  void RelayServer::replace_host(
      NativeSocket fd) noexcept
  {
    std::lock_guard lock(host_mutex_);

    const auto previous =
        active_host_.exchange(fd);

    if (previous != InvalidSocket &&
        previous != fd)
    {
      shutdown_socket(previous);
      close_socket(previous);
    }

    host_connected_ =
        fd != InvalidSocket;
  }

  void RelayServer::clear_host(
      NativeSocket fd) noexcept
  {
    NativeSocket expected =
        fd;

    if (active_host_.compare_exchange_strong(
            expected,
            InvalidSocket))
    {
      shutdown_socket(fd);
      close_socket(fd);
      host_connected_ = false;
    }
  }

  void RelayServer::register_client(
      NativeSocket fd)
  {
    std::lock_guard lock(clients_mutex_);

    clients_.push_back(fd);
  }

  bool RelayServer::unregister_client(
      NativeSocket fd) noexcept
  {
    std::lock_guard lock(clients_mutex_);

    const auto found =
        std::find(
            clients_.begin(),
            clients_.end(),
            fd);

    if (found == clients_.end())
    {
      return false;
    }

    clients_.erase(found);

    return true;
  }

  void RelayServer::close_clients() noexcept
  {
    std::vector<NativeSocket> clients;

    {
      std::lock_guard lock(clients_mutex_);

      clients.swap(clients_);
    }

    for (const auto fd : clients)
    {
      shutdown_socket(fd);
      close_socket(fd);
    }
  }

} // namespace softadastra
