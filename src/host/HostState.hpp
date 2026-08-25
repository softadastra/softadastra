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

#include "software/SoftwareEntry.hpp"
#include "software/SoftwareId.hpp"
#include "software/ProjectIdentity.hpp"

#include <cstddef>
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
     * A Host cannot contain more than one software entry with the same
     * SoftwareId.
     *
     * @param entry Software entry to add.
     *
     * @return true if the entry was added, otherwise false if an entry with the
     *         same identifier already exists.
     */
    bool add_software(SoftwareEntry entry);
    bool remove_software(const SoftwareId &id);

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
    [[nodiscard]] SoftwareEntry *find_software_by_name(const std::string &name) noexcept;
    [[nodiscard]] const SoftwareEntry *find_software_by_name(const std::string &name) const noexcept;

    [[nodiscard]] SoftwareEntry *find_software(const ProjectIdentity &identity) noexcept;
    [[nodiscard]] const SoftwareEntry *find_software(const ProjectIdentity &identity) const noexcept;

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
     * @brief Returns registered software infrastructure metadata.
     */
    [[nodiscard]] const std::vector<SoftwareEntry> &software() const noexcept;

  private:
    std::vector<SoftwareEntry> software_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_STATE_HPP
