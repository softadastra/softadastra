/**
 *
 *  @file ProjectConfig.cpp
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

#include "software/ProjectConfig.hpp"

#include <charconv>
#include <fstream>

namespace softadastra
{
  namespace
  {
    std::optional<std::string> string_value(
        const std::string &line,
        const std::string &key)
    {
      if (!line.starts_with(key + " = "))
      {
        return std::nullopt;
      }

      const auto text =
          line.substr(key.size() + 3);

      if (text.size() < 2 ||
          text.front() != '"' ||
          text.back() != '"')
      {
        return std::nullopt;
      }

      std::string decoded;
      bool escaped = false;

      for (std::size_t i = 1;
           i + 1 < text.size();
           ++i)
      {
        if (escaped)
        {
          decoded += text[i];
          escaped = false;
        }
        else if (text[i] == '\\')
        {
          escaped = true;
        }
        else
        {
          decoded += text[i];
        }
      }

      return escaped
                 ? std::nullopt
                 : std::optional<std::string>(decoded);
    }

    std::string quote(const std::string &value)
    {
      std::string result{"\""};

      for (const char c : value)
      {
        if (c == '\\' || c == '"')
        {
          result += '\\';
        }

        result += c;
      }

      return result + '"';
    }

    std::optional<std::uint16_t> port_value(
        const std::string &value) noexcept
    {
      if (value.empty())
      {
        return std::nullopt;
      }

      unsigned int number{};
      const auto [end, error] =
          std::from_chars(
              value.data(),
              value.data() + value.size(),
              number);

      if (error != std::errc{} ||
          end != value.data() + value.size() ||
          number == 0 ||
          number > 65535)
      {
        return std::nullopt;
      }

      return static_cast<std::uint16_t>(number);
    }

    std::optional<std::uint16_t> access_port_value(
        const std::string &line)
    {
      if (!line.starts_with("port = "))
      {
        return std::nullopt;
      }

      if (const auto quoted = string_value(line, "port"))
      {
        return port_value(*quoted);
      }

      return port_value(line.substr(7));
    }

  } // namespace

  std::optional<std::pair<std::filesystem::path, ProjectConfig>>
  ProjectConfigFile::find(
      const std::filesystem::path &directory,
      std::string *error)
  {
    std::error_code ec;
    auto root =
        std::filesystem::weakly_canonical(
            directory,
            ec);

    while (!ec)
    {
      const auto path =
          root / "softadastra.toml";

      if (std::filesystem::exists(path, ec))
      {
        std::ifstream input(path);

        std::optional<std::string> name;
        std::optional<std::string> command;
        std::optional<std::string> access;

        std::vector<AccessPoint> access_points;

        bool in_access = false;

        std::optional<std::string> endpoint_protocol;
        std::optional<std::uint16_t> endpoint_port;

        const auto finish_access = [&]() -> bool
        {
          if (!in_access)
          {
            return true;
          }

          const auto protocol =
              endpoint_protocol
                  ? AccessPoint::protocol(*endpoint_protocol)
                  : std::nullopt;

          if (!protocol || !endpoint_port)
          {
            return false;
          }

          const auto point =
              AccessPoint::create(
                  *protocol,
                  *endpoint_port);

          if (!point)
          {
            return false;
          }

          access_points.push_back(*point);

          return true;
        };

        std::string line;

        while (std::getline(input, line))
        {
          if (line == "[[access]]")
          {
            if (access)
            {
              if (error)
              {
                *error =
                    "invalid softadastra.toml: " +
                    path.string();
              }

              return std::nullopt;
            }

            if (!finish_access())
            {
              if (error)
              {
                *error =
                    "invalid access in " +
                    path.string();
              }

              return std::nullopt;
            }

            in_access = true;
            endpoint_protocol.reset();
            endpoint_port.reset();
          }
          else if (in_access &&
                   line.starts_with("protocol = "))
          {
            if (endpoint_protocol)
            {
              if (error)
              {
                *error =
                    "invalid access in " +
                    path.string();
              }

              return std::nullopt;
            }

            endpoint_protocol = string_value(line, "protocol");

            if (!endpoint_protocol)
            {
              if (error)
              {
                *error =
                    "invalid access in " +
                    path.string();
              }

              return std::nullopt;
            }
          }
          else if (in_access &&
                   line.starts_with("port = "))
          {
            if (endpoint_port)
            {
              if (error)
              {
                *error =
                    "invalid access in " +
                    path.string();
              }

              return std::nullopt;
            }

            endpoint_port = access_port_value(line);

            if (!endpoint_port)
            {
              if (error)
              {
                *error =
                    "invalid access in " +
                    path.string();
              }

              return std::nullopt;
            }
          }
          else if (!in_access &&
                   string_value(line, "id"))
          {
            // Accept legacy project files, but keep this user-owned field
            // out of Host identity decisions.
          }
          else if (!in_access &&
                   string_value(line, "name"))
          {
            if (name)
            {
              if (error)
              {
                *error =
                    "invalid softadastra.toml: " +
                    path.string();
              }

              return std::nullopt;
            }

            name = string_value(line, "name");
          }
          else if (!in_access &&
                   string_value(line, "command"))
          {
            if (command)
            {
              if (error)
              {
                *error =
                    "invalid softadastra.toml: " +
                    path.string();
              }

              return std::nullopt;
            }

            command = string_value(line, "command");
          }
          else if (!in_access &&
                   string_value(line, "access"))
          {
            if (access)
            {
              if (error)
              {
                *error =
                    "invalid softadastra.toml: " +
                    path.string();
              }

              return std::nullopt;
            }

            access = string_value(line, "access");
          }
          else if (!line.empty() &&
                   line[0] != '#')
          {
            if (error)
            {
              *error =
                  "invalid softadastra.toml: " +
                  path.string();
            }

            return std::nullopt;
          }
        }

        if (!finish_access())
        {
          if (error)
          {
            *error =
                "invalid access in " +
                path.string();
          }

          return std::nullopt;
        }

        if (!name.has_value() ||
            name->empty() ||
            !command.has_value())
        {
          if (error)
          {
            *error =
                "invalid softadastra.toml: " +
                path.string();
          }

          return std::nullopt;
        }

        std::optional<AccessPoint> point;

        if (access.has_value())
        {
          const auto colon =
              access->find(':');

          const auto protocol =
              colon == std::string::npos
                  ? std::nullopt
                  : AccessPoint::protocol(
                        access->substr(0, colon));

          const auto port =
              colon == std::string::npos
                  ? std::optional<std::uint16_t>{}
                  : port_value(access->substr(colon + 1));

          if (!protocol.has_value() ||
              !port.has_value() ||
              port.value() == 0)
          {
            if (error)
            {
              *error =
                  "invalid access in " +
                  path.string();
            }

            return std::nullopt;
          }

          point =
              AccessPoint::create(
                  protocol.value(),
                  port.value());

          if (!point.has_value())
          {
            if (error)
            {
              *error =
                  "invalid access in " +
                  path.string();
            }

            return std::nullopt;
          }
        }

        if (point)
        {
          access_points.insert(
              access_points.begin(),
              *point);
        }

        const auto primary =
            access_points.empty()
                ? std::optional<AccessPoint>{}
                : std::optional<AccessPoint>(
                      access_points.front());

        return std::pair(
            root,
            ProjectConfig{
                name.value(),
                command.value(),
                primary,
                std::move(access_points)});
      }

      const auto parent =
          root.parent_path();

      if (parent == root)
      {
        break;
      }

      root = parent;
    }

    return std::nullopt;
  }

  bool ProjectConfigFile::create(
      const std::filesystem::path &root,
      const ProjectConfig &config)
  {
    const auto path =
        root / "softadastra.toml";

    std::error_code error;

    if (std::filesystem::exists(path, error) ||
        error)
    {
      return false;
    }

    std::ofstream output(path);

    output
        << "name = "
        << quote(config.name)
        << "\ncommand = "
        << quote(config.command)
        << "\n";

    const auto &points =
        config.access_points;

    if (points.empty() &&
        config.access.has_value())
    {
      output
          << "access = "
          << quote(
                 std::string(
                     AccessPoint::name(
                         config.access->protocol())) +
                 ":" +
                 std::to_string(
                     config.access->port()))
          << "\n";
    }

    for (const auto &point : points)
    {
      output
          << "[[access]]\nprotocol = "
          << quote(
                 std::string(
                     AccessPoint::name(
                         point.protocol())))
          << "\nport = "
          << std::to_string(
                 point.port())
          << "\n";
    }

    return static_cast<bool>(output);
  }

} // namespace softadastra
