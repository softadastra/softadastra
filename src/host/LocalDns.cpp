/**
 *
 *  @file LocalDns.cpp
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

#include "host/LocalDns.hpp"
#include "software/LocalName.hpp"

#include <array>
#include <thread>

#if defined(__linux__)

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

namespace
{
  constexpr std::string_view zone =
      "softadastra.home.arpa";

  constexpr std::size_t maximum_packet = 512;

  std::uint16_t read16(
      const std::string &value,
      std::size_t position)
  {
    return (static_cast<unsigned char>(value[position]) << 8) |
           static_cast<unsigned char>(value[position + 1]);
  }

  void write16(
      std::string &value,
      std::uint16_t number)
  {
    value.push_back(
        static_cast<char>(number >> 8));

    value.push_back(
        static_cast<char>(number));
  }

  bool qname(
      const std::string &query,
      std::size_t &position,
      std::string &name)
  {
    while (position < query.size())
    {
      const auto size =
          static_cast<unsigned char>(
              query[position++]);

      if (size == 0)
      {
        return !name.empty();
      }

      if (size > 63 ||
          position + size > query.size())
      {
        return false;
      }

      if (!name.empty())
      {
        name += '.';
      }

      name.append(
          query,
          position,
          size);

      position += size;
    }

    return false;
  }

  std::string response(
      const std::string &query,
      const std::string &address)
  {
    if (query.size() < 12)
    {
      return {};
    }

    const auto id =
        read16(query, 0);

    const auto flags =
        read16(query, 2);

    std::uint16_t code = 0;
    std::uint16_t answers = 0;

    std::size_t position = 12;
    std::string name;

    bool valid =
        read16(query, 4) == 1 &&
        read16(query, 6) == 0 &&
        read16(query, 8) == 0 &&
        read16(query, 10) == 0 &&
        qname(query, position, name) &&
        position + 4 == query.size();

    std::uint16_t type = 0;
    std::uint16_t klass = 0;

    if (valid)
    {
      type =
          read16(query, position);

      klass =
          read16(query, position + 2);

      const auto suffix =
          "." + std::string(zone);

      const bool in_zone =
          name.ends_with(suffix);

      const auto label =
          in_zone
              ? name.substr(
                    0,
                    name.size() - suffix.size())
              : "";

      valid =
          in_zone &&
          softadastra::LocalName::from_software_name(
              label)
              .has_value();

      if (!valid)
      {
        code = 3;
      }
      else if (type == 1 &&
               klass == 1)
      {
        answers = 1;
      }
    }
    else
    {
      code = 1;
    }

    std::string result;

    write16(
        result,
        id);

    write16(
        result,
        static_cast<std::uint16_t>(
            0x8000 |
            0x0400 |
            (flags & 0x0100) |
            code));

    write16(
        result,
        valid ? 1 : 0);

    write16(
        result,
        answers);

    write16(
        result,
        0);

    write16(
        result,
        0);

    if (valid)
    {
      result.append(
          query,
          12,
          position + 4 - 12);

      if (answers)
      {
        write16(
            result,
            0xc00c);

        write16(
            result,
            1);

        write16(
            result,
            1);

        result.append(
            "\0\0\0\36",
            4);

        write16(
            result,
            4);

        in_addr ipv4{};

        if (::inet_pton(
                AF_INET,
                address.c_str(),
                &ipv4) != 1)
        {
          return {};
        }

        result.append(
            reinterpret_cast<const char *>(&ipv4.s_addr),
            4);
      }
    }

    return result;
  }

} // namespace

namespace softadastra
{
  class LocalDns::Thread : public std::thread
  {
  public:
    using std::thread::thread;
  };

  LocalDns::~LocalDns()
  {
    stop();
  }

  bool LocalDns::start(
      std::string address,
      std::uint16_t port)
  {
    if (descriptor_ >= 0)
    {
      return false;
    }

    int descriptor =
        ::socket(
            AF_INET,
            SOCK_DGRAM,
            0);

    sockaddr_in socket_address{};
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);

    if (descriptor < 0 ||
        ::inet_pton(
            AF_INET,
            address.c_str(),
            &socket_address.sin_addr) != 1 ||
        ::bind(
            descriptor,
            reinterpret_cast<sockaddr *>(&socket_address),
            sizeof(socket_address)))
    {
      if (descriptor >= 0)
      {
        ::close(descriptor);
      }

      status_.state =
          LocalDnsState::Failed;

      return false;
    }

    socklen_t address_size =
        sizeof(socket_address);

    if (::getsockname(
            descriptor,
            reinterpret_cast<sockaddr *>(&socket_address),
            &address_size))
    {
      ::close(descriptor);

      status_.state =
          LocalDnsState::Failed;

      return false;
    }

    descriptor_ = descriptor;
    stopping_ = false;

    status_ = {
        LocalDnsState::Running,
        std::move(address),
        ntohs(socket_address.sin_port)};

    thread_ =
        new Thread(
            [this]
            {
              run();
            });

    return true;
  }

  void LocalDns::stop() noexcept
  {
    stopping_ = true;

    if (descriptor_ >= 0)
    {
      ::shutdown(
          descriptor_,
          SHUT_RDWR);

      ::close(descriptor_);

      descriptor_ = -1;
    }

    if (thread_)
    {
      thread_->join();

      delete thread_;
      thread_ = nullptr;
    }

    status_ = {};
  }

  LocalDnsStatus LocalDns::status() const
  {
    return status_;
  }

  void LocalDns::run() noexcept
  {
    std::array<char, maximum_packet> buffer{};

    while (!stopping_)
    {
      sockaddr_in peer{};
      socklen_t peer_size =
          sizeof(peer);

      auto received =
          ::recvfrom(
              descriptor_,
              buffer.data(),
              buffer.size(),
              0,
              reinterpret_cast<sockaddr *>(&peer),
              &peer_size);

      if (received <= 0)
      {
        continue;
      }

      const auto result =
          response(
              std::string(
                  buffer.data(),
                  static_cast<std::size_t>(received)),
              status_.address);

      if (!result.empty())
      {
        static_cast<void>(
            ::sendto(
                descriptor_,
                result.data(),
                result.size(),
                0,
                reinterpret_cast<sockaddr *>(&peer),
                peer_size));
      }
    }
  }

} // namespace softadastra
