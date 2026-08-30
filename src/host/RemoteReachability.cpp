/**
 *
 *  @file RemoteReachability.cpp
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

#include "host/RemoteReachability.hpp"

#include "host/HttpProxy.hpp"
#include "host/NativeSocket.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <limits>
#include <mutex>

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

namespace
{
  constexpr std::size_t max_frame = 1024 * 1024;
  constexpr std::size_t max_headers = 128;
  constexpr std::size_t max_field = 32768;

  int close_socket(
      softadastra::NativeSocket fd)
  {
#if defined(_WIN32)

    return closesocket(fd);

#else

    return ::close(fd);

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

  std::uint16_t u16(
      const char *value)
  {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(
             static_cast<unsigned char>(value[0]))
         << 8) |
        static_cast<std::uint16_t>(
            static_cast<unsigned char>(value[1])));
  }

  std::uint32_t u32(
      const char *value)
  {
    return (static_cast<std::uint32_t>(
                static_cast<unsigned char>(value[0]))
            << 24) |
           (static_cast<std::uint32_t>(
                static_cast<unsigned char>(value[1]))
            << 16) |
           (static_cast<std::uint32_t>(
                static_cast<unsigned char>(value[2]))
            << 8) |
           static_cast<unsigned char>(value[3]);
  }

  void put32(
      std::string &value,
      std::uint32_t number)
  {
    value.push_back(
        static_cast<char>(number >> 24));

    value.push_back(
        static_cast<char>(number >> 16));

    value.push_back(
        static_cast<char>(number >> 8));

    value.push_back(
        static_cast<char>(number));
  }

  bool read_frame(
      softadastra::NativeSocket fd,
      std::string &frame)
  {
    std::array<char, 4> header{};

    if (!receive_all(
            fd,
            header.data(),
            header.size()))
    {
      return false;
    }

    const auto size =
        u32(header.data());

    if (size == 0 ||
        size > max_frame)
    {
      return false;
    }

    frame.resize(size);

    return receive_all(
        fd,
        frame.data(),
        size);
  }

  bool write_frame(
      softadastra::NativeSocket fd,
      const std::string &frame)
  {
    if (frame.size() > max_frame)
    {
      return false;
    }

    std::string header;

    put32(
        header,
        static_cast<std::uint32_t>(
            frame.size()));

    return send_all(fd, header) &&
           send_all(fd, frame);
  }

  bool take(
      const std::string &value,
      std::size_t &position,
      std::size_t size,
      std::string &output)
  {
    if (size > max_field ||
        position > value.size() ||
        size > value.size() - position)
    {
      return false;
    }

    output.assign(
        value.data() + position,
        size);

    position += size;

    return true;
  }

  bool parse_request(
      const std::string &frame,
      std::string &software,
      softadastra::HttpProxyRequest &request)
  {
    if (frame.empty() ||
        frame[0] != 1)
    {
      return false;
    }

    std::size_t position = 1;

    if (position + 2 > frame.size())
    {
      return false;
    }

    auto size =
        u16(frame.data() + position);

    position += 2;

    if (!take(
            frame,
            position,
            size,
            software) ||
        position + 2 > frame.size())
    {
      return false;
    }

    size =
        u16(frame.data() + position);

    position += 2;

    if (!take(
            frame,
            position,
            size,
            request.method) ||
        position + 4 > frame.size())
    {
      return false;
    }

    const auto path =
        u32(frame.data() + position);

    position += 4;

    if (!take(
            frame,
            position,
            path,
            request.target) ||
        position + 2 > frame.size())
    {
      return false;
    }

    const auto count =
        u16(frame.data() + position);

    position += 2;

    if (count > max_headers)
    {
      return false;
    }

    for (std::size_t index = 0;
         index < count;
         ++index)
    {
      if (position + 2 > frame.size())
      {
        return false;
      }

      size =
          u16(frame.data() + position);

      position += 2;

      std::string key;
      std::string value;

      if (!take(
              frame,
              position,
              size,
              key) ||
          position + 2 > frame.size())
      {
        return false;
      }

      size =
          u16(frame.data() + position);

      position += 2;

      if (!take(
              frame,
              position,
              size,
              value))
      {
        return false;
      }

      request.headers.emplace_back(
          std::move(key),
          std::move(value));
    }

    if (position + 4 > frame.size())
    {
      return false;
    }

    const auto body =
        u32(frame.data() + position);

    position += 4;

    return take(
               frame,
               position,
               body,
               request.body) &&
           position == frame.size();
  }

  softadastra::NativeSocket connect_to(
      const softadastra::RemoteEndpoint &endpoint,
      std::atomic<softadastra::NativeSocket> &active)
  {
#if defined(_WIN32)

    static std::once_flag winsock_once;
    static bool winsock_ready = false;

    std::call_once(
        winsock_once,
        []
        {
          WSADATA data{};

          winsock_ready =
              WSAStartup(
                  MAKEWORD(2, 2),
                  &data) == 0;
        });

    if (!winsock_ready)
    {
      return softadastra::InvalidSocket;
    }

#endif

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *found{};

    const auto port =
        std::to_string(endpoint.port);

    if (::getaddrinfo(
            endpoint.address.c_str(),
            port.c_str(),
            &hints,
            &found) != 0)
    {
      return softadastra::InvalidSocket;
    }

    softadastra::NativeSocket fd =
        softadastra::InvalidSocket;

    for (auto *item = found;
         item;
         item = item->ai_next)
    {
      fd =
          ::socket(
              item->ai_family,
              item->ai_socktype,
              item->ai_protocol);

      if (fd == softadastra::InvalidSocket)
      {
        continue;
      }

      active = fd;

      if (::connect(
              fd,
              item->ai_addr,
#if defined(_WIN32)
              static_cast<int>(item->ai_addrlen)) == 0)
#else
              item->ai_addrlen) == 0)
#endif
      {
        break;
      }

      if (active.exchange(
              softadastra::InvalidSocket) == fd)
      {
        close_socket(fd);
      }

      fd =
          softadastra::InvalidSocket;
    }

    ::freeaddrinfo(found);

    return fd;
  }

} // namespace

namespace softadastra
{
  RemoteReachability::RemoteReachability(
      LocalGatewayTargetResolver &resolver) noexcept
      : resolver_(resolver)
  {
  }

  RemoteReachability::~RemoteReachability()
  {
    disable();
  }

  RemoteReachabilityState RemoteReachability::state() const noexcept
  {
    std::lock_guard lock(mutex_);

    return state_;
  }

  RemoteEndpoint RemoteReachability::endpoint() const
  {
    std::lock_guard lock(mutex_);

    return endpoint_;
  }

  void RemoteReachability::configure(
      RemoteEndpoint endpoint)
  {
    disable();

    if (!endpoint.valid())
    {
      return;
    }

    {
      std::lock_guard lock(mutex_);

      endpoint_ =
          std::move(endpoint);

      state_ =
          RemoteReachabilityState::Connecting;

      stopping_ = false;
    }

    thread_ =
        std::thread(
            &RemoteReachability::run,
            this);
  }

  void RemoteReachability::disable() noexcept
  {
    stopping_ = true;

    const auto fd =
        socket_.exchange(InvalidSocket);

    if (fd != InvalidSocket)
    {
#if defined(_WIN32)

      ::shutdown(
          fd,
          SD_BOTH);

#else

      ::shutdown(
          fd,
          SHUT_RDWR);

#endif

      close_socket(fd);
    }

    wake_.notify_all();

    if (thread_.joinable())
    {
      thread_.join();
    }

    std::lock_guard lock(mutex_);

    endpoint_ = {};

    state_ =
        RemoteReachabilityState::Disabled;
  }

  void RemoteReachability::run() noexcept
  {
    HttpProxy proxy(resolver_);

    auto delay =
        std::chrono::milliseconds(50);

    while (!stopping_)
    {
      RemoteEndpoint endpoint;

      {
        std::lock_guard lock(mutex_);

        endpoint = endpoint_;

        state_ =
            RemoteReachabilityState::Connecting;
      }

      NativeSocket fd =
          connect_to(
              endpoint,
              socket_);

      if (fd == InvalidSocket ||
          stopping_)
      {
        if (fd != InvalidSocket &&
            socket_.exchange(InvalidSocket) == fd)
        {
          close_socket(fd);
        }

        break;
      }

      delay =
          std::chrono::milliseconds(50);

      {
        std::lock_guard lock(mutex_);

        if (!stopping_)
        {
          state_ =
              RemoteReachabilityState::Ready;
        }
      }

      bool ok = true;

      while (!stopping_ && ok)
      {
        std::string frame;
        std::string software;
        HttpProxyRequest request;

        ok =
            read_frame(fd, frame) &&
            parse_request(
                frame,
                software,
                request);

        if (ok)
        {
          std::string answer(1, 2);

          answer +=
              proxy.forward(
                  software,
                  request);

          ok =
              write_frame(
                  fd,
                  answer);
        }
      }

      if (socket_.exchange(InvalidSocket) == fd)
      {
        close_socket(fd);
      }

      if (!stopping_)
      {
        std::unique_lock lock(mutex_);

        state_ =
            RemoteReachabilityState::Degraded;

        wake_.wait_for(
            lock,
            delay,
            [this]
            {
              return stopping_.load();
            });

        delay =
            std::min(
                delay * 2,
                std::chrono::milliseconds(1000));
      }
    }
  }

} // namespace softadastra
