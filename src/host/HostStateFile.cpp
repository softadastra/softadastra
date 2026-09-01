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

#include <cerrno>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <utility>

#if defined(__linux__)

#include <fcntl.h>
#include <unistd.h>

#elif defined(_WIN32)

#include <windows.h>

#endif

namespace softadastra
{
  namespace
  {
    bool write_temporary_file(
        const std::filesystem::path &path,
        const std::string &content)
    {
#if defined(__linux__)
      const int descriptor = ::open(
          path.c_str(),
          O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
          0666);

      if (descriptor < 0)
      {
        return false;
      }

      const char *data = content.data();
      std::size_t written = 0;

      while (written < content.size())
      {
        const ssize_t result = ::write(
            descriptor,
            data + written,
            content.size() - written);

        if (result > 0)
        {
          written += static_cast<std::size_t>(result);
          continue;
        }

        if (result < 0 && errno == EINTR)
        {
          continue;
        }

        static_cast<void>(::close(descriptor));
        return false;
      }

      const bool synchronized = ::fsync(descriptor) == 0;
      const bool closed = ::close(descriptor) == 0;
      return synchronized && closed;
#elif defined(_WIN32)
      const HANDLE handle = ::CreateFileW(
          path.c_str(),
          GENERIC_WRITE,
          0,
          nullptr,
          CREATE_ALWAYS,
          FILE_ATTRIBUTE_NORMAL,
          nullptr);

      if (handle == INVALID_HANDLE_VALUE)
      {
        return false;
      }

      const auto size = content.size();
      DWORD written = 0;
      const bool saved =
          size <= static_cast<std::size_t>(std::numeric_limits<DWORD>::max()) &&
          ::WriteFile(
              handle,
              content.data(),
              static_cast<DWORD>(size),
              &written,
              nullptr) != FALSE &&
          written == size;
      const bool synchronized =
          saved && ::FlushFileBuffers(handle) != FALSE;
      const bool closed = ::CloseHandle(handle) != FALSE;
      return synchronized && closed;
#else
      std::ofstream output(path, std::ios::binary | std::ios::trunc);
      output << content;
      output.close();
      return static_cast<bool>(output);
#endif
    }

    bool replace_state_file(
        const std::filesystem::path &temporary,
        const std::filesystem::path &path)
    {
#if defined(__linux__)
      return ::rename(temporary.c_str(), path.c_str()) == 0;
#elif defined(_WIN32)
      return ::MoveFileExW(
                 temporary.c_str(),
                 path.c_str(),
                 MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
#else
      std::error_code error;
      std::filesystem::rename(temporary, path, error);
      return !error;
#endif
    }

    bool synchronize_parent_directory(
        const std::filesystem::path &path)
    {
#if defined(__linux__)
      const int descriptor = ::open(
          path.parent_path().c_str(),
          O_RDONLY | O_DIRECTORY | O_CLOEXEC);

      if (descriptor < 0)
      {
        return false;
      }

      const bool synchronized = ::fsync(descriptor) == 0;
      const bool closed = ::close(descriptor) == 0;
      return synchronized && closed;
#else
      static_cast<void>(path);
      return true;
#endif
    }
  } // namespace

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

    const std::filesystem::path temporary =
        path_.string() + ".tmp";
    const std::string content =
        SoftwareRegistrationFormat::serialize(state.software());

    if (!write_temporary_file(temporary, content))
    {
      std::filesystem::remove(temporary, error);
      return false;
    }

    if (!replace_state_file(temporary, path_))
    {
      std::filesystem::remove(temporary, error);
      return false;
    }

    return synchronize_parent_directory(path_);
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
