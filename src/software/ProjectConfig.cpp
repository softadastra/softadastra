#include "software/ProjectConfig.hpp"

#include <fstream>

namespace softadastra
{
  namespace
  {
    std::optional<std::string> string_value(const std::string &line, const std::string &key)
    {
      if (!line.starts_with(key + " = ")) return std::nullopt;
      const auto value = line.substr(key.size() + 3);
      if (value.size() < 2 || value.front() != '"' || value.back() != '"') return std::nullopt;
      std::string decoded;
      bool escaped = false;
      for (std::size_t i = 1; i + 1 < value.size(); ++i)
      {
        if (escaped) { decoded += value[i]; escaped = false; }
        else if (value[i] == '\\') escaped = true;
        else decoded += value[i];
      }
      return escaped ? std::nullopt : std::optional<std::string>(decoded);
    }
    std::string quote(const std::string &value)
    {
      std::string result{"\""};
      for (const char c : value) { if (c == '\\' || c == '"') result += '\\'; result += c; }
      return result + '"';
    }
  }

  std::optional<std::pair<std::filesystem::path, ProjectConfig>> ProjectConfigFile::find(const std::filesystem::path &directory, std::string *error)
  {
    std::error_code ec;
    auto root = std::filesystem::weakly_canonical(directory, ec);
    while (!ec)
    {
      const auto path = root / "softadastra.toml";
      if (std::filesystem::exists(path, ec))
      {
        std::ifstream input(path);
        std::optional<std::string> id, name, command, access;
        std::string line;
        while (std::getline(input, line))
        {
          if (const auto value = string_value(line, "id")) id = value;
          else if (const auto value = string_value(line, "name")) name = value;
          else if (const auto value = string_value(line, "command")) command = value;
          else if (const auto value = string_value(line, "access")) access = value;
          else if (!line.empty() && line[0] != '#') { if (error) *error = "invalid softadastra.toml: " + path.string(); return std::nullopt; }
        }
        if (!id.has_value() || id->empty() || !name.has_value() || name->empty() || !command.has_value()) { if (error) *error = "invalid softadastra.toml: " + path.string(); return std::nullopt; }
        std::optional<AccessPoint> point;
        if (access.has_value())
        {
          const auto colon = access->find(':');
          const auto protocol = colon == std::string::npos ? std::nullopt : AccessPoint::protocol(access->substr(0, colon));
          std::optional<std::uint16_t> port;
          try
          {
            const auto number = colon == std::string::npos ? -1 : std::stoi(access->substr(colon + 1));
            if (number > 0 && number <= 65535) port = static_cast<std::uint16_t>(number);
          }
          catch (...) {}
          if (!protocol.has_value() || !port.has_value() || port.value() == 0) { if (error) *error = "invalid access in " + path.string(); return std::nullopt; }
          point = AccessPoint::create(protocol.value(), port.value());
          if (!point.has_value()) { if (error) *error = "invalid access in " + path.string(); return std::nullopt; }
        }
        return std::pair(root, ProjectConfig{ProjectIdentity(id.value()), name.value(), command.value(), point});
      }
      const auto parent = root.parent_path(); if (parent == root) break; root = parent;
    }
    return std::nullopt;
  }

  bool ProjectConfigFile::create(const std::filesystem::path &root, const ProjectConfig &config)
  {
    const auto path = root / "softadastra.toml";
    std::error_code error;
    if (std::filesystem::exists(path, error) || error) return false;
    std::ofstream output(path);
    output << "id = " << quote(config.id.value()) << "\nname = " << quote(config.name) << "\ncommand = " << quote(config.command) << "\n";
    if (config.access.has_value()) output << "access = " << quote(std::string(AccessPoint::name(config.access->protocol())) + ":" + std::to_string(config.access->port())) << "\n";
    return static_cast<bool>(output);
  }
}
