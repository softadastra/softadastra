/**
 *
 *  @file RemoteControlServer.cpp
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

#include "control/RemoteControlServer.hpp"

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <mutex>

#include "control/LocalControlServer.hpp"
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
    constexpr std::size_t max_message = 16384;
    std::mutex control_mutex;
    bool write_all(SSL *ssl, const std::string &value)
    {
      return SSL_write(ssl, value.data(), static_cast<int>(value.size())) ==
             static_cast<int>(value.size());
    }
  }  // namespace
  RemoteControlServer::RemoteControlServer(ControlServer &server, RemoteAccessConfig &config,
                                           std::string secret,
                                           std::filesystem::path directory) noexcept
      : server_(server),
        config_(config),
        secret_(std::move(secret)),
        certificate_directory_(std::move(directory))
  {
  }
  RemoteControlServer::~RemoteControlServer()
  {
    stop();
  }
  bool RemoteControlServer::ensure_certificate() const
  {
    const auto certificate = certificate_directory_ / "remote-cert.pem";
    const auto key = certificate_directory_ / "remote-key.pem";
    if (std::filesystem::exists(certificate) && std::filesystem::exists(key))
      return true;
    std::error_code error;
    std::filesystem::create_directories(certificate_directory_, error);
    if (error)
      return false;
    EVP_PKEY *pkey = EVP_PKEY_new();
    RSA *rsa = RSA_new();
    BIGNUM *exponent = BN_new();
    X509 *x509 = X509_new();
    bool ok = pkey && rsa && exponent && x509 && BN_set_word(exponent, RSA_F4) == 1 &&
              RSA_generate_key_ex(rsa, 2048, exponent, nullptr) == 1 &&
              EVP_PKEY_assign_RSA(pkey, rsa) == 1;
    if (ok)
      rsa = nullptr;
    if (ok) {
      ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
      X509_gmtime_adj(X509_get_notBefore(x509), 0);
      X509_gmtime_adj(X509_get_notAfter(x509), 315360000L);
      X509_set_pubkey(x509, pkey);
      X509_NAME *name = X509_get_subject_name(x509);
      ok = X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                      reinterpret_cast<const unsigned char *>("Softadastra Host"),
                                      -1, -1, 0) == 1 &&
           X509_set_issuer_name(x509, name) == 1 && X509_sign(x509, pkey, EVP_sha256()) > 0;
    }
    FILE *key_file = ok ? std::fopen(key.c_str(), "wb") : nullptr;
    if (key_file) {
      ok = PEM_write_PrivateKey(key_file, pkey, nullptr, nullptr, 0, nullptr, nullptr) == 1;
      std::fclose(key_file);
    }
    FILE *certificate_file = ok ? std::fopen(certificate.c_str(), "wb") : nullptr;
    if (certificate_file) {
      ok = PEM_write_X509(certificate_file, x509) == 1;
      std::fclose(certificate_file);
    }
    EVP_PKEY_free(pkey);
    RSA_free(rsa);
    BN_free(exponent);
    X509_free(x509);
    return ok;
  }
  bool RemoteControlServer::apply()
  {
    stop();
    RemoteAccessSettings settings;
    if (!config_.load(settings))
      return false;
    if (!settings.enabled)
      return true;
    if (!ensure_certificate())
      return false;
    running_ = true;
    thread_ = std::thread(&RemoteControlServer::run, this, settings);
    return true;
  }
  void RemoteControlServer::stop() noexcept
  {
    running_ = false;
    if (descriptor_ >= 0) {
      ::shutdown(descriptor_, SHUT_RDWR);
      ::close(descriptor_);
      descriptor_ = -1;
    }
    if (thread_.joinable())
      thread_.join();
    listening_ = false;
  }
  bool RemoteControlServer::listening() const noexcept
  {
    return listening_;
  }
  void RemoteControlServer::run(RemoteAccessSettings settings) noexcept
  {
#if defined(__linux__)
    SSL_CTX *context = SSL_CTX_new(TLS_server_method());
    if (!context) {
      running_ = false;
      return;
    }
    SSL_CTX_set_min_proto_version(context, TLS1_3_VERSION);
    SSL_CTX_set_max_proto_version(context, TLS1_3_VERSION);
    const auto certificate = certificate_directory_ / "remote-cert.pem";
    const auto key = certificate_directory_ / "remote-key.pem";
    if (SSL_CTX_use_certificate_file(context, certificate.c_str(), SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_use_PrivateKey_file(context, key.c_str(), SSL_FILETYPE_PEM) != 1) {
      SSL_CTX_free(context);
      running_ = false;
      return;
    }
    descriptor_ = ::socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    if (descriptor_ < 0 ||
        ::setsockopt(descriptor_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
      if (descriptor_ >= 0)
        ::close(descriptor_);
      SSL_CTX_free(context);
      running_ = false;
      return;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(settings.port);
    if (::inet_pton(AF_INET, settings.address.c_str(), &address.sin_addr) != 1 ||
        ::bind(descriptor_, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) != 0 ||
        ::listen(descriptor_, 16) != 0) {
      ::close(descriptor_);
      descriptor_ = -1;
      SSL_CTX_free(context);
      running_ = false;
      return;
    }
    listening_ = true;
    while (running_) {
      const int client = ::accept(descriptor_, nullptr, nullptr);
      if (client < 0)
        continue;
      SSL *ssl = SSL_new(context);
      SSL_set_fd(ssl, client);
      if (SSL_accept(ssl) == 1) {
        std::array<char, max_message> input{};
        const int count = SSL_read(ssl, input.data(), static_cast<int>(input.size()));
        const std::string request(input.data(), count > 0 ? static_cast<std::size_t>(count) : 0);
        const auto newline = request.find('\n');
        bool authorized =
            newline != std::string::npos && request.substr(0, newline).rfind("auth ", 0) == 0;
        const std::string provided = authorized ? request.substr(5, newline - 5) : "";
        authorized = authorized && provided.size() == secret_.size() &&
                     CRYPTO_memcmp(provided.data(), secret_.data(), secret_.size()) == 0;
        if (authorized) {
          const auto response = LocalControlServer::handle(server_, request.substr(newline + 1));
          std::lock_guard lock(control_mutex);
          write_all(ssl, response);
        }
        else
          write_all(ssl, "denied");
      }
      SSL_shutdown(ssl);
      SSL_free(ssl);
      ::close(client);
    }
    listening_ = false;
    SSL_CTX_free(context);
#else
    static_cast<void>(settings);
#endif
  }
}  // namespace softadastra
