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
        std::vector<std::string> arguments = {})
        : executable_(std::move(executable)),
          arguments_(std::move(arguments))
    {
    }

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

  private:
    std::string executable_;
    std::vector<std::string> arguments_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PROCESS_SPEC_HPP
