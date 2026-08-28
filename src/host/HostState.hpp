/**
 *
 *  @file HostState.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_STATE_HPP
#define SOFTADASTRA_HOST_HOST_STATE_HPP

#include "software/ProjectIdentity.hpp"
#include "software/SoftwareEntry.hpp"
#include "software/SoftwareId.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Stores infrastructure state owned by a Softadastra Host.
   *
   * HostState owns the software entries known to the Host. It stores only
   * Softadastra infrastructure metadata and does not contain business data,
   * application users, application sessions, or other state owned internally
   * by hosted software.
   *
   * Lifecycle operations such as starting and stopping software do not belong
   * to HostState. Those operations are handled by higher-level Host components.
   */
  class HostState
  {
  public:
    /**
     * @brief Adds a software entry to the Host state.
     *
     * Software identifiers must be unique. Non-empty software names must also
     * be unique within the Host state.
     *
     * @param entry Software entry to add.
     *
     * @return true if the entry was added successfully, otherwise false.
     */
    bool add_software(
        SoftwareEntry entry);

    /**
     * @brief Removes a software entry by identifier.
     *
     * @param id Identifier of the software to remove.
     *
     * @return true if a matching entry was removed, otherwise false.
     */
    bool remove_software(
        const SoftwareId &id);

    /**
     * @brief Finds a software entry by identifier.
     *
     * @param id Identifier of the software to find.
     *
     * @return Pointer to the matching entry, or nullptr if no entry exists.
     */
    [[nodiscard]] SoftwareEntry *find_software(
        const SoftwareId &id) noexcept;

    /**
     * @brief Finds a software entry by identifier.
     *
     * @param id Identifier of the software to find.
     *
     * @return Constant pointer to the matching entry, or nullptr if no entry
     *         exists.
     */
    [[nodiscard]] const SoftwareEntry *find_software(
        const SoftwareId &id) const noexcept;

    /**
     * @brief Finds a software entry by name.
     *
     * @param name Software name to find.
     *
     * @return Pointer to the matching entry, or nullptr if no entry exists.
     */
    [[nodiscard]] SoftwareEntry *find_software_by_name(
        const std::string &name) noexcept;

    /**
     * @brief Finds a software entry by name.
     *
     * @param name Software name to find.
     *
     * @return Constant pointer to the matching entry, or nullptr if no entry
     *         exists.
     */
    [[nodiscard]] const SoftwareEntry *find_software_by_name(
        const std::string &name) const noexcept;

    /**
     * @brief Finds a software entry by project identity.
     *
     * @param identity Project identity to find.
     *
     * @return Pointer to the matching entry, or nullptr if no entry exists.
     */
    [[nodiscard]] SoftwareEntry *find_software(
        const ProjectIdentity &identity) noexcept;

    /**
     * @brief Finds a software entry by project identity.
     *
     * @param identity Project identity to find.
     *
     * @return Constant pointer to the matching entry, or nullptr if no entry
     *         exists.
     */
    [[nodiscard]] const SoftwareEntry *find_software(
        const ProjectIdentity &identity) const noexcept;

    /**
     * @brief Returns the number of software entries known to the Host.
     *
     * @return Number of software entries.
     */
    [[nodiscard]] std::size_t software_count() const noexcept;

    /**
     * @brief Checks whether the Host state contains no software entries.
     *
     * @return true if no software entries are stored, otherwise false.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Returns all software entries stored by the Host.
     *
     * @return Constant reference to the registered software entries.
     */
    [[nodiscard]] const std::vector<SoftwareEntry> &
    software() const noexcept;

  private:
    std::vector<SoftwareEntry> software_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_STATE_HPP
