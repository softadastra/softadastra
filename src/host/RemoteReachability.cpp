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
#include "relay/RelayProtocol.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <limits>
#include <mutex>
#include <utility>

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

        ok =
            read_frame(fd, frame);

        if (ok)
        {
          const auto relay_request =
              RelayProtocol::decode_request(frame);

          if (!relay_request)
          {
            ok = false;
            continue;
          }

          HttpProxyRequest request;

          request.method =
              relay_request->method;

          request.target =
              relay_request->target;

          request.headers =
              relay_request->headers;

          request.body =
              relay_request->body;

          const auto answer =
              RelayProtocol::encode_response(
                  proxy.forward(
                      relay_request->software,
                      request));

          ok =
              answer.has_value() &&
              write_frame(
                  fd,
                  answer.value());
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
