/**
 *
 *  @file SoftwareEntry.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP
#define SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP

#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <utility>

namespace softadastra
{
  /**
   * @brief Represents software known to a Softadastra Host.
   *
   * SoftwareEntry contains the minimal infrastructure information Softadastra
   * needs to identify hosted software and track its lifecycle state.
   *
   * It does not describe the software's programming language, framework,
   * protocol, database, business logic, or internal architecture.
   */
  class SoftwareEntry
  {
  public:
    /**
     * @brief Creates a software entry.
     *
     * New software entries begin in the Stopped state.
     *
     * @param id Identifier assigned to the software.
     */
    explicit SoftwareEntry(SoftwareId id)
        : id_(std::move(id))
    {
    }

    /**
     * @brief Returns the software identifier.
     *
     * @return Constant reference to the software identifier.
     */
    [[nodiscard]] const SoftwareId &id() const noexcept
    {
      return id_;
    }

    /**
     * @brief Returns the current infrastructure lifecycle state.
     *
     * @return Current software state.
     */
    [[nodiscard]] SoftwareState state() const noexcept
    {
      return state_;
    }

    /**
     * @brief Updates the infrastructure lifecycle state.
     *
     * This operation changes only the state tracked by Softadastra. It does not
     * modify any state owned internally by the hosted software.
     *
     * @param state New lifecycle state.
     */
    void set_state(SoftwareState state) noexcept
    {
      state_ = state;
    }

  private:
    SoftwareId id_;
    SoftwareState state_{SoftwareState::Stopped};
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP
