/**
 *
 *  @file ProjectIdentity.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_PROJECT_IDENTITY_HPP
#define SOFTADASTRA_SOFTWARE_PROJECT_IDENTITY_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace softadastra
{
  /**
   * @brief Represents the persistent opaque identity of a project.
   *
   * ProjectIdentity provides a stable identifier that can be stored within
   * a project and discovered by walking the directory hierarchy.
   */
  class ProjectIdentity
  {
  public:
    /**
     * @brief Creates a project identity from an existing value.
     *
     * @param value Identity value.
     */
    explicit ProjectIdentity(std::string value)
        : value_(std::move(value))
    {
    }

    /**
     * @brief Returns the underlying identity value.
     *
     * @return Identity value.
     */
    [[nodiscard]] const std::string &value() const noexcept
    {
      return value_;
    }

    /**
     * @brief Compares two project identities.
     */
    [[nodiscard]] bool operator==(
        const ProjectIdentity &) const noexcept = default;

    /**
     * @brief Creates and stores a new identity for a project.
     *
     * An existing project identity is not replaced.
     *
     * @param root Project root directory.
     *
     * @return The newly created identity, or std::nullopt if an identity
     *         already exists or cannot be stored.
     */
    [[nodiscard]] static std::optional<ProjectIdentity> create(
        const std::filesystem::path &root);

    /**
     * @brief Generates a new project identity.
     *
     * @return Newly generated project identity.
     */
    [[nodiscard]] static ProjectIdentity generate();

    /**
     * @brief Finds a project identity by walking toward the filesystem root.
     *
     * @param directory Directory from which to begin the search.
     *
     * @return The project root and identity when found, or std::nullopt
     *         otherwise.
     */
    [[nodiscard]] static std::optional<
        std::pair<std::filesystem::path, ProjectIdentity>>
    find(const std::filesystem::path &directory);

  private:
    std::string value_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_PROJECT_IDENTITY_HPP
