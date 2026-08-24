/**
 *
 *  @file QrCode.cpp
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

#include "platform/QrCode.hpp"

#include "internal/QrEncoder.hpp"

#include <iostream>

namespace softadastra
{
  std::string QrCode::render(std::string_view content) noexcept
  {
    try
    {
      return internal::generate(content).to_ascii();
    }
    catch (...)
    {
      return {};
    }
  }

  bool QrCode::print(std::string_view content) noexcept
  {
    const std::string qr = render(content);
    if (qr.empty())
    {
      return false;
    }

    std::cout << qr;
    return true;
  }

} // namespace softadastra
