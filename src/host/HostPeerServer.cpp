/**
 *
 *  @file HostPeerServer.cpp
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

#include "host/HostPeerServer.hpp"

#include <openssl/ssl.h>

#include <array>
#include <chrono>
#include <csignal>
#include <utility>

#if defined(__linux__)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace softadastra
{
  namespace
  {
    constexpr std::size_t maximum_request_size = 256;

    std::string response_for(
        const HostIdentity &identity,
        const std::string &infrastructure_info,
        const std::string &request)
    {
      if (request == "identity\n")
      {
        return "identity " + identity.id() + "\n";
      }

      if (request == "ping\n")
      {
        return "pong\n";
      }

      if (request == "infrastructure\n")
      {
        return "infrastructure " + infrastructure_info + "\n";
      }

      return "invalid\n";
    }
  } // namespace

  HostPeerServer::HostPeerServer(
      const HostIdentity &identity,
      std::filesystem::path certificate_directory,
      std::string address,
      std::uint16_t port,
      std::string infrastructure_info) noexcept
      : identity_(identity),
        certificate_directory_(std::move(certificate_directory)),
        address_(std::move(address)),
        port_(port),
        infrastructure_info_(std::move(infrastructure_info))
  {
  }

  HostPeerServer::~HostPeerServer()
  {
    stop();
  }

  bool HostPeerServer::start()
  {
#if defined(__linux__)
    std::signal(SIGPIPE, SIG_IGN);

    if (running_)
    {
      return false;
    }

    const auto certificate = certificate_directory_ / "host-peer-cert.pem";
    const auto key = certificate_directory_ / "host-peer-key.pem";

    if (!identity_.write_tls_certificate(certificate, key))
    {
      return false;
    }

    running_ = true;
    thread_ = std::thread(&HostPeerServer::run, this);

    for (int attempt = 0; attempt < 100 && running_ && !listening_; ++attempt)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (!listening_)
    {
      stop();
      return false;
    }

    return true;
#else
    return false;
#endif
  }

  void HostPeerServer::stop() noexcept
  {
    running_ = false;
    const int descriptor = descriptor_.exchange(-1);

#if defined(__linux__)
    if (descriptor >= 0)
    {
      ::shutdown(descriptor, SHUT_RDWR);
      ::close(descriptor);
    }
#else
    static_cast<void>(descriptor);
#endif

    if (thread_.joinable())
    {
      thread_.join();
    }

    listening_ = false;
  }

  bool HostPeerServer::listening() const noexcept
  {
    return listening_;
  }

  std::uint16_t HostPeerServer::port() const noexcept
  {
    return port_;
  }

  void HostPeerServer::run() noexcept
  {
#if defined(__linux__)
    SSL_CTX *context = SSL_CTX_new(TLS_server_method());

    if (context == nullptr)
    {
      running_ = false;
      return;
    }

    SSL_CTX_set_min_proto_version(context, TLS1_3_VERSION);
    SSL_CTX_set_max_proto_version(context, TLS1_3_VERSION);
    const auto certificate = certificate_directory_ / "host-peer-cert.pem";
    const auto key = certificate_directory_ / "host-peer-key.pem";

    if (SSL_CTX_use_certificate_file(context, certificate.c_str(), SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_use_PrivateKey_file(context, key.c_str(), SSL_FILETYPE_PEM) != 1)
    {
      SSL_CTX_free(context);
      running_ = false;
      return;
    }

    const int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;

    if (descriptor < 0 ||
        ::setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
    {
      if (descriptor >= 0)
      {
        ::close(descriptor);
      }

      SSL_CTX_free(context);
      running_ = false;
      return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);

    if (::inet_pton(AF_INET, address_.c_str(), &address.sin_addr) != 1 ||
        ::bind(descriptor, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) != 0 ||
        ::listen(descriptor, 16) != 0)
    {
      ::close(descriptor);
      SSL_CTX_free(context);
      running_ = false;
      return;
    }

    sockaddr_in bound_address{};
    socklen_t bound_address_size = sizeof(bound_address);

    if (::getsockname(
            descriptor,
            reinterpret_cast<sockaddr *>(&bound_address),
            &bound_address_size) != 0)
    {
      ::close(descriptor);
      SSL_CTX_free(context);
      running_ = false;
      return;
    }

    port_ = ntohs(bound_address.sin_port);
    descriptor_ = descriptor;
    listening_ = true;

    while (running_)
    {
      const int client = ::accept(descriptor, nullptr, nullptr);

      if (client < 0)
      {
        continue;
      }

      SSL *ssl = SSL_new(context);

      if (ssl != nullptr)
      {
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) == 1)
        {
          std::array<char, maximum_request_size> request{};
          const int size = SSL_read(ssl, request.data(), static_cast<int>(request.size()));

          if (size > 0)
          {
            const std::string response = response_for(
                identity_, infrastructure_info_,
                std::string(request.data(), static_cast<std::size_t>(size)));
            static_cast<void>(SSL_write(
                ssl, response.data(), static_cast<int>(response.size())));
          }
        }

        SSL_free(ssl);
      }

      ::close(client);
    }

    if (descriptor_.exchange(-1) == descriptor)
    {
      ::close(descriptor);
    }

    listening_ = false;
    SSL_CTX_free(context);
#endif
  }

} // namespace softadastra
