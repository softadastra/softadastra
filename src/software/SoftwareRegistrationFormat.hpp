/**
 *
 *  @file SoftwareRegistrationFormat.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_SOFTWARE_REGISTRATION_FORMAT_HPP
#define SOFTADASTRA_SOFTWARE_SOFTWARE_REGISTRATION_FORMAT_HPP

#include "software/SoftwareEntry.hpp"

#include <optional>
#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Encodes registered software metadata in a versioned text format.
   */
  class SoftwareRegistrationFormat
  {
  public:
    /**
     * @brief Serializes registered software metadata deterministically.
     */
    [[nodiscard]] static std::string serialize(
        const std::vector<SoftwareEntry> &entries);

    /**
     * @brief Parses registered software metadata.
     *
     * @return Parsed entries, or std::nullopt when the input is invalid.
     */
    [[nodiscard]] static std::optional<std::vector<SoftwareEntry>> deserialize(
        const std::string &text);
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_REGISTRATION_FORMAT_HPP
