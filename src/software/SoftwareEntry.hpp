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

#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <utility>

namespace softadastra
{
  /**
   * @brief Represents software registered with a Host.
   *
   * SoftwareEntry stores only Host-owned infrastructure information required
   * to identify, launch, and track software.
   *
   * It does not describe the software language, framework, protocol, database,
   * or application architecture.
   */
  class SoftwareEntry
  {
  public:
    /**
     * @brief Creates a software entry.
     *
     * @param id Stable identifier used by the Host.
     * @param process_spec Information required to launch the software process.
     */
    SoftwareEntry(SoftwareId id, ProcessSpec process_spec)
        : id_(std::move(id)),
          process_spec_(std::move(process_spec))
    {
    }

    /**
     * @brief Returns the software identifier.
     */
    [[nodiscard]] const SoftwareId &id() const noexcept
    {
      return id_;
    }

    /**
     * @brief Returns the process launch specification.
     */
    [[nodiscard]] const ProcessSpec &process_spec() const noexcept
    {
      return process_spec_;
    }

    /**
     * @brief Returns the current software lifecycle state.
     */
    [[nodiscard]] SoftwareState state() const noexcept
    {
      return state_;
    }

    /**
     * @brief Updates the software lifecycle state.
     *
     * @param state New lifecycle state.
     */
    void set_state(SoftwareState state) noexcept
    {
      state_ = state;
    }

  private:
    SoftwareId id_;
    ProcessSpec process_spec_;
    SoftwareState state_{SoftwareState::Stopped};
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_SOFTWARE_ENTRY_HPP
