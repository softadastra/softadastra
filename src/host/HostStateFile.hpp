/**
 *
 *  @file HostStateFile.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_STATE_FILE_HPP
#define SOFTADASTRA_HOST_HOST_STATE_FILE_HPP

#include "host/HostState.hpp"

#include <filesystem>

namespace softadastra
{
  /**
   * @brief Describes the last failed Host state load operation.
   */
  enum class HostStateLoadError
  {
    None,
    FileUnavailable,
    InvalidContent,
    StateNotEmpty
  };

  /**
   * @brief Saves Host registration metadata to a local state file.
   */
  class HostStateFile
  {
  public:
    /**
     * @brief Creates a state file at an explicit location.
     */
    explicit HostStateFile(std::filesystem::path path) noexcept;

    /**
     * @brief Returns whether a saved Host state exists.
     */
    [[nodiscard]] bool exists() const noexcept;

    /**
     * @brief Saves registered software metadata atomically.
     */
    [[nodiscard]] bool save(const HostState &state) const;

    /**
     * @brief Restores registered software metadata into an empty Host state.
     *
     * Restored software is not associated with any process and therefore
     * remains Stopped.
     *
     * @return true when every registration was restored.
     */
    [[nodiscard]] bool load(HostState &state) const;

    /**
     * @brief Returns the last load failure cause.
     */
    [[nodiscard]] HostStateLoadError last_load_error() const noexcept;

  private:
    std::filesystem::path path_;
    mutable HostStateLoadError last_load_error_{HostStateLoadError::None};
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_STATE_FILE_HPP
