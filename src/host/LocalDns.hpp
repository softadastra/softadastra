/**
 *
 *  @file LocalDns.hpp
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

#ifndef SOFTADASTRA_HOST_LOCAL_DNS_HPP
#define SOFTADASTRA_HOST_LOCAL_DNS_HPP

#include "host/LocalReachability.hpp"

#include <cstdint>
#include <string>

namespace softadastra
{
  /**
   * @brief Describes the runtime state of the local DNS service.
   */
  enum class LocalDnsState
  {
    Stopped,
    Running,
    Failed
  };

  /**
   * @brief Describes the current local DNS service status.
   */
  struct LocalDnsStatus
  {
    /**
     * @brief Current service state.
     */
    LocalDnsState state{
        LocalDnsState::Stopped};

    /**
     * @brief IPv4 address on which the DNS service is listening.
     */
    std::string address;

    /**
     * @brief UDP port on which the DNS service is listening.
     */
    std::uint16_t port{};
  };

  /**
   * @brief Provides the local DNS endpoint used for software name resolution.
   */
  class LocalDns final : public LocalDnsEndpoint
  {
  public:
    /**
     * @brief Creates a stopped local DNS service.
     */
    LocalDns() = default;

    /**
     * @brief Stops the service and releases its resources.
     */
    ~LocalDns();

    LocalDns(const LocalDns &) = delete;
    LocalDns &operator=(const LocalDns &) = delete;

    /**
     * @brief Starts the local DNS service.
     *
     * @param address IPv4 address on which to listen.
     * @param port UDP port on which to listen.
     *
     * @return true if the service started successfully, otherwise false.
     */
    bool start(
        std::string address,
        std::uint16_t port) override;

    /**
     * @brief Stops the local DNS service.
     */
    void stop() noexcept override;

    /**
     * @brief Returns the current local DNS service status.
     *
     * @return Current service status.
     */
    [[nodiscard]] LocalDnsStatus status() const;

  private:
    void run() noexcept;

    int descriptor_{-1};
    LocalDnsStatus status_{};
    bool stopping_{false};

    class Thread;
    Thread *thread_{nullptr};
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_LOCAL_DNS_HPP
