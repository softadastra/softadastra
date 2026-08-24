/**
 *
 *  @file SoftwareRegistrationFormat.cpp
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

#include "software/SoftwareRegistrationFormat.hpp"

#include <limits>

namespace softadastra
{
  namespace
  {
    constexpr const char *header_v1 = "softadastra-registrations 1\n";
    constexpr const char *header_v2 = "softadastra-registrations 2\n";
    constexpr const char *header_v3 = "softadastra-registrations 3\n";
    constexpr const char *header_v4 = "softadastra-registrations 4\n";

    void append_string(std::string &output, const std::string &value)
    {
      output += std::to_string(value.size());
      output += '\n';
      output += value;
      output += '\n';
    }

    std::optional<std::string> read_string(
        const std::string &text,
        std::size_t &offset)
    {
      const auto end = text.find('\n', offset);

      if (end == std::string::npos)
      {
        return std::nullopt;
      }

      std::size_t length = 0;

      try
      {
        const auto value = std::stoull(text.substr(offset, end - offset));

        if (value > std::numeric_limits<std::size_t>::max())
        {
          return std::nullopt;
        }

        length = static_cast<std::size_t>(value);
      }
      catch (...)
      {
        return std::nullopt;
      }

      offset = end + 1;

      if (length > text.size() - offset ||
          offset + length >= text.size() ||
          text[offset + length] != '\n')
      {
        return std::nullopt;
      }

      const std::string value = text.substr(offset, length);
      offset += length + 1;
      return value;
    }

    std::optional<std::size_t> read_count(
        const std::string &text,
        std::size_t &offset)
    {
      const auto end = text.find('\n', offset);

      if (end == std::string::npos)
      {
        return std::nullopt;
      }

      try
      {
        const auto value = std::stoull(text.substr(offset, end - offset));

        if (value > std::numeric_limits<std::size_t>::max())
        {
          return std::nullopt;
        }

        offset = end + 1;
        return static_cast<std::size_t>(value);
      }
      catch (...)
      {
        return std::nullopt;
      }
    }
  } // namespace

  std::string SoftwareRegistrationFormat::serialize(
      const std::vector<SoftwareEntry> &entries)
  {
    std::string output(header_v4);
    output += std::to_string(entries.size());
    output += '\n';

    for (const auto &entry : entries)
    {
      append_string(output, entry.id().value());
      output += entry.project_identity().has_value() ? "1\n" : "0\n";
      if (entry.project_identity().has_value())
        append_string(output, entry.project_identity()->value());
      append_string(output, entry.process_spec().executable());
      output += entry.process_spec().working_directory().has_value() ? "1\n" : "0\n";
      if (entry.process_spec().working_directory().has_value())
        append_string(output, entry.process_spec().working_directory().value());
      const auto access_point = entry.access_point();
      output += access_point.has_value() ? "1\n" : "0\n";
      if (access_point.has_value())
      {
        append_string(output, std::string(AccessPoint::name(access_point->protocol())));
        output += std::to_string(access_point->port());
        output += '\n';
      }
      output += std::to_string(entry.process_spec().arguments().size());
      output += '\n';

      for (const auto &argument : entry.process_spec().arguments())
      {
        append_string(output, argument);
      }
    }

    return output;
  }

  std::optional<std::vector<SoftwareEntry>>
  SoftwareRegistrationFormat::deserialize(const std::string &text)
  {
    const bool version_four = text.starts_with(header_v4);
    const bool version_three = text.starts_with(header_v3);
    const bool version_two = text.starts_with(header_v2);
    if (!version_four && !version_three && !version_two && !text.starts_with(header_v1))
    {
      return std::nullopt;
    }

    std::size_t offset = std::char_traits<char>::length(
        version_four ? header_v4 : (version_three ? header_v3 : (version_two ? header_v2 : header_v1)));
    const auto count = read_count(text, offset);

    if (!count.has_value())
    {
      return std::nullopt;
    }

    if (count.value() > (text.size() - offset) / 8)
    {
      return std::nullopt;
    }

    std::vector<SoftwareEntry> entries;
    entries.reserve(count.value());

    for (std::size_t index = 0; index < count.value(); ++index)
    {
      const auto id = read_string(text, offset);
      std::optional<ProjectIdentity> project_identity;
      if (version_four)
      {
        const auto configured = read_count(text, offset);
        if (!configured.has_value() || configured.value() > 1) return std::nullopt;
        if (configured.value() == 1)
        {
          const auto value = read_string(text, offset);
          if (!value.has_value() || value->empty()) return std::nullopt;
          project_identity.emplace(value.value());
        }
      }
      const auto executable = read_string(text, offset);
      std::optional<std::string> working_directory;
      if (version_three || version_four)
      {
        const auto configured = read_count(text, offset);
        if (!configured.has_value() || configured.value() > 1)
          return std::nullopt;
        if (configured.value() == 1)
        {
          working_directory = read_string(text, offset);
          if (!working_directory.has_value())
            return std::nullopt;
        }
      }
      std::optional<AccessPoint> access_point;

      if (version_two || version_three || version_four)
      {
        const auto configured = read_count(text, offset);
        if (!configured.has_value() || configured.value() > 1)
          return std::nullopt;
        if (configured.value() == 1)
        {
          const auto protocol_name = read_string(text, offset);
          const auto port = read_count(text, offset);
          if (!protocol_name.has_value() || !port.has_value() || port.value() > 65535)
            return std::nullopt;
          const auto protocol = AccessPoint::protocol(protocol_name.value());
          if (!protocol.has_value())
            return std::nullopt;
          access_point = AccessPoint::create(protocol.value(), static_cast<std::uint16_t>(port.value()));
          if (!access_point.has_value())
            return std::nullopt;
        }
      }
      const auto argument_count = read_count(text, offset);

      if (!id.has_value() || !executable.has_value() ||
          !argument_count.has_value() || id->empty() || executable->empty())
      {
        return std::nullopt;
      }

      if (argument_count.value() > (text.size() - offset) / 3)
      {
        return std::nullopt;
      }

      std::vector<std::string> arguments;
      arguments.reserve(argument_count.value());

      for (std::size_t argument = 0;
           argument < argument_count.value();
           ++argument)
      {
        const auto value = read_string(text, offset);

        if (!value.has_value())
        {
          return std::nullopt;
        }

        arguments.push_back(value.value());
      }

      entries.emplace_back(
          SoftwareId(id.value()),
          ProcessSpec(executable.value(), std::move(arguments), working_directory),
          std::move(project_identity),
          access_point);

      for (std::size_t previous = 0; previous + 1 < entries.size(); ++previous)
      {
        if (entries[previous].id() == entries.back().id())
        {
          return std::nullopt;
        }
      }
    }

    if (offset != text.size())
    {
      return std::nullopt;
    }

    return entries;
  }

} // namespace softadastra
