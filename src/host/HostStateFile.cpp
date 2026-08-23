/**
 *
 *  @file HostStateFile.cpp
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

#include "host/HostStateFile.hpp"

#include "software/SoftwareRegistrationFormat.hpp"

#include <fstream>
#include <utility>

namespace softadastra
{
  HostStateFile::HostStateFile(std::filesystem::path path) noexcept
      : path_(std::move(path))
  {
  }

  bool HostStateFile::exists() const noexcept
  {
    std::error_code error;
    return std::filesystem::exists(path_, error) && !error;
  }

  bool HostStateFile::save(const HostState &state) const
  {
    std::error_code error;
    std::filesystem::create_directories(path_.parent_path(), error);

    if (error)
    {
      return false;
    }

    const auto temporary = path_.string() + ".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);

    if (!output)
    {
      return false;
    }

    output << SoftwareRegistrationFormat::serialize(state.software());
    output.close();

    if (!output)
    {
      return false;
    }

    std::filesystem::rename(temporary, path_, error);

    if (!error)
    {
      return true;
    }

    std::filesystem::remove(path_, error);
    error.clear();
    std::filesystem::rename(temporary, path_, error);
    return !error;
  }

  bool HostStateFile::load(HostState &state) const
  {
    std::ifstream input(path_, std::ios::binary);

    if (!input)
    {
      last_load_error_ = HostStateLoadError::FileUnavailable;
      return false;
    }

    const std::string content(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    const auto entries = SoftwareRegistrationFormat::deserialize(content);

    if (!entries.has_value())
    {
      last_load_error_ = HostStateLoadError::InvalidContent;
      return false;
    }

    if (!state.empty())
    {
      last_load_error_ = HostStateLoadError::StateNotEmpty;
      return false;
    }

    for (const auto &entry : entries.value())
    {
      if (!state.add_software(entry))
      {
        last_load_error_ = HostStateLoadError::InvalidContent;
        return false;
      }
    }

    last_load_error_ = HostStateLoadError::None;
    return true;
  }

  HostStateLoadError HostStateFile::last_load_error() const noexcept
  {
    return last_load_error_;
  }

} // namespace softadastra
