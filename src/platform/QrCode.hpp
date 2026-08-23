/**
 *
 *  @file QrCode.hpp
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

#ifndef SOFTADASTRA_PLATFORM_QR_CODE_HPP
#define SOFTADASTRA_PLATFORM_QR_CODE_HPP

#include <string_view>

namespace softadastra
{
  /**
   * @brief Prints QR codes through an optional native terminal generator.
   */
  class QrCode
  {
  public:
    /**
     * @brief Returns whether the native QR generator is available.
     */
    [[nodiscard]] static bool available(
        std::string_view executable = "/usr/bin/qrencode") noexcept;

    /**
     * @brief Prints content as a UTF-8 terminal QR code when available.
     */
    [[nodiscard]] static bool print(std::string_view content) noexcept;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_QR_CODE_HPP
