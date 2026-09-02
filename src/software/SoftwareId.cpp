/**
 *
 *  @file SoftwareId.cpp
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

#include "software/SoftwareId.hpp"

#include <random>
#include <utility>

namespace softadastra
{
  SoftwareId SoftwareId::generate()
  {
    std::random_device random;

    std::string value;

    constexpr char digits[] =
        "0123456789abcdef";

    for (int index = 0;
         index < 4;
         ++index)
    {
      const auto number =
          static_cast<unsigned int>(
              random());

      for (int shift = 28;
           shift >= 0;
           shift -= 4)
      {
        value +=
            digits[(number >> shift) &
                   0xF];
      }
    }

    return SoftwareId(
        std::move(value));
  }

} // namespace softadastra
