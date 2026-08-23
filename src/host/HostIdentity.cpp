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
#include <fstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <utility>

namespace softadastra
{
  namespace
  {
    std::string hexadecimal(const unsigned char *bytes, std::size_t size)
    {
      constexpr char digits[] = "0123456789abcdef";
      std::string value;
      value.reserve(size * 2);

      for (std::size_t index = 0; index < size; ++index)
      {
        value += digits[bytes[index] >> 4U];
        value += digits[bytes[index] & 0x0fU];
      }

      return value;
    }

    std::string random_secret()
    {
      std::array<unsigned char, 32> bytes{};

      if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1)
      {
        return {};
      }

      return hexadecimal(bytes.data(), bytes.size());
    }
  } // namespace

  HostIdentity::HostIdentity(std::filesystem::path path) noexcept
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

      if (version == "ed25519-v1" && id_.size() == 64 &&
          secret_.size() == 64 && public_key_.size() == 64 &&
          private_key_.size() == 64)
      {
        return true;
      }
    }

    std::array<unsigned char, 32> private_key{};
    std::array<unsigned char, 32> public_key{};
    std::array<unsigned char, 32> fingerprint{};
    std::size_t private_key_size = private_key.size();
    std::size_t public_key_size = public_key.size();
    std::size_t fingerprint_size = fingerprint.size();
    EVP_PKEY *key = EVP_PKEY_Q_keygen(nullptr, nullptr, "ED25519");

    const bool generated = key != nullptr &&
                           EVP_PKEY_get_raw_private_key(
                               key, private_key.data(), &private_key_size) == 1 &&
                           EVP_PKEY_get_raw_public_key(
                               key, public_key.data(), &public_key_size) == 1 &&
                           EVP_Q_digest(
                               nullptr, "SHA256", nullptr, public_key.data(),
                               public_key_size, fingerprint.data(), &fingerprint_size) == 1;
    EVP_PKEY_free(key);

    if (!generated || private_key_size != private_key.size() ||
        public_key_size != public_key.size() || fingerprint_size != fingerprint.size())
    {
      return false;
    }

    id_ = hexadecimal(fingerprint.data(), fingerprint.size());
    secret_ = random_secret();
    public_key_ = hexadecimal(public_key.data(), public_key.size());
    private_key_ = hexadecimal(private_key.data(), private_key.size());

    if (secret_.empty())
    {
      return false;
    }

    std::error_code error;
    std::filesystem::create_directories(path_.parent_path(), error);

    if (error)
    {
      return false;
    }

    std::ofstream output(path_, std::ios::trunc);

    if (!output)
    {
      return false;
    }

    output << "ed25519-v1\n" << id_ << '\n' << secret_ << '\n'
           << public_key_ << '\n' << private_key_ << '\n';
    output.close();

    if (!output)
    {
      return false;
    }

    std::filesystem::permissions(
        path_,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
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

  const std::string &HostIdentity::secret() const noexcept
  {
    return secret_;
  }

} // namespace softadastra
