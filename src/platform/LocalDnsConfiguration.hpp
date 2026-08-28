/**
 *
 *  @file LocalDnsConfiguration.hpp
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

#ifndef SOFTADASTRA_PLATFORM_LOCAL_DNS_CONFIGURATION_HPP
#define SOFTADASTRA_PLATFORM_LOCAL_DNS_CONFIGURATION_HPP

#include <cstdint>

namespace softadastra
{
  /**
   * @brief UDP port used by the local Softadastra DNS service.
   */
  inline constexpr std::uint16_t local_dns_port = 53535;

  /**
   * @brief DNS zone reserved for local Softadastra host names.
   */
  inline constexpr const char *local_dns_zone =
      "softadastra.home.arpa";

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_LOCAL_DNS_CONFIGURATION_HPP
