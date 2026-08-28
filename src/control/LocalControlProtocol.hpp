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
   * @brief Provides encoding and parsing utilities for the local Host control protocol.
   *
   * LocalControlProtocol defines the field-level transformations used to
   * construct and interpret messages exchanged through the local control
   * interface.
   */
  class LocalControlProtocol
  {
  public:
    /**
     * @brief Encodes a string as a whitespace-safe hexadecimal field.
     *
     * @param value String value to encode.
     *
     * @return Hexadecimal representation of the supplied value.
     */
    [[nodiscard]] static std::string encode(std::string_view value);

    /**
     * @brief Decodes a hexadecimal protocol field.
     *
     * @param value Encoded field to decode.
     *
     * @return Decoded string, or std::nullopt if the field is invalid.
     */
    [[nodiscard]] static std::optional<std::string> decode(
        std::string_view value);

    /**
     * @brief Splits a protocol message into whitespace-separated fields.
     *
     * @param message Protocol message to split.
     *
     * @return Fields contained in the message.
     */
    [[nodiscard]] static std::vector<std::string> fields(
        std::string_view message);

    /**
     * @brief Parses an integer protocol field.
     *
     * @param value Field containing the integer representation.
     *
     * @return Parsed integer, or std::nullopt if the field is invalid.
     */
    [[nodiscard]] static std::optional<int> integer(
        std::string_view value);
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_LOCAL_CONTROL_PROTOCOL_HPP
