/**
 *
 *  @file LocalControlProtocol.hpp
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

#ifndef SOFTADASTRA_CONTROL_LOCAL_CONTROL_PROTOCOL_HPP
#define SOFTADASTRA_CONTROL_LOCAL_CONTROL_PROTOCOL_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace softadastra
{
  /**
   * @brief Encodes fields used by the local Host control protocol.
   */
  class LocalControlProtocol
  {
  public:
    /**
     * @brief Encodes a string as a whitespace-safe hexadecimal field.
     */
    [[nodiscard]] static std::string encode(std::string_view value);

    /**
     * @brief Decodes a hexadecimal protocol field.
     */
    [[nodiscard]] static std::optional<std::string> decode(
        std::string_view value);

    /**
     * @brief Splits a protocol message into whitespace-separated fields.
     */
    [[nodiscard]] static std::vector<std::string> fields(
        std::string_view message);

    /**
     * @brief Parses an integer protocol field.
     */
    [[nodiscard]] static std::optional<int> integer(
        std::string_view value);
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_LOCAL_CONTROL_PROTOCOL_HPP
