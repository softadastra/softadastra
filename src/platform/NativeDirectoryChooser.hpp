/**
 *
 *  @file NativeDirectoryChooser.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_DIRECTORY_CHOOSER_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_DIRECTORY_CHOOSER_HPP

#include <filesystem>

namespace softadastra
{
  /**
   * @brief Describes the outcome of a native directory selection request.
   */
  enum class DirectoryChooserStatus
  {
    Selected,
    Cancelled,
    Unavailable
  };

  /**
   * @brief Represents the result of a native directory selection request.
   */
  struct DirectoryChooserResult
  {
    /**
     * @brief Status of the directory selection operation.
     */
    DirectoryChooserStatus status{
        DirectoryChooserStatus::Unavailable};

    /**
     * @brief Selected directory path when available.
     */
    std::filesystem::path path;
  };

  /**
   * @brief Opens the operating system's native directory chooser.
   *
   * @return Directory selection result.
   */
  [[nodiscard]] DirectoryChooserResult choose_project_directory();

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_DIRECTORY_CHOOSER_HPP
