/**
 *
 *  @file HostIdentity.cpp
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

#include "host/HostIdentity.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <utility>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509.h>

namespace softadastra
{
  namespace
  {
    FILE *open_binary_file(
        const std::filesystem::path &path,
        const char *mode)
    {
#if defined(_WIN32)

      static_cast<void>(mode);

      FILE *file = nullptr;

      return _wfopen_s(
                 &file,
                 path.c_str(),
                 L"wb") == 0
                 ? file
                 : nullptr;

#else

      return std::fopen(
          path.c_str(),
          mode);

#endif
    }

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

    std::string random_secret()
    {
      std::array<unsigned char, 32> bytes{};

      if (RAND_bytes(
              bytes.data(),
              static_cast<int>(bytes.size())) != 1)
      {
        return {};
      }

      return hexadecimal(
          bytes.data(),
          bytes.size());
    }

    unsigned char hex_value(char value)
    {
      return static_cast<unsigned char>(
          value >= 'a'
              ? value - 'a' + 10
              : value - '0');
    }

    bool valid_hexadecimal_key(
        const std::string &value)
    {
      if (value.size() != 64)
      {
        return false;
      }

      for (const char character : value)
      {
        if (!((character >= '0' && character <= '9') ||
              (character >= 'a' && character <= 'f')))
        {
          return false;
        }
      }

      return true;
    }

    bool valid_identity_material(
        const std::string &id,
        const std::string &public_key,
        const std::string &private_key)
    {
      if (id.size() != 64 ||
          !valid_hexadecimal_key(public_key) ||
          !valid_hexadecimal_key(private_key))
      {
        return false;
      }

      std::array<unsigned char, 32> private_key_bytes{};
      std::array<unsigned char, 32> public_key_bytes{};
      std::array<unsigned char, 32> derived_public_key{};
      std::array<unsigned char, 32> fingerprint{};

      for (std::size_t index = 0;
           index < private_key_bytes.size();
           ++index)
      {
        private_key_bytes[index] =
            static_cast<unsigned char>(
                (hex_value(private_key[index * 2]) << 4) |
                hex_value(private_key[index * 2 + 1]));

        public_key_bytes[index] =
            static_cast<unsigned char>(
                (hex_value(public_key[index * 2]) << 4) |
                hex_value(public_key[index * 2 + 1]));
      }

      EVP_PKEY *key =
          EVP_PKEY_new_raw_private_key(
              EVP_PKEY_ED25519,
              nullptr,
              private_key_bytes.data(),
              private_key_bytes.size());

      std::size_t derived_public_key_size =
          derived_public_key.size();

      std::size_t fingerprint_size =
          fingerprint.size();

      const bool valid =
          key != nullptr &&
          EVP_PKEY_get_raw_public_key(
              key,
              derived_public_key.data(),
              &derived_public_key_size) == 1 &&
          derived_public_key == public_key_bytes &&
          EVP_Q_digest(
              nullptr,
              "SHA256",
              nullptr,
              public_key_bytes.data(),
              public_key_bytes.size(),
              fingerprint.data(),
              &fingerprint_size) == 1 &&
          id == hexadecimal(
                    fingerprint.data(),
                    fingerprint_size);

      EVP_PKEY_free(key);

      return valid;
    }

  } // namespace

  HostIdentity::HostIdentity(
      std::filesystem::path path) noexcept
      : path_(std::move(path))
  {
  }

  bool HostIdentity::load_or_create()
  {
    std::ifstream input(path_);
    std::string version;

    if (input)
    {
      std::getline(input, version);
      std::getline(input, id_);
      std::getline(input, secret_);
      std::getline(input, public_key_);
      std::getline(input, private_key_);

      if (version == "ed25519-v1" &&
          id_.size() == 64 &&
          secret_.size() == 64 &&
          public_key_.size() == 64 &&
          private_key_.size() == 64 &&
          valid_identity_material(
              id_,
              public_key_,
              private_key_))
      {
        return true;
      }
    }

    std::array<unsigned char, 32> private_key{};
    std::array<unsigned char, 32> public_key{};
    std::array<unsigned char, 32> fingerprint{};

    std::size_t private_key_size =
        private_key.size();

    std::size_t public_key_size =
        public_key.size();

    std::size_t fingerprint_size =
        fingerprint.size();

    EVP_PKEY *key =
        EVP_PKEY_Q_keygen(
            nullptr,
            nullptr,
            "ED25519");

    const bool generated =
        key != nullptr &&
        EVP_PKEY_get_raw_private_key(
            key,
            private_key.data(),
            &private_key_size) == 1 &&
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
            fingerprint.data(),
            &fingerprint_size) == 1;

    EVP_PKEY_free(key);

    if (!generated ||
        private_key_size != private_key.size() ||
        public_key_size != public_key.size() ||
        fingerprint_size != fingerprint.size())
    {
      return false;
    }

    id_ =
        hexadecimal(
            fingerprint.data(),
            fingerprint.size());

    secret_ =
        random_secret();

    public_key_ =
        hexadecimal(
            public_key.data(),
            public_key.size());

    private_key_ =
        hexadecimal(
            private_key.data(),
            private_key.size());

    if (secret_.empty())
    {
      return false;
    }

    std::error_code error;

    std::filesystem::create_directories(
        path_.parent_path(),
        error);

    if (error)
    {
      return false;
    }

    std::ofstream output(
        path_,
        std::ios::trunc);

    if (!output)
    {
      return false;
    }

    output
        << "ed25519-v1\n"
        << id_
        << '\n'
        << secret_
        << '\n'
        << public_key_
        << '\n'
        << private_key_
        << '\n';

    output.close();

    if (!output)
    {
      return false;
    }

    std::filesystem::permissions(
        path_,
        std::filesystem::perms::owner_read |
            std::filesystem::perms::owner_write,
        std::filesystem::perm_options::replace,
        error);

    return !error;
  }

  const std::string &HostIdentity::id() const noexcept
  {
    return id_;
  }

  const std::string &HostIdentity::public_key() const noexcept
  {
    return public_key_;
  }

  bool HostIdentity::write_tls_certificate(
      const std::filesystem::path &certificate_path,
      const std::filesystem::path &private_key_path) const
  {
    if (!valid_hexadecimal_key(private_key_))
    {
      return false;
    }

    std::array<unsigned char, 32> private_key{};

    for (std::size_t index = 0;
         index < private_key.size();
         ++index)
    {
      private_key[index] =
          static_cast<unsigned char>(
              (hex_value(private_key_[index * 2]) << 4) |
              hex_value(private_key_[index * 2 + 1]));
    }

    EVP_PKEY *key =
        EVP_PKEY_new_raw_private_key(
            EVP_PKEY_ED25519,
            nullptr,
            private_key.data(),
            private_key.size());

    X509 *certificate =
        X509_new();

    const bool valid =
        key != nullptr &&
        certificate != nullptr &&
        X509_set_version(certificate, 2) == 1 &&
        ASN1_INTEGER_set(
            X509_get_serialNumber(certificate),
            1) == 1 &&
        X509_gmtime_adj(
            X509_get_notBefore(certificate),
            0) != nullptr &&
        X509_gmtime_adj(
            X509_get_notAfter(certificate),
            315360000L) != nullptr &&
        X509_set_pubkey(
            certificate,
            key) == 1 &&
        X509_set_issuer_name(
            certificate,
            X509_get_subject_name(certificate)) == 1 &&
        X509_sign(
            certificate,
            key,
            nullptr) > 0;

    std::error_code error;

    std::filesystem::create_directories(
        certificate_path.parent_path(),
        error);

    if (!error)
    {
      std::filesystem::create_directories(
          private_key_path.parent_path(),
          error);
    }

    FILE *certificate_file =
        valid && !error
            ? open_binary_file(
                  certificate_path,
                  "wb")
            : nullptr;

    FILE *key_file =
        certificate_file != nullptr
            ? open_binary_file(
                  private_key_path,
                  "wb")
            : nullptr;

    const bool written =
        key_file != nullptr &&
        PEM_write_X509(
            certificate_file,
            certificate) == 1 &&
        PEM_write_PrivateKey(
            key_file,
            key,
            nullptr,
            nullptr,
            0,
            nullptr,
            nullptr) == 1;

    if (certificate_file != nullptr)
    {
      std::fclose(certificate_file);
    }

    if (key_file != nullptr)
    {
      std::fclose(key_file);
    }

    EVP_PKEY_free(key);
    X509_free(certificate);

    return written;
  }

  const std::string &HostIdentity::secret() const noexcept
  {
    return secret_;
  }

} // namespace softadastra
