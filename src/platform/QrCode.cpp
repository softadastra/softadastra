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

#include <string>

#if defined(__linux__)

#include <sys/wait.h>
#include <unistd.h>

#endif

namespace softadastra
{
  bool QrCode::available(std::string_view executable) noexcept
  {
#if defined(__linux__)
    return ::access(std::string(executable).c_str(), X_OK) == 0;
#else
    static_cast<void>(executable);
    return false;
#endif
  }

  bool QrCode::print(std::string_view content) noexcept
  {
#if defined(__linux__)
    if (!available())
    {
      return false;
    }

    const std::string value(content);
    const pid_t child = ::fork();

    if (child < 0)
    {
      return false;
    }

    if (child == 0)
    {
      ::execl(
          "/usr/bin/qrencode",
          "qrencode",
          "-t",
          "UTF8",
          "-o",
          "-",
          value.c_str(),
          nullptr);
      ::_exit(127);
    }

    int status = 0;
    return ::waitpid(child, &status, 0) == child &&
           WIFEXITED(status) && WEXITSTATUS(status) == 0;
#else
    static_cast<void>(content);
    return false;
#endif
  }

} // namespace softadastra
