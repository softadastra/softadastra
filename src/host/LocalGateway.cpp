/**
 *
 *  @file LocalGateway.cpp
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

#include "host/LocalGateway.hpp"
#include "host/HttpProxy.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <sstream>
#include <thread>

#if defined(__linux__)

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

namespace
{
  constexpr std::size_t max_headers = 32768;
  constexpr std::size_t max_body = 1048576;

  bool send_all(
      int fd,
      std::string_view value)
  {
    for (std::size_t sent = 0;
         sent < value.size();)
    {
      auto result =
          ::send(
              fd,
              value.data() + sent,
              value.size() - sent,
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

  void reply(
      int fd,
      int status,
      const char *reason)
  {
    static_cast<void>(
        send_all(
            fd,
            "HTTP/1.1 " +
                std::to_string(status) +
                " " +
                reason +
                "\r\nContent-Length: 0\r\n"
                "Connection: close\r\n\r\n"));
  }

  std::string lower(
      std::string value)
  {
    for (char &character : value)
    {
      character =
          static_cast<char>(
              std::tolower(
                  static_cast<unsigned char>(character)));
    }

    return value;
  }

  bool read_request(
      int fd,
      softadastra::HttpProxyRequest &request,
      std::string &host)
  {
    std::string raw;
    std::array<char, 4096> buffer{};

    while (raw.find("\r\n\r\n") == std::string::npos)
    {
      auto received =
          ::recv(
              fd,
              buffer.data(),
              buffer.size(),
              0);

      if (received <= 0 ||
          raw.size() +
                  static_cast<std::size_t>(received) >
              max_headers)
      {
        return false;
      }

      raw.append(
          buffer.data(),
          static_cast<std::size_t>(received));
    }

    auto end =
        raw.find("\r\n\r\n");

    auto first =
        raw.find("\r\n");

    if (first == std::string::npos)
    {
      return false;
    }

    std::istringstream line(
        raw.substr(0, first));

    std::string version;
    std::string extra;

    if (!(line >> request.method >> request.target >> version) ||
        line >> extra ||
        version != "HTTP/1.1" ||
        request.target.empty() ||
        request.target.front() != '/')
    {
      return false;
    }

    std::optional<std::size_t> length;

    for (std::size_t position = first + 2;
         position < end;)
    {
      auto next =
          raw.find(
              "\r\n",
              position);

      auto colon =
          raw.find(
              ':',
              position);

      if (next == std::string::npos ||
          colon == std::string::npos ||
          colon >= next)
      {
        return false;
      }

      auto key =
          lower(
              raw.substr(
                  position,
                  colon - position));

      auto value =
          raw.substr(
              colon + 1,
              next - colon - 1);

      while (!value.empty() &&
             (value.front() == ' ' ||
              value.front() == '\t'))
      {
        value.erase(0, 1);
      }

      if (key == "transfer-encoding")
      {
        return false;
      }

      if (key == "content-length")
      {
        if (length ||
            value.empty() ||
            !std::all_of(
                value.begin(),
                value.end(),
                [](unsigned char character)
                {
                  return std::isdigit(character);
                }))
        {
          return false;
        }

        try
        {
          length =
              std::stoull(value);
        }
        catch (...)
        {
          return false;
        }

        if (*length > max_body)
        {
          return false;
        }
      }

      if (key == "host")
      {
        if (!host.empty() &&
            host != value)
        {
          return false;
        }

        host = value;
      }

      request.headers.emplace_back(
          std::move(key),
          std::move(value));

      position = next + 2;
    }

    const auto required =
        length.value_or(0);

    while (raw.size() <
           end + 4 + required)
    {
      auto received =
          ::recv(
              fd,
              buffer.data(),
              buffer.size(),
              0);

      if (received <= 0 ||
          raw.size() +
                  static_cast<std::size_t>(received) >
              max_headers + max_body)
      {
        return false;
      }

      raw.append(
          buffer.data(),
          static_cast<std::size_t>(received));
    }

    request.body.assign(
        raw.data() + end + 4,
        required);

    if (host.empty() ||
        host.find('/') != std::string::npos ||
        host.find('@') != std::string::npos)
    {
      return false;
    }

    auto colon =
        host.rfind(':');

    if (colon != std::string::npos)
    {
      if (host.find(':') != colon ||
          colon == 0)
      {
        return false;
      }

      host.resize(colon);
    }

    host = lower(host);

    return !host.empty();
  }

} // namespace

namespace softadastra
{
  class LocalGateway::Thread : public std::thread
  {
  public:
    using std::thread::thread;
  };

  LocalGateway::LocalGateway(
      LocalGatewayTargetResolver &resolver) noexcept
      : resolver_(resolver)
  {
  }

  LocalGateway::~LocalGateway()
  {
    stop();
  }

  bool LocalGateway::start(
      std::string address,
      std::uint16_t port)
  {
    if (listener_ >= 0)
    {
      return false;
    }

    int fd =
        ::socket(
            AF_INET,
            SOCK_STREAM,
            0);

    int reuse = 1;

    if (fd < 0)
    {
      return false;
    }

    ::setsockopt(
        fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &reuse,
        sizeof(reuse));

    sockaddr_in socket_address{};
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);

    if (::inet_pton(
            AF_INET,
            address.c_str(),
            &socket_address.sin_addr) != 1 ||
        ::bind(
            fd,
            reinterpret_cast<sockaddr *>(&socket_address),
            sizeof(socket_address)) ||
        ::listen(
            fd,
            16))
    {
      ::close(fd);

      status_.state =
          LocalGatewayState::Failed;

      return false;
    }

    if (!start_from_socket(fd))
    {
      ::close(fd);

      status_.state =
          LocalGatewayState::Failed;

      return false;
    }

    return true;
  }

  bool LocalGateway::start_from_socket(
      int fd)
  {
    if (listener_ >= 0 ||
        fd < 0)
    {
      return false;
    }

    int type{};
    int accepting{};

    socklen_t size =
        sizeof(type);

    if (::getsockopt(
            fd,
            SOL_SOCKET,
            SO_TYPE,
            &type,
            &size) ||
        type != SOCK_STREAM)
    {
      return false;
    }

    size =
        sizeof(accepting);

    if (::getsockopt(
            fd,
            SOL_SOCKET,
            SO_ACCEPTCONN,
            &accepting,
            &size) ||
        !accepting)
    {
      return false;
    }

    sockaddr_in address{};

    size =
        sizeof(address);

    if (::getsockname(
            fd,
            reinterpret_cast<sockaddr *>(&address),
            &size) ||
        address.sin_family != AF_INET)
    {
      return false;
    }

    char text[INET_ADDRSTRLEN]{};

    if (!::inet_ntop(
            AF_INET,
            &address.sin_addr,
            text,
            sizeof(text)))
    {
      return false;
    }

    listener_ = fd;
    stopping_ = false;

    status_ = {
        LocalGatewayState::Running,
        text,
        ntohs(address.sin_port)};

    thread_ =
        new Thread(
            [this]
            {
              run();
            });

    return true;
  }

  void LocalGateway::stop() noexcept
  {
    stopping_ = true;

    if (listener_ >= 0)
    {
      ::shutdown(
          listener_,
          SHUT_RDWR);

      ::close(listener_);

      listener_ = -1;
    }

    if (thread_)
    {
      thread_->join();

      delete thread_;
      thread_ = nullptr;
    }

    status_ = {};
  }

  LocalGatewayStatus LocalGateway::status() const
  {
    return status_;
  }

  void LocalGateway::run() noexcept
  {
    HttpProxy proxy(resolver_);

    while (!stopping_)
    {
      int client =
          ::accept(
              listener_,
              nullptr,
              nullptr);

      if (client < 0)
      {
        continue;
      }

      HttpProxyRequest request;
      std::string host;

      if (!read_request(
              client,
              request,
              host))
      {
        reply(
            client,
            400,
            "Bad Request");
      }
      else
      {
        static_cast<void>(
            send_all(
                client,
                proxy.forward(
                    host,
                    request)));
      }

      ::close(client);
    }
  }

} // namespace softadastra
