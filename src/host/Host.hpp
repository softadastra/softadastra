/**
 *
 *  @file Host.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_HPP
#define SOFTADASTRA_HOST_HOST_HPP

#include "host/HostState.hpp"
#include "platform/Platform.hpp"

namespace softadastra
{
  /**
   * @brief Represents a machine operating as a Softadastra Host.
   *
   * Host is the central infrastructure abstraction in Softadastra. It combines
   * the platform capabilities of a supported machine with the infrastructure
   * state owned by Softadastra.
   *
   * Host does not define the internal architecture of hosted software and does
   * not directly manage software lifecycle operations. Those responsibilities
   * belong to dedicated Host components.
   *
   * A Host follows the foundational Softadastra model:
   *
   * @code
   * Machine + Softadastra = Host
   * @endcode
   */
  class Host
  {
  public:
    /**
     * @brief Creates a Host on top of a supported platform.
     *
     * The Platform instance must remain valid for the lifetime of the Host.
     *
     * @param platform Platform capabilities provided by the machine.
     */
    explicit Host(Platform &platform) noexcept;

    /**
     * @brief Returns the infrastructure state owned by the Host.
     *
     * @return Reference to the Host state.
     */
    [[nodiscard]] HostState &state() noexcept;

    /**
     * @brief Returns the infrastructure state owned by the Host.
     *
     * @return Constant reference to the Host state.
     */
    [[nodiscard]] const HostState &state() const noexcept;

    /**
     * @brief Returns the platform capabilities available to the Host.
     *
     * @return Reference to the underlying platform.
     */
    [[nodiscard]] Platform &platform() noexcept;

    /**
     * @brief Returns the platform capabilities available to the Host.
     *
     * @return Constant reference to the underlying platform.
     */
    [[nodiscard]] const Platform &platform() const noexcept;

  private:
    Platform &platform_;
    HostState state_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_HPP
