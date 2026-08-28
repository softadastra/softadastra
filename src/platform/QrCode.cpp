/**
 *
 *  @file QrCode.cpp
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

#include "platform/QrCode.hpp"
#include "internal/QrEncoder.hpp"

#include <cstdio>
#include <iostream>

#if defined(_WIN32)

#include <io.h>

#else

#include <unistd.h>

#endif

namespace softadastra
{
  namespace
  {
    [[nodiscard]] bool stdout_is_terminal() noexcept
    {
#if defined(_WIN32)
      return ::_isatty(::_fileno(stdout)) != 0;
#else
      return ::isatty(::fileno(stdout)) != 0;
#endif
    }
  } // namespace

  std::string QrCode::render(std::string_view content) noexcept
  {
    try
    {
      return internal::generate(content).to_ascii();
    }
    catch (...)
    {
      return {};
    }
  }

  bool QrCode::print(std::string_view content) noexcept
  {
    try
    {
      const internal::ASCIIOptions options{
          .quiet_zone = 4,
          .use_ansi = stdout_is_terminal()};
      std::cout << internal::generate(content).to_ascii(options);
      return true;
    }
    catch (...)
    {
      return false;
    }
  }

} // namespace softadastra
