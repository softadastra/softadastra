#ifndef SOFTADASTRA_PLATFORM_ENVIRONMENT_HPP
#define SOFTADASTRA_PLATFORM_ENVIRONMENT_HPP

#include <cstdlib>
#include <optional>
#include <string>

namespace softadastra
{
  inline std::optional<std::string> environment_value(const char *name)
  {
#if defined(_WIN32)
    char *raw = nullptr;
    std::size_t size = 0;
    if (_dupenv_s(&raw, &size, name) != 0 || raw == nullptr) return std::nullopt;
    std::string value(raw);
    std::free(raw);
    return value;
#else
    const char *raw = std::getenv(name);
    return raw == nullptr ? std::nullopt : std::optional<std::string>(raw);
#endif
  }
}

#endif
