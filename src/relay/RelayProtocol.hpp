/**
 *
 *  @file RelayProtocol.hpp
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

#ifndef SOFTADASTRA_RELAY_RELAY_PROTOCOL_HPP
#define SOFTADASTRA_RELAY_RELAY_PROTOCOL_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace softadastra
{
  /**
   * @brief Describes the relay payload kind carried inside a relay frame.
   *
   * The numeric values are part of the wire contract already spoken by
   * RemoteReachability.
   */
  enum class RelayMessageType : std::uint8_t
  {
    Request = 1,
    Response = 2
  };

  /**
   * @brief Represents an HTTP request sent by the relay to a connected Host.
   */
  struct RelayHttpRequest
  {
    /**
     * @brief Software name to resolve on the Host.
     */
    std::string software;

    /**
     * @brief HTTP request method.
     */
    std::string method;

    /**
     * @brief HTTP request target.
     */
    std::string target;

    /**
     * @brief HTTP request headers.
     */
    std::vector<std::pair<std::string, std::string>> headers;

    /**
     * @brief HTTP request body.
     */
    std::string body;
  };

  /**
   * @brief Defines the byte-level protocol shared by the relay and Host.
   *
   * RelayProtocol only owns framing and payload encoding. Socket ownership,
   * connection retry, authentication, routing, and TLS stay outside this class.
   */
  class RelayProtocol
  {
  public:
    /**
     * @brief Number of bytes used to prefix every relay frame.
     */
    static constexpr std::size_t frame_header_size = 4;

    /**
     * @brief Maximum accepted relay frame payload size.
     */
    static constexpr std::size_t maximum_frame_size = 1024 * 1024;

    /**
     * @brief Maximum number of HTTP headers accepted in a relay request.
     */
    static constexpr std::size_t maximum_headers = 128;

    /**
     * @brief Maximum accepted size for one encoded request field.
     */
    static constexpr std::size_t maximum_field_size = 32768;

    /**
     * @brief Checks whether a relay payload size can be read safely.
     *
     * @param size Payload size decoded from a relay frame header.
     *
     * @return true if the payload size is accepted, otherwise false.
     */
    [[nodiscard]] static bool valid_payload_size(
        std::size_t size) noexcept;

    /**
     * @brief Encodes the four-byte big-endian frame header for a payload.
     *
     * @param payload_size Payload size to encode.
     *
     * @return Encoded frame header, or std::nullopt if the size is invalid.
     */
    [[nodiscard]] static std::optional<std::string> encode_frame_header(
        std::size_t payload_size);

    /**
     * @brief Decodes the four-byte big-endian frame header.
     *
     * @param header Four-byte frame header.
     *
     * @return Payload size, or std::nullopt if the header is invalid.
     */
    [[nodiscard]] static std::optional<std::uint32_t> decode_frame_header(
        std::string_view header);

    /**
     * @brief Encodes a complete relay frame.
     *
     * @param payload Relay payload to frame.
     *
     * @return Frame header followed by payload, or std::nullopt if invalid.
     */
    [[nodiscard]] static std::optional<std::string> encode_frame(
        std::string_view payload);

    /**
     * @brief Encodes an HTTP relay request payload.
     *
     * @param request Request to encode.
     *
     * @return Encoded request payload, or std::nullopt if invalid.
     */
    [[nodiscard]] static std::optional<std::string> encode_request(
        const RelayHttpRequest &request);

    /**
     * @brief Decodes an HTTP relay request payload.
     *
     * @param payload Encoded request payload.
     *
     * @return Decoded request, or std::nullopt if invalid.
     */
    [[nodiscard]] static std::optional<RelayHttpRequest> decode_request(
        std::string_view payload);

    /**
     * @brief Encodes a raw HTTP response payload.
     *
     * @param raw_http_response Raw HTTP response returned by the Host proxy.
     *
     * @return Encoded response payload, or std::nullopt if invalid.
     */
    [[nodiscard]] static std::optional<std::string> encode_response(
        std::string_view raw_http_response);

    /**
     * @brief Decodes a raw HTTP response payload.
     *
     * @param payload Encoded response payload.
     *
     * @return Raw HTTP response, or std::nullopt if invalid.
     */
    [[nodiscard]] static std::optional<std::string> decode_response(
        std::string_view payload);
  };

} // namespace softadastra

#endif // SOFTADASTRA_RELAY_RELAY_PROTOCOL_HPP
