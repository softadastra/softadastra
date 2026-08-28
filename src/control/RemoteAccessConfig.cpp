/**
 *
 *  @file RemoteAccessConfig.cpp
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

#include "control/RemoteAccessConfig.hpp"

#include <fstream>
#include <utility>

namespace softadastra
{
  RemoteAccessConfig::RemoteAccessConfig(std::filesystem::path path) noexcept
      : path_(std::move(path))
  {
  }

  bool RemoteAccessConfig::load(RemoteAccessSettings &settings) const
  {
    std::ifstream input(path_);

    unsigned int version = 0;
    int enabled = 0;
    unsigned int port = 0;
    std::string address;

    if (!(input >> version >> enabled >> address >> port) ||
        version != 2 ||
        (enabled != 0 && enabled != 1) ||
        port > 65535)
    {
      return false;
    }

    settings = {
        enabled != 0,
        address == "-" ? "" : address,
        static_cast<std::uint16_t>(port)};

    return !settings.enabled ||
           (!settings.address.empty() && settings.port != 0);
  }

  bool RemoteAccessConfig::save(
      const RemoteAccessSettings &settings) const
  {
    if (settings.enabled &&
        (settings.address.empty() || settings.port == 0))
    {
      return false;
    }

    std::error_code error;
    std::filesystem::create_directories(
        path_.parent_path(),
        error);

    if (error)
    {
      return false;
    }

    const auto temporary =
        path_.string() + ".tmp";

    std::ofstream output(
        temporary,
        std::ios::trunc);

    if (!output)
    {
      return false;
    }

    output << "2 "
           << (settings.enabled ? 1 : 0)
           << ' '
           << (settings.address.empty() ? "-" : settings.address)
           << ' '
           << settings.port
           << '\n';

    output.close();

    if (!output)
    {
      return false;
    }

    std::filesystem::rename(
        temporary,
        path_,
        error);

    if (!error)
    {
      return true;
    }

    std::filesystem::remove(
        path_,
        error);

    error.clear();

    std::filesystem::rename(
        temporary,
        path_,
        error);

    return !error;
  }

} // namespace softadastra
