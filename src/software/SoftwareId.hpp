/**
 *
 *  @file SoftwareId.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_SOFTWARE_ID_HPP
#define SOFTADASTRA_SOFTWARE_SOFTWARE_ID_HPP

#include <string>
#include <utility>

namespace softadastra
{
  /**
   * @brief Identifies software known to a Softadastra Host.
   *
   * SoftwareId is a strongly typed identifier used by Softadastra to refer to
   * hosted software without making assumptions about the software's language,
   * architecture, protocol, framework, or internal implementation.
   *
   * The identifier is infrastructure metadata owned by Softadastra. It does not
   * represent an identifier defined inside the hosted software itself.
   */
  class SoftwareId
  {
  public:
    /**
     * @brief Creates a software identifier from a string value.
     *
     * @param value Identifier value.
     */
    explicit SoftwareId(std::string value)
        : value_(std::move(value))
    {
    }

    /**
     * @brief Returns the underlying identifier value.
     *
     * @return Constant reference to the identifier string.
     */
    [[nodiscard]] const std::string &value() const noexcept
    {
      return value_;
    }

    /**
     * @brief Compares two software identifiers.
     *
     * Two identifiers are equal when their underlying string values are equal.
     */
    [[nodiscard]] bool operator==(const SoftwareId &) const noexcept = default;

  private:
    std::string value_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_ID_HPP
