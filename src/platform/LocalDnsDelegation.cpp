/**
 *
 *  @file LocalDnsDelegation.cpp
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

#include "platform/LocalDnsDelegation.hpp"
#include "platform/LocalDnsConfiguration.hpp"

namespace softadastra
{
  std::string LocalDnsDelegation::configuration()
  {
    return std::string("server=/") +
           local_dns_zone +
           "/127.0.0.1#" +
           std::to_string(local_dns_port) +
           "\n";
  }

} // namespace softadastra
