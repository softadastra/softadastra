/**
 *
 *  @file LocalControlEndpoint.cpp
 *  @brief Windows named-pipe endpoint identity implementation.
 */

#include "control/LocalControlEndpoint.hpp"

#if defined(_WIN32)

#include <array>
#include <cstdint>
#include <cwctype>
#include <system_error>
#include <vector>

#include <windows.h>

namespace softadastra
{
  namespace
  {
    std::wstring normalized_endpoint_path(
        const std::filesystem::path &path)
    {
      std::error_code error;
      const auto absolute = std::filesystem::absolute(path, error);
      const auto resolved = error ? path : absolute;
      auto parent = resolved.parent_path();
      std::vector<std::filesystem::path> missing;

      while (!parent.empty() &&
             !std::filesystem::exists(parent, error))
      {
        missing.push_back(parent.filename());
        parent = parent.parent_path();
        error.clear();
      }

      std::array<wchar_t, 32768> buffer{};
      const DWORD length = ::GetLongPathNameW(
          parent.c_str(),
          buffer.data(),
          static_cast<DWORD>(buffer.size()));

      const std::filesystem::path long_parent =
          length != 0 && length < buffer.size()
              ? std::filesystem::path(
                    std::wstring(buffer.data(), length))
              : parent;

      std::filesystem::path normalized_parent = long_parent;

      for (auto component = missing.rbegin();
           component != missing.rend();
           ++component)
      {
        normalized_parent /= *component;
      }

      std::wstring value =
          (normalized_parent / resolved.filename()).wstring();

      for (wchar_t &character : value)
      {
        character = character == L'/'
                        ? L'\\'
                        : static_cast<wchar_t>(
                              std::towlower(character));
      }

      return value;
    }
  } // namespace

  std::wstring local_control_pipe_name(
      const std::filesystem::path &path)
  {
    std::uint64_t hash = 1469598103934665603ULL;

    for (const wchar_t character : normalized_endpoint_path(path))
    {
      hash ^= static_cast<std::uint16_t>(character);
      hash *= 1099511628211ULL;
    }

    return L"\\\\.\\pipe\\Softadastra-" +
           std::to_wstring(hash);
  }
} // namespace softadastra

#endif
