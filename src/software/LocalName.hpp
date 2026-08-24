/**
 *
 *  @file LocalName.hpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#ifndef SOFTADASTRA_SOFTWARE_LOCAL_NAME_HPP
#define SOFTADASTRA_SOFTWARE_LOCAL_NAME_HPP

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace softadastra
{
  /**
   * @brief A DNS name derived directly from an eligible human Software name.
   *
   * LocalName has no relationship to SoftwareId. It is not persisted: callers
   * derive it from the current Software name whenever it is needed.
   */
  class LocalName
  {
  public:
    /**
     * @brief Creates a LocalName when @p software_name is one DNS label.
     *
     * The accepted form is [a-z0-9]([a-z0-9-]{0,61}[a-z0-9])?. No
     * normalization, transliteration, or alias generation is performed.
     */
    [[nodiscard]] static std::optional<LocalName> from_software_name(
        std::string_view software_name)
    {
      if (!eligible(software_name))
      {
        return std::nullopt;
      }
      return LocalName(std::string(software_name));
    }

    [[nodiscard]] const std::string &label() const noexcept { return label_; }
    [[nodiscard]] const std::string &short_name() const noexcept { return label_; }
    [[nodiscard]] const std::string &canonical_name() const noexcept
    {
      return canonical_name_;
    }

  private:
    static bool eligible(std::string_view value) noexcept
    {
      if (value.empty() || value.size() > 63 || value.front() == '-' ||
          value.back() == '-')
      {
        return false;
      }

      for (const unsigned char character : value)
      {
        if (!((character >= 'a' && character <= 'z') ||
              (character >= '0' && character <= '9') || character == '-'))
        {
          return false;
        }
      }
      return true;
    }

    explicit LocalName(std::string label)
        : label_(std::move(label)),
          canonical_name_(label_ + ".softadastra.home.arpa")
    {
    }

    std::string label_;
    std::string canonical_name_;
  };
}

#endif // SOFTADASTRA_SOFTWARE_LOCAL_NAME_HPP
