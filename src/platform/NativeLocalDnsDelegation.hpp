/**
 *
 *  @file NativeLocalDnsDelegation.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_LOCAL_DNS_DELEGATION_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_LOCAL_DNS_DELEGATION_HPP

#include "platform/LocalDnsDelegation.hpp"

#include <filesystem>
#include <utility>

namespace softadastra
{
  /**
   * @brief Inspects native local DNS delegation configuration.
   */
  class NativeLocalDnsDelegation final : public LocalDnsDelegation
  {
  public:
    /**
     * @brief Creates a native local DNS delegation inspector.
     *
     * @param directory Directory containing DNS delegation configuration.
     */
    explicit NativeLocalDnsDelegation(
        std::filesystem::path directory =
            "/etc/NetworkManager/dnsmasq-shared.d")
        : directory_(std::move(directory))
    {
    }

    /**
     * @brief Returns the current local DNS delegation state.
     *
     * @return Current delegation state.
     */
    [[nodiscard]] LocalDnsDelegationState status() const override;

  private:
    std::filesystem::path directory_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_LOCAL_DNS_DELEGATION_HPP
