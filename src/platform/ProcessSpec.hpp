/**
 *
 *  @file ProcessSpec.hpp
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

#ifndef SOFTADASTRA_PLATFORM_PROCESS_SPEC_HPP
#define SOFTADASTRA_PLATFORM_PROCESS_SPEC_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace softadastra
{
  /**
   * @brief Describes how an external process should be launched.
   *
   * ProcessSpec contains only infrastructure information required to start
   * software. It does not describe the software language, framework, protocol,
   * database, or application architecture.
   */
  class ProcessSpec
  {
  public:
    /**
     * @brief Creates a process specification.
     *
     * @param executable Path or name of the executable to launch.
     * @param arguments Arguments passed to the executable.
     */
    explicit ProcessSpec(
        std::string executable,
        std::vector<std::string> arguments = {},
        std::optional<std::string> working_directory = std::nullopt,
        std::optional<std::string> output_file = std::nullopt)
        : executable_(std::move(executable)),
          arguments_(std::move(arguments)),
          working_directory_(std::move(working_directory)), output_file_(std::move(output_file))
    {
    }

    [[nodiscard]] const std::optional<std::string> &working_directory() const noexcept
    {
      return working_directory_;
    }
    [[nodiscard]] const std::optional<std::string> &output_file() const noexcept { return output_file_; }

    /**
     * @brief Returns the executable path or name.
     */
    [[nodiscard]] const std::string &executable() const noexcept
    {
      return executable_;
    }

    /**
     * @brief Returns the arguments passed to the executable.
     */
    [[nodiscard]] const std::vector<std::string> &arguments() const noexcept
    {
      return arguments_;
    }

    /**
     * @brief Resolves an executable path used during registration.
     *
     * A relative value containing a directory component is resolved against
     * the current working directory. Bare executable names are preserved for
     * PATH lookup, and absolute paths are preserved unchanged.
     *
     * @return The normalized executable, or std::nullopt when a relative path
     *         does not exist.
     */
    [[nodiscard]] static std::optional<std::string> normalize_executable(
        const std::string &executable)
    {
      const std::filesystem::path path(executable);

      if (!path.is_relative() || !path.has_parent_path())
      {
        return executable;
      }

      std::error_code error;
      const auto normalized = std::filesystem::weakly_canonical(path, error);

      if (error || !std::filesystem::exists(normalized, error) || error)
      {
        return std::nullopt;
      }

      return executable;
    }

  private:
    std::string executable_;
    std::vector<std::string> arguments_;
    std::optional<std::string> working_directory_;
    std::optional<std::string> output_file_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PROCESS_SPEC_HPP
