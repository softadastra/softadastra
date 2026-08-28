/**
 *
 *  @file NativeLocalDnsDelegation.cpp
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

#include "platform/NativeLocalDnsDelegation.hpp"

#include <fstream>

namespace softadastra
{
  LocalDnsDelegationState NativeLocalDnsDelegation::status() const
  {
    std::error_code ec;

    if (!std::filesystem::is_directory(directory_, ec) || ec)
    {
      return LocalDnsDelegationState::Unavailable;
    }

    const auto expected =
        LocalDnsDelegation::configuration();

    bool relevant = false;

    for (const auto &item :
         std::filesystem::directory_iterator(directory_, ec))
    {
      if (ec)
      {
        return LocalDnsDelegationState::Unavailable;
      }

      if (!item.is_regular_file())
      {
        continue;
      }

      std::ifstream input(item.path());

      std::string content(
          (std::istreambuf_iterator<char>(input)),
          {});

      if (content == expected)
      {
        return LocalDnsDelegationState::Available;
      }

      if (content.find("softadastra.home.arpa") !=
          std::string::npos)
      {
        relevant = true;
      }
    }

    return relevant
               ? LocalDnsDelegationState::Misconfigured
               : LocalDnsDelegationState::Unavailable;
  }

} // namespace softadastra
