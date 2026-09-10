
/**
 *
 *  @file RelayProtocol.cpp
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

#include "RelayProtocol.hpp"
#include <limits>

namespace
{
  [[nodiscard]] std::uint16_t u16(
      std::string_view value,
      std::size_t position) noexcept
  {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(
             static_cast<unsigned char>(value[position]))
         << 8) |
        static_cast<std::uint16_t>(
            static_cast<unsigned char>(value[position + 1])));
  }

  [[nodiscard]] std::uint32_t u32(
      std::string_view value,
      std::size_t position) noexcept
  {
    return (static_cast<std::uint32_t>(
                static_cast<unsigned char>(value[position]))
            << 24) |
           (static_cast<std::uint32_t>(
                static_cast<unsigned char>(value[position + 1]))
            << 16) |
           (static_cast<std::uint32_t>(
                static_cast<unsigned char>(value[position + 2]))
            << 8) |
           static_cast<unsigned char>(value[position + 3]);
  }

  void put16(
      std::string &value,
      std::uint16_t number)
  {
    value.push_back(
        static_cast<char>(number >> 8));

    value.push_back(
        static_cast<char>(number));
  }

  void put32(
      std::string &value,
      std::uint32_t number)
  {
    value.push_back(
        static_cast<char>(number >> 24));

    value.push_back(
        static_cast<char>(number >> 16));

    value.push_back(
        static_cast<char>(number >> 8));

    value.push_back(
        static_cast<char>(number));
  }

  [[nodiscard]] bool has(
      std::string_view value,
      std::size_t position,
      std::size_t size) noexcept
  {
    return position <= value.size() &&
           size <= value.size() - position;
  }

  [[nodiscard]] bool valid_field(
      std::string_view value) noexcept
  {
    return value.size() <=
           softadastra::RelayProtocol::maximum_field_size;
  }

  [[nodiscard]] bool valid_u16_field(
      std::string_view value) noexcept
  {
    return valid_field(value) &&
           value.size() <=
               std::numeric_limits<std::uint16_t>::max();
  }

  [[nodiscard]] bool valid_u32_field(
      std::string_view value) noexcept
  {
    return valid_field(value) &&
           value.size() <=
               std::numeric_limits<std::uint32_t>::max();
  }

  [[nodiscard]] bool take(
      std::string_view value,
      std::size_t &position,
      std::size_t size,
      std::string &output)
  {
    if (size >
            softadastra::RelayProtocol::maximum_field_size ||
        !has(value, position, size))
    {
      return false;
    }

    output.assign(
        value.data() + position,
        size);

    position += size;

    return true;
  }

  [[nodiscard]] bool append_u16_field(
      std::string &output,
      std::string_view value)
  {
    if (!valid_u16_field(value))
    {
      return false;
    }

    put16(
        output,
        static_cast<std::uint16_t>(
            value.size()));

    output.append(
        value.data(),
        value.size());

    return true;
  }

  [[nodiscard]] bool append_u32_field(
      std::string &output,
      std::string_view value)
  {
    if (!valid_u32_field(value))
    {
      return false;
    }

    put32(
        output,
        static_cast<std::uint32_t>(
            value.size()));

    output.append(
        value.data(),
        value.size());

    return true;
  }

} // namespace

namespace softadastra
{
  bool RelayProtocol::valid_payload_size(
      std::size_t size) noexcept
  {
    return size != 0 &&
           size <= maximum_frame_size;
  }

  std::optional<std::string> RelayProtocol::encode_frame_header(
      std::size_t payload_size)
  {
    if (!valid_payload_size(payload_size) ||
        payload_size >
            std::numeric_limits<std::uint32_t>::max())
    {
      return std::nullopt;
    }

    std::string output;

    put32(
        output,
        static_cast<std::uint32_t>(
            payload_size));

    return output;
  }

  std::optional<std::uint32_t> RelayProtocol::decode_frame_header(
      std::string_view header)
  {
    if (header.size() != frame_header_size)
    {
      return std::nullopt;
    }

    const auto size =
        u32(header, 0);

    if (!valid_payload_size(size))
    {
      return std::nullopt;
    }

    return size;
  }

  std::optional<std::string> RelayProtocol::encode_frame(
      std::string_view payload)
  {
    auto header =
        encode_frame_header(payload.size());

    if (!header)
    {
      return std::nullopt;
    }

    std::string output;

    output.reserve(
        header->size() + payload.size());

    output.append(*header);

    output.append(
        payload.data(),
        payload.size());

    return output;
  }

  std::optional<std::string> RelayProtocol::encode_request(
      const RelayHttpRequest &request)
  {
    if (request.headers.size() > maximum_headers ||
        request.headers.size() >
            std::numeric_limits<std::uint16_t>::max())
    {
      return std::nullopt;
    }

    std::string output;

    output.push_back(
        static_cast<char>(RelayMessageType::Request));

    if (!append_u16_field(output, request.software) ||
        !append_u16_field(output, request.method) ||
        !append_u32_field(output, request.target))
    {
      return std::nullopt;
    }

    put16(
        output,
        static_cast<std::uint16_t>(
            request.headers.size()));

    for (const auto &[key, value] : request.headers)
    {
      if (!append_u16_field(output, key) ||
          !append_u16_field(output, value))
      {
        return std::nullopt;
      }
    }

    if (!append_u32_field(output, request.body) ||
        !valid_payload_size(output.size()))
    {
      return std::nullopt;
    }

    return output;
  }

  std::optional<RelayHttpRequest> RelayProtocol::decode_request(
      std::string_view payload)
  {
    if (!valid_payload_size(payload.size()) ||
        payload.front() !=
            static_cast<char>(RelayMessageType::Request))
    {
      return std::nullopt;
    }

    RelayHttpRequest request;
    std::size_t position = 1;

    if (!has(payload, position, 2))
    {
      return std::nullopt;
    }

    auto size =
        u16(payload, position);

    position += 2;

    if (!take(
            payload,
            position,
            size,
            request.software) ||
        !has(payload, position, 2))
    {
      return std::nullopt;
    }

    size =
        u16(payload, position);

    position += 2;

    if (!take(
            payload,
            position,
            size,
            request.method) ||
        !has(payload, position, 4))
    {
      return std::nullopt;
    }

    const auto target_size =
        u32(payload, position);

    position += 4;

    if (!take(
            payload,
            position,
            target_size,
            request.target) ||
        !has(payload, position, 2))
    {
      return std::nullopt;
    }

    const auto count =
        u16(payload, position);

    position += 2;

    if (count > maximum_headers)
    {
      return std::nullopt;
    }

    request.headers.reserve(count);

    for (std::size_t index = 0;
         index < count;
         ++index)
    {
      if (!has(payload, position, 2))
      {
        return std::nullopt;
      }

      size =
          u16(payload, position);

      position += 2;

      std::string key;
      std::string value;

      if (!take(
              payload,
              position,
              size,
              key) ||
          !has(payload, position, 2))
      {
        return std::nullopt;
      }

      size =
          u16(payload, position);

      position += 2;

      if (!take(
              payload,
              position,
              size,
              value))
      {
        return std::nullopt;
      }

      request.headers.emplace_back(
          std::move(key),
          std::move(value));
    }

    if (!has(payload, position, 4))
    {
      return std::nullopt;
    }

    const auto body_size =
        u32(payload, position);

    position += 4;

    if (!take(
            payload,
            position,
            body_size,
            request.body) ||
        position != payload.size())
    {
      return std::nullopt;
    }

    return request;
  }

  std::optional<std::string> RelayProtocol::encode_response(
      std::string_view raw_http_response)
  {
    std::string output;

    output.reserve(
        raw_http_response.size() + 1);

    output.push_back(
        static_cast<char>(RelayMessageType::Response));

    output.append(
        raw_http_response.data(),
        raw_http_response.size());

    if (!valid_payload_size(output.size()))
    {
      return std::nullopt;
    }

    return output;
  }

  std::optional<std::string> RelayProtocol::decode_response(
      std::string_view payload)
  {
    if (!valid_payload_size(payload.size()) ||
        payload.front() !=
            static_cast<char>(RelayMessageType::Response))
    {
      return std::nullopt;
    }

    return std::string(
        payload.substr(1));
  }

} // namespace softadastra
