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
#include <openssl/rand.h>
#include <utility>

namespace softadastra
{
  namespace
  {
    std::string random_hex()
    {
      std::array<unsigned char, 32> bytes{};

      if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1)
      {
        return {};
      }

      constexpr char digits[] = "0123456789abcdef";
      std::string value;
      value.reserve(bytes.size() * 2);

      for (const unsigned char byte : bytes)
      {
        value += digits[byte >> 4U];
        value += digits[byte & 0x0fU];
      }

      return value;
    }
  }

  HostIdentity::HostIdentity(std::filesystem::path path) noexcept
      : path_(std::move(path))
  {
  }

  bool HostIdentity::load_or_create()
  {
    std::ifstream input(path_);

    if (input)
    {
      std::getline(input, id_);
      std::getline(input, secret_);
      return !id_.empty() && !secret_.empty();
    }

    id_ = random_hex();
    secret_ = random_hex();

    if (id_.empty() || secret_.empty())
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

    output << id_ << '\n' << secret_ << '\n';
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

  const std::string &HostIdentity::id() const noexcept { return id_; }
  const std::string &HostIdentity::secret() const noexcept { return secret_; }
}
