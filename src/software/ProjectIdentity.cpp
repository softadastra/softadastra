/**
 *
 *  @file ProjectIdentity.cpp
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

#include "software/ProjectIdentity.hpp"

#include <fstream>
#include <random>

namespace softadastra
{
  std::optional<ProjectIdentity> ProjectIdentity::create(
      const std::filesystem::path &root)
  {
    std::error_code error;

    const auto path =
        root / ".softadastra" / "project";

    if (std::filesystem::exists(path, error) || error)
    {
      return std::nullopt;
    }

    std::filesystem::create_directories(
        root / ".softadastra",
        error);

    if (error)
    {
      return std::nullopt;
    }

    const auto identity = generate();

    std::ofstream output(path);
    output << identity.value() << '\n';

    return output
               ? std::optional<ProjectIdentity>(identity)
               : std::nullopt;
  }

  ProjectIdentity ProjectIdentity::generate()
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

    return ProjectIdentity(std::move(value));
  }

  std::optional<std::pair<std::filesystem::path, ProjectIdentity>>
  ProjectIdentity::find(
      const std::filesystem::path &directory)
  {
    std::error_code error;

    // Keep the caller's path spelling (notably Windows short paths) in the
    // returned project root while still making relative paths usable.
    auto current =
        std::filesystem::absolute(
            directory,
            error);

    while (!error)
    {
      std::ifstream input(
          current / ".softadastra" / "project");

      std::string id;

      if (std::getline(input, id) && !id.empty())
      {
        return std::pair(
            current,
            ProjectIdentity(id));
      }

      const auto parent =
          current.parent_path();

      if (parent == current)
      {
        break;
      }

      current = parent;
    }

    return std::nullopt;
  }

} // namespace softadastra
