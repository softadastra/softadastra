/**
 *
 *  @file NativeLocalFirewall.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_LOCAL_FIREWALL_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_LOCAL_FIREWALL_HPP

#include "platform/LocalFirewall.hpp"

#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Represents the result of executing a UFW command.
   */
  struct UfwCommandResult
  {
    /**
     * @brief Process exit code.
     */
    int exit_code{-1};

    /**
     * @brief Combined command output.
     */
    std::string output;
  };

  /**
   * @brief Provides an interface for executing UFW commands.
   */
  class UfwCommandRunner
  {
  public:
    /**
     * @brief Destroys the UFW command runner.
     */
    virtual ~UfwCommandRunner() = default;

    /**
     * @brief Executes a UFW command.
     *
     * @param arguments Arguments passed to UFW.
     *
     * @return Command execution result.
     */
    [[nodiscard]] virtual UfwCommandResult run(
        const std::vector<std::string> &arguments) = 0;
  };

  /**
   * @brief Provides the native local firewall implementation backed by UFW.
   *
   * Unsupported platforms report LocalFirewallResult::Unsupported for
   * platform-dependent operations.
   */
  class NativeLocalFirewall final : public LocalFirewall
  {
  public:
    /**
     * @brief Creates a native local firewall using the system UFW command.
     */
    NativeLocalFirewall() = default;

    /**
     * @brief Creates a native local firewall with a custom UFW command runner.
     *
     * @param runner Command runner used to execute UFW operations.
     */
    explicit NativeLocalFirewall(
        UfwCommandRunner &runner) noexcept;

    /**
     * @brief Builds the UFW arguments for a Host-owned local TCP allow rule.
     *
     * @param rule Firewall rule.
     *
     * @return UFW command arguments.
     */
    [[nodiscard]] static std::vector<std::string> allow_arguments(
        const LocalFirewallRule &rule);

    /**
     * @brief Builds the UFW arguments for non-interactive rule deletion.
     *
     * @param number UFW rule number to delete.
     *
     * @return UFW command arguments.
     */
    [[nodiscard]] static std::vector<std::string> delete_arguments(
        int number);

    /**
     * @brief Checks whether a local firewall rule is usable.
     *
     * @param rule Firewall rule to inspect.
     *
     * @return Result of the firewall inspection.
     */
    [[nodiscard]] LocalFirewallResult ensure(
        const LocalFirewallRule &rule) override;

    /**
     * @brief Returns the current state of a local firewall rule.
     *
     * @param rule Firewall rule to inspect.
     *
     * @return Current firewall result for the rule.
     */
    [[nodiscard]] LocalFirewallResult status(
        const LocalFirewallRule &rule) override;

    /**
     * @brief Enables a Host-owned local firewall rule when required.
     *
     * @param rule Firewall rule to enable.
     *
     * @return Result of the firewall operation.
     */
    [[nodiscard]] LocalFirewallResult allow(
        const LocalFirewallRule &rule) override;

    /**
     * @brief Releases matching rules carrying the Host ownership tag.
     *
     * @param rule Firewall rule identifying the owned rules to release.
     */
    void release(
        const LocalFirewallRule &rule) noexcept override;

    /**
     * @brief Removes matching rules carrying the Host ownership tag.
     *
     * @param rule Firewall rule identifying the owned rules to remove.
     *
     * @return Result of the firewall operation.
     */
    [[nodiscard]] LocalFirewallResult deny(
        const LocalFirewallRule &rule) override;

  private:
    UfwCommandRunner *runner_{nullptr};
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_LOCAL_FIREWALL_HPP
