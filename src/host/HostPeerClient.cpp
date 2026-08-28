/**
 *
 *  @file HostPeerClient.cpp
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

#include "host/HostPeerClient.hpp"

#include <array>
#include <csignal>
#include <optional>
#include <string>
#include <utility>

#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#if defined(__linux__)

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

namespace softadastra
{
  namespace
  {
    std::string hexadecimal(
        const unsigned char *bytes,
        std::size_t size)
    {
      constexpr char digits[] =
          "0123456789abcdef";

      std::string value;
      value.reserve(size * 2);

      for (std::size_t index = 0;
           index < size;
           ++index)
      {
        value += digits[bytes[index] >> 4U];
        value += digits[bytes[index] & 0x0fU];
      }

      return value;
    }

    std::optional<std::string> certificate_id(
        X509 *certificate)
    {
      EVP_PKEY *key =
          X509_get_pubkey(certificate);

      std::array<unsigned char, EVP_MAX_MD_SIZE> public_key{};
      std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};

      std::size_t public_key_size =
          public_key.size();

      std::size_t digest_size =
          digest.size();

      const bool extracted =
          key != nullptr &&
          EVP_PKEY_get_raw_public_key(
              key,
              public_key.data(),
              &public_key_size) == 1 &&
          EVP_Q_digest(
              nullptr,
              "SHA256",
              nullptr,
              public_key.data(),
              public_key_size,
              digest.data(),
              &digest_size) == 1;

      EVP_PKEY_free(key);

      if (!extracted)
      {
        return std::nullopt;
      }

      return hexadecimal(
          digest.data(),
          digest_size);
    }

  } // namespace

  HostPeerClient::HostPeerClient(
      HostPeerTrust trust,
      std::uint16_t port) noexcept
      : trust_(std::move(trust)),
        port_(port)
  {
  }

  std::optional<std::string> HostPeerClient::request(
      std::string_view command) const noexcept
  {
#if defined(__linux__)

    std::signal(SIGPIPE, SIG_IGN);

    if (command != "identity" &&
        command != "ping" &&
        command != "infrastructure")
    {
      return std::nullopt;
    }

    SSL_CTX *context =
        SSL_CTX_new(TLS_client_method());

    if (context == nullptr)
    {
      return std::nullopt;
    }

    SSL_CTX_set_min_proto_version(
        context,
        TLS1_3_VERSION);

    SSL_CTX_set_max_proto_version(
        context,
        TLS1_3_VERSION);

    SSL_CTX_set_verify(
        context,
        SSL_VERIFY_NONE,
        nullptr);

    const int descriptor =
        ::socket(
            AF_INET,
            SOCK_STREAM,
            0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);

    if (descriptor < 0 ||
        ::inet_pton(
            AF_INET,
            trust_.address().c_str(),
            &address.sin_addr) != 1 ||
        ::connect(
            descriptor,
            reinterpret_cast<const sockaddr *>(&address),
            sizeof(address)) != 0)
    {
      if (descriptor >= 0)
      {
        ::close(descriptor);
      }

      SSL_CTX_free(context);

      return std::nullopt;
    }

    SSL *ssl =
        SSL_new(context);

    if (ssl == nullptr)
    {
      ::close(descriptor);
      SSL_CTX_free(context);

      return std::nullopt;
    }

    SSL_set_fd(
        ssl,
        descriptor);

    std::optional<std::string> result;

    if (SSL_connect(ssl) == 1)
    {
      X509 *certificate =
          SSL_get1_peer_certificate(ssl);

      const auto presented_id =
          certificate != nullptr
              ? certificate_id(certificate)
              : std::optional<std::string>{};

      X509_free(certificate);

      if (presented_id &&
          trust_.accepts(*presented_id))
      {
        const std::string request(command);
        const std::string wire_request =
            request + "\n";

        if (SSL_write(
                ssl,
                wire_request.data(),
                static_cast<int>(wire_request.size())) ==
            static_cast<int>(wire_request.size()))
        {
          std::array<char, 512> response{};

          const int size =
              SSL_read(
                  ssl,
                  response.data(),
                  static_cast<int>(response.size()));

          if (size > 0)
          {
            result =
                std::string(
                    response.data(),
                    static_cast<std::size_t>(size));

            if (command == "identity" &&
                *result != "identity " + *presented_id + "\n")
            {
              result.reset();
            }
          }
        }
      }
    }

    SSL_free(ssl);
    ::close(descriptor);
    SSL_CTX_free(context);

    return result;

#else

    static_cast<void>(command);

    return std::nullopt;

#endif
  }

} // namespace softadastra
