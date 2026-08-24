#ifndef SOFTADASTRA_SOFTWARE_PROJECT_CONFIG_HPP
#define SOFTADASTRA_SOFTWARE_PROJECT_CONFIG_HPP

#include "software/AccessPoint.hpp"
#include "software/ProjectIdentity.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace softadastra
{
  struct ProjectConfig
  {
    ProjectIdentity id;
    std::string name;
    std::string command;
    std::optional<AccessPoint> access;
  };

  class ProjectConfigFile
  {
  public:
    [[nodiscard]] static std::optional<std::pair<std::filesystem::path, ProjectConfig>> find(const std::filesystem::path &directory, std::string *error = nullptr);
    [[nodiscard]] static bool create(const std::filesystem::path &root, const ProjectConfig &config);
  };
}
#endif
