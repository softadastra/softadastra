/**
 *
 *  @file HostPeerIdentity.cpp
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

#include "host/HostPeerIdentity.hpp"

#include <openssl/crypto.h>

namespace softadastra
{
  bool HostPeerIdentity::valid(std::string_view identity) noexcept
  {
    if (identity.size() != 64)
    {
      return false;
    }

    for (const char character : identity)
    {
      if (!((character >= '0' && character <= '9') ||
            (character >= 'a' && character <= 'f')))
      {
        return false;
      }
    }

    return true;
  }

  bool HostPeerIdentity::matches(
      std::string_view expected,
      std::string_view presented) noexcept
  {
    return valid(expected) && valid(presented) &&
           CRYPTO_memcmp(expected.data(), presented.data(), expected.size()) == 0;
  }

} // namespace softadastra
