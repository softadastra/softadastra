/**
 *
 *  @file LocalDnsDelegation.hpp
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

#ifndef SOFTADASTRA_PLATFORM_LOCAL_DNS_DELEGATION_HPP
#define SOFTADASTRA_PLATFORM_LOCAL_DNS_DELEGATION_HPP

#include <string>

namespace softadastra
{
  /**
   * @brief Describes the state of local DNS delegation.
   */
  enum class LocalDnsDelegationState
  {
    Available,
    Unavailable,
    Misconfigured
  };

  /**
   * @brief Provides the platform interface for local DNS delegation.
   */
  class LocalDnsDelegation
  {
  public:
    /**
     * @brief Destroys the local DNS delegation interface.
     */
    virtual ~LocalDnsDelegation() = default;

    /**
     * @brief Returns the current local DNS delegation state.
     *
     * @return Current delegation state.
     */
    [[nodiscard]] virtual LocalDnsDelegationState status() const = 0;

    /**
     * @brief Returns the local DNS delegation configuration.
     *
     * @return Configuration required for local DNS delegation.
     */
    [[nodiscard]] static std::string configuration();
  };

  /**
   * @brief Represents a platform without local DNS delegation support.
   */
  class UnavailableLocalDnsDelegation final
      : public LocalDnsDelegation
  {
  public:
    /**
     * @brief Returns the unavailable delegation state.
     *
     * @return LocalDnsDelegationState::Unavailable.
     */
    [[nodiscard]] LocalDnsDelegationState status() const override
    {
      return LocalDnsDelegationState::Unavailable;
    }
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_LOCAL_DNS_DELEGATION_HPP
