/**
 *
 *  @file CliStyle.hpp
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

#ifndef SOFTADASTRA_CLI_CLISTYLE_HPP
#define SOFTADASTRA_CLI_CLISTYLE_HPP

#include <cstddef>
#include <string>
#include <string_view>

namespace softadastra::cli::style
{
  /**
   * @brief Identifies which output stream a fragment is destined for.
   *
   * Color is decided independently per stream, so a redirected stdout never
   * suppresses color on an interactive stderr, and vice versa.
   */
  enum class Stream
  {
    Out,
    Err
  };

  /**
   * @brief Reports whether colored output is enabled for a given stream.
   *
   * The decision is computed once, lazily, on first use. Color is enabled only
   * when the stream is a real terminal, NO_COLOR is unset, and the platform
   * terminal supports ANSI sequences. Any redirection or pipe disables it, so
   * captured output and log files never receive escape sequences.
   *
   * @param stream Stream whose color capability is queried.
   *
   * @return true when ANSI color may be emitted for @p stream.
   */
  bool colored(
      Stream stream = Stream::Out);

  /**
   * @brief Marks text as the Softadastra brand accent.
   *
   * Reserved for identity and primary elements. Used sparingly; overuse defeats
   * the visual hierarchy it is meant to establish.
   *
   * @param text Text to style.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The styled text, or @p text unchanged when color is disabled.
   */
  std::string accent(
      std::string_view text,
      Stream stream = Stream::Out);

  /**
   * @brief Marks text as a successful outcome.
   *
   * @param text Text to style.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The styled text, or @p text unchanged when color is disabled.
   */
  std::string success(
      std::string_view text,
      Stream stream = Stream::Out);

  /**
   * @brief Marks text as an error.
   *
   * @param text Text to style.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The styled text, or @p text unchanged when color is disabled.
   */
  std::string error(
      std::string_view text,
      Stream stream = Stream::Err);

  /**
   * @brief Marks text as a warning or a degraded, non-fatal state.
   *
   * @param text Text to style.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The styled text, or @p text unchanged when color is disabled.
   */
  std::string warning(
      std::string_view text,
      Stream stream = Stream::Out);

  /**
   * @brief Marks text as secondary and de-emphasized.
   *
   * The dim role is the foundation of the layout: paths, ports, defaults,
   * hints, and table headers recede so that primary values stand out without
   * requiring a vivid color.
   *
   * @param text Text to style.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The styled text, or @p text unchanged when color is disabled.
   */
  std::string muted(
      std::string_view text,
      Stream stream = Stream::Out);

  /**
   * @brief Marks text as a field label.
   *
   * @param text Text to style.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The styled text, or @p text unchanged when color is disabled.
   */
  std::string label(
      std::string_view text,
      Stream stream = Stream::Out);

  /**
   * @brief Marks text as a command the reader is expected to type.
   *
   * @param text Text to style.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The styled text, or @p text unchanged when color is disabled.
   */
  std::string command(
      std::string_view text,
      Stream stream = Stream::Out);

  /**
   * @brief Formats an aligned label/value pair.
   *
   * The label is styled with the label role and padded to @p width columns; the
   * value follows verbatim in the terminal's default color so that it stays
   * readable on both light and dark backgrounds. Padding is measured on the raw
   * label text, never on the emitted escape sequences, so alignment is
   * identical whether or not color is enabled.
   *
   * @param field_label Label shown on the left.
   * @param field_value Value shown after the padded label.
   * @param width Column width the label is padded to.
   * @param stream Destination stream governing whether color is applied.
   *
   * @return The formatted "label value" line, without a trailing newline.
   */
  std::string field(
      std::string_view field_label,
      std::string_view field_value,
      std::size_t width,
      Stream stream = Stream::Out);

  /**
   * @brief Returns an arrow marker suitable for URLs and commands.
   *
   * A single Unicode arrow is used when color is enabled and the terminal is
   * assumed to render it; otherwise an ASCII fallback keeps the marker legible
   * in plain-text output and captured logs.
   *
   * @param stream Destination stream governing which marker is returned.
   *
   * @return The arrow marker, styled with the accent role when color applies.
   */
  std::string arrow(
      Stream stream = Stream::Out);

} // namespace softadastra::cli::style

#endif // SOFTADASTRA_CLI_CLISTYLE_HPP
