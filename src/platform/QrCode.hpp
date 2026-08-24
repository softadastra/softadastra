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

#include <string>
#include <string_view>

namespace softadastra
{
  /**
   * @brief Renders QR codes for the terminal.
   */
  class QrCode
  {
  public:
    /** @brief Renders content as a UTF-8 terminal QR code. */
    [[nodiscard]] static std::string render(std::string_view content) noexcept;

    /** @brief Prints content as a UTF-8 terminal QR code. */
    [[nodiscard]] static bool print(std::string_view content) noexcept;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_QR_CODE_HPP
