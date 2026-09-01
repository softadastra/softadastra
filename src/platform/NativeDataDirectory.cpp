/**
 *
 *  @file NativeDataDirectory.cpp
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

#include "platform/NativeDataDirectory.hpp"
#include "platform/Environment.hpp"

namespace softadastra
{
  std::filesystem::path NativeDataDirectory::path()
  {
#if defined(_WIN32)
    const auto app_data = environment_value("LOCALAPPDATA");

    if (app_data)
    {
      return std::filesystem::path(*app_data) / "Softadastra";
    }
#else
    const auto state_home = environment_value("XDG_STATE_HOME");

    if (state_home)
    {
      return std::filesystem::path(*state_home) / "softadastra";
    }
#endif

    const auto home = environment_value("HOME");

    if (!home)
    {
      return std::filesystem::temp_directory_path() / "softadastra";
    }

#if defined(_WIN32)
    return std::filesystem::path(*home) / "AppData" / "Roaming" / "Softadastra";
#else
    return std::filesystem::path(*home) / ".local" / "state" / "softadastra";
#endif
  }

  std::filesystem::path NativeDataDirectory::box_path()
  {
#if defined(__linux__)
    return "/var/lib/softadastra";
#else
    return path();
#endif
  }

  bool NativeDataDirectory::ensure_exists()
  {
    std::error_code error;
    const auto directory = path();

    std::filesystem::create_directories(directory, error);

    return !error && std::filesystem::is_directory(directory, error) &&
           !error;
  }

} // namespace softadastra
