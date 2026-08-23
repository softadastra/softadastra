/**
 *
 *  @file RemoteControlClient.cpp
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

#include "control/RemoteControlClient.hpp"

#include <openssl/ssl.h>

#include <array>
#include <utility>
#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
namespace softadastra
{
  RemoteControlClient::RemoteControlClient(std::string address, std::uint16_t port,
                                           std::string secret) noexcept
      : address_(std::move(address)), port_(port), secret_(std::move(secret))
  {
  }
  std::optional<std::string> RemoteControlClient::request(const std::string &command) const noexcept
  {
#if defined(__linux__)
    SSL_CTX *context = SSL_CTX_new(TLS_client_method());
    if (!context)
      return std::nullopt;
    SSL_CTX_set_min_proto_version(context, TLS1_3_VERSION);
    SSL_CTX_set_max_proto_version(context, TLS1_3_VERSION);
    SSL_CTX_set_verify(context, SSL_VERIFY_NONE, nullptr);
    const int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    if (descriptor < 0 || ::inet_pton(AF_INET, address_.c_str(), &address.sin_addr) != 1 ||
        ::connect(descriptor, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) != 0) {
      if (descriptor >= 0)
        ::close(descriptor);
      SSL_CTX_free(context);
      return std::nullopt;
    }
    SSL *ssl = SSL_new(context);
    SSL_set_fd(ssl, descriptor);
    const std::string request = "auth " + secret_ + "\n" + command;
    std::optional<std::string> result;
    if (SSL_connect(ssl) == 1 && SSL_write(ssl, request.data(), static_cast<int>(request.size())) ==
                                     static_cast<int>(request.size())) {
      std::array<char, 16384> output{};
      const int count = SSL_read(ssl, output.data(), static_cast<int>(output.size()));
      if (count > 0)
        result = std::string(output.data(), static_cast<std::size_t>(count));
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ::close(descriptor);
    SSL_CTX_free(context);
    return result;
#else
    static_cast<void>(command);
    return std::nullopt;
#endif
  }
}  // namespace softadastra
