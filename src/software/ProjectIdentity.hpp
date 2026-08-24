/**
 *
 *  @file ProjectIdentity.hpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */
#ifndef SOFTADASTRA_SOFTWARE_PROJECT_IDENTITY_HPP
#define SOFTADASTRA_SOFTWARE_PROJECT_IDENTITY_HPP

#include <filesystem>
#include <optional>
#include <string>

namespace softadastra
{
  class ProjectIdentity
  {
  public:
    explicit ProjectIdentity(std::string value) : value_(std::move(value)) {}

    [[nodiscard]] const std::string &value() const noexcept { return value_; }
    [[nodiscard]] bool operator==(const ProjectIdentity &) const noexcept = default;

    /** @brief Creates a project-local opaque identity without replacing one. */
    [[nodiscard]] static std::optional<ProjectIdentity> create(const std::filesystem::path &root);
    [[nodiscard]] static ProjectIdentity generate();
    /** @brief Finds a project identity by walking from a directory to the root. */
    [[nodiscard]] static std::optional<std::pair<std::filesystem::path, ProjectIdentity>> find(
        const std::filesystem::path &directory);

  private:
    std::string value_;
  };
}
#endif // SOFTADASTRA_SOFTWARE_PROJECT_IDENTITY_HPP
