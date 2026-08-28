/**
 *
 *  @file ProjectConfig.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_PROJECT_CONFIG_HPP
#define SOFTADASTRA_SOFTWARE_PROJECT_CONFIG_HPP

#include "software/AccessPoint.hpp"
#include "software/ProjectIdentity.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace softadastra
{
  /**
   * @brief Describes the software configuration of a project.
   */
  struct ProjectConfig
  {
    /**
     * @brief Stable identity of the project.
     */
    ProjectIdentity id;

    /**
     * @brief Human-readable project name.
     */
    std::string name;

    /**
     * @brief Command used to run the project software.
     */
    std::string command;

    /**
     * @brief Optional primary access point exposed by the project.
     */
    std::optional<AccessPoint> access;

    /**
     * @brief Access points exposed by the project.
     */
    std::vector<AccessPoint> access_points;
  };

  /**
   * @brief Provides project configuration file discovery and creation.
   */
  class ProjectConfigFile
  {
  public:
    /**
     * @brief Finds and loads a project configuration from a directory.
     *
     * @param directory Directory from which to search for the configuration.
     * @param error Optional destination for an error description.
     *
     * @return The configuration file path and parsed project configuration,
     *         or std::nullopt if no valid configuration is found.
     */
    [[nodiscard]] static std::optional<
        std::pair<std::filesystem::path, ProjectConfig>>
    find(
        const std::filesystem::path &directory,
        std::string *error = nullptr);

    /**
     * @brief Creates a project configuration file.
     *
     * @param root Project root directory.
     * @param config Configuration to write.
     *
     * @return true if the configuration was created successfully, otherwise false.
     */
    [[nodiscard]] static bool create(
        const std::filesystem::path &root,
        const ProjectConfig &config);
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_PROJECT_CONFIG_HPP
