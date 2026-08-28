/**
 *
 *  @file Environment.hpp
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

#ifndef SOFTADASTRA_PLATFORM_ENVIRONMENT_HPP
#define SOFTADASTRA_PLATFORM_ENVIRONMENT_HPP

#include <cstdlib>
#include <optional>
#include <string>

namespace softadastra
{
  /**
   * @brief Returns the value of an environment variable.
   *
   * @param name Name of the environment variable.
   *
   * @return The environment variable value when defined, or std::nullopt
   *         otherwise.
   */
  inline std::optional<std::string> environment_value(
      const char *name)
  {
#if defined(_WIN32)

    char *raw = nullptr;
    std::size_t size = 0;

    if (_dupenv_s(&raw, &size, name) != 0 ||
        raw == nullptr)
    {
      return std::nullopt;
    }

    std::string value(raw);
    std::free(raw);

    return value;

#else

    const char *raw = std::getenv(name);

    return raw == nullptr
               ? std::nullopt
               : std::optional<std::string>(raw);

#endif
  }

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_ENVIRONMENT_HPP
