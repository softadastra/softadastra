/**
 * @file SoftwareId.cpp
 * @brief Host-owned software identifier generation.
 */

#include "software/SoftwareId.hpp"

#include <random>

namespace softadastra
{
  SoftwareId SoftwareId::generate()
  {
    std::random_device random;
    std::string value;

    constexpr char digits[] =
        "0123456789abcdef";

    for (int index = 0; index < 4; ++index)
    {
      const auto number =
          static_cast<unsigned int>(random());

      for (int shift = 28; shift >= 0; shift -= 4)
      {
        value += digits[(number >> shift) & 0xF];
      }
    }

    return SoftwareId(std::move(value));
  }
} // namespace softadastra
