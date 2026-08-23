/**
 *
 *  @file NativeDataDirectory.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_DATA_DIRECTORY_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_DATA_DIRECTORY_HPP

#include <filesystem>

namespace softadastra
{
  /**
   * @brief Resolves the native user data directory owned by Softadastra.
   */
  class NativeDataDirectory
  {
  public:
    /**
     * @brief Returns the stable directory used for Softadastra state.
     */
    [[nodiscard]] static std::filesystem::path path();

    /**
     * @brief Creates the Softadastra state directory when necessary.
     *
     * @return true when the directory exists after the call.
     */
    static bool ensure_exists();
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_DATA_DIRECTORY_HPP
