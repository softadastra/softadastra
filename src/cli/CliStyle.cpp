/**
 *
 *  @file CliStyle.cpp
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

#include "cli/CliStyle.hpp"

#include <string>
#include <string_view>

#if defined(_WIN32)

#include <io.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#else

#include <cstdlib>
#include <unistd.h>

#endif

namespace softadastra::cli::style
{
  namespace
  {
    // Select-Graphic-Rendition codes. Only the eight base colors and the dim
    // and bold attributes are used: they render legibly on both light and dark
    // terminals, unlike fixed grays or colored backgrounds.
    constexpr std::string_view reset = "\033[0m";
    constexpr std::string_view accent_code = "\033[36m";  // cyan
    constexpr std::string_view success_code = "\033[32m"; // green
    constexpr std::string_view error_code = "\033[31m";   // red
    constexpr std::string_view warning_code = "\033[33m"; // yellow
    constexpr std::string_view muted_code = "\033[2m";    // dim
    constexpr std::string_view label_code = "\033[2m";    // dim
    constexpr std::string_view command_code = "\033[36m"; // cyan

    bool has_no_color() noexcept
    {
      // The NO_COLOR convention: any non-empty value disables color.
#if defined(_WIN32)
      const DWORD size =
          GetEnvironmentVariableA(
              "NO_COLOR",
              nullptr,
              0);

      return size != 0;
#else
      const char *const value =
          std::getenv("NO_COLOR");

      return value != nullptr &&
             value[0] != '\0';
#endif
    }

#if defined(_WIN32)

    bool enable_virtual_terminal(
        DWORD handle_id) noexcept
    {
      const HANDLE handle =
          GetStdHandle(handle_id);

      if (handle == INVALID_HANDLE_VALUE ||
          handle == nullptr)
      {
        return false;
      }

      DWORD mode = 0;

      if (!GetConsoleMode(
              handle,
              &mode))
      {
        return false;
      }

      if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
      {
        return true;
      }

      return SetConsoleMode(
          handle,
          mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    bool detect(
        Stream stream) noexcept
    {
      if (has_no_color())
      {
        return false;
      }

      const int descriptor =
          stream == Stream::Out ? 1 : 2;

      if (!_isatty(descriptor))
      {
        return false;
      }

      return enable_virtual_terminal(
          stream == Stream::Out
              ? STD_OUTPUT_HANDLE
              : STD_ERROR_HANDLE);
    }

#else

    bool detect(
        Stream stream) noexcept
    {
      if (has_no_color())
      {
        return false;
      }

      const int descriptor =
          stream == Stream::Out
              ? STDOUT_FILENO
              : STDERR_FILENO;

      if (!isatty(descriptor))
      {
        return false;
      }

      // A dumb or absent terminal cannot be assumed to interpret ANSI.
      const char *const term =
          std::getenv("TERM");

      return term != nullptr &&
             term[0] != '\0' &&
             std::string_view(term) != "dumb";
    }

#endif

    bool enabled(
        Stream stream)
    {
      // Computed once per stream and cached for the process lifetime.
      static const bool out =
          detect(Stream::Out);
      static const bool err =
          detect(Stream::Err);

      return stream == Stream::Out ? out : err;
    }

    std::string wrap(
        std::string_view code,
        std::string_view text,
        Stream stream)
    {
      if (!enabled(stream))
      {
        return std::string(text);
      }

      std::string value;
      value.reserve(
          code.size() +
          text.size() +
          reset.size());

      value.append(code);
      value.append(text);
      value.append(reset);

      return value;
    }

  } // namespace

  bool colored(
      Stream stream)
  {
    return enabled(stream);
  }

  std::string accent(
      std::string_view text,
      Stream stream)
  {
    return wrap(
        accent_code,
        text,
        stream);
  }

  std::string success(
      std::string_view text,
      Stream stream)
  {
    return wrap(
        success_code,
        text,
        stream);
  }

  std::string error(
      std::string_view text,
      Stream stream)
  {
    return wrap(
        error_code,
        text,
        stream);
  }

  std::string warning(
      std::string_view text,
      Stream stream)
  {
    return wrap(
        warning_code,
        text,
        stream);
  }

  std::string muted(
      std::string_view text,
      Stream stream)
  {
    return wrap(
        muted_code,
        text,
        stream);
  }

  std::string label(
      std::string_view text,
      Stream stream)
  {
    return wrap(
        label_code,
        text,
        stream);
  }

  std::string command(
      std::string_view text,
      Stream stream)
  {
    return wrap(
        command_code,
        text,
        stream);
  }

  std::string field(
      std::string_view field_label,
      std::string_view field_value,
      std::size_t width,
      Stream stream)
  {
    std::string value =
        label(
            field_label,
            stream);

    // Padding is measured on the raw label so that alignment is identical
    // whether or not escape sequences were added above.
    if (field_label.size() < width)
    {
      value.append(
          width - field_label.size(),
          ' ');
    }

    value.append(field_value);

    return value;
  }

  std::string arrow(
      Stream stream)
  {
    return accent(
        "->",
        stream);
  }

} // namespace softadastra::cli::style
