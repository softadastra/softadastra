/**
 *
 *  @file MdnsPublisher.hpp
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

#ifndef SOFTADASTRA_PLATFORM_MDNS_PUBLISHER_HPP
#define SOFTADASTRA_PLATFORM_MDNS_PUBLISHER_HPP

#include <array>
#include <string>
#include <string_view>

namespace softadastra
{
  /** @brief Publishes a stable Host name through the Linux mDNS service. */
  class MdnsPublisher
  {
  public:
    /** @brief Creates a collision-resistant local name from a public HostId. */
    explicit MdnsPublisher(std::string host_id) noexcept;

    ~MdnsPublisher();
    MdnsPublisher(const MdnsPublisher &) = delete;

    /** @brief Returns whether the supported Linux publisher is installed. */
    [[nodiscard]] static bool available() noexcept;

    /** @brief Returns the stable local mDNS name. */
    [[nodiscard]] const std::string &name() const noexcept;

    /** @brief Returns the Avahi arguments for direct address publication. */
    [[nodiscard]] static std::array<std::string, 3> publisher_arguments(
        std::string_view name,
        std::string_view ipv4);

    /** @brief Starts publishing the provided current IPv4 address. */
    [[nodiscard]] bool start(const std::string &ipv4) noexcept;

    /** @brief Stops publishing the local name. */
    void stop() noexcept;

  private:
    std::string name_;
    int process_{-1};
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_MDNS_PUBLISHER_HPP
