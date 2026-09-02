/**
 *
 *  @file PrivilegedLocalFirewall.hpp
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

#ifndef SOFTADASTRA_PLATFORM_PRIVILEGED_LOCAL_FIREWALL_HPP
#define SOFTADASTRA_PLATFORM_PRIVILEGED_LOCAL_FIREWALL_HPP

#include "platform/LocalFirewall.hpp"

#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Represents the result of executing a privileged process.
   */
  struct PrivilegedProcessResult
  {
    /**
     * @brief Process exit code.
     */
    int exit_code{-1};

    /**
     * @brief Combined process output.
     */
    std::string output;
  };

  /**
   * @brief Provides an interface for executing privileged processes.
   */
  class PrivilegedProcessRunner
  {
  public:
    /**
     * @brief Destroys the privileged process runner.
     */
    virtual ~PrivilegedProcessRunner() = default;

    /**
     * @brief Executes a privileged process.
     *
     * @param program Program to execute.
     * @param arguments Arguments passed to the program.
     *
     * @return Process execution result.
     */
    [[nodiscard]] virtual PrivilegedProcessResult run(
        const std::string &program,
        const std::vector<std::string> &arguments) = 0;
  };

  /**
   * @brief Provides the native privileged process runner.
   */
  class NativePrivilegedProcessRunner final : public PrivilegedProcessRunner
  {
  public:
    /**
     * @brief Executes a process and captures its combined output.
     *
     * @param program Program to execute.
     * @param arguments Arguments passed to the program.
     *
     * @return Process execution result.
     */
    [[nodiscard]] PrivilegedProcessResult run(
        const std::string &program,
        const std::vector<std::string> &arguments) override;
  };

  /**
   * @brief Provides local firewall operations through privileged helper
   * processes.
   */
  class PrivilegedLocalFirewall final : public LocalFirewall
  {
  public:
    /**
     * @brief Creates a privileged local firewall.
     *
     * @param runner Runner used to execute privileged helper processes.
     */
    explicit PrivilegedLocalFirewall(
        PrivilegedProcessRunner &runner) noexcept
        : runner_(runner)
    {
    }

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
     * @brief Enables a Host-owned local firewall rule.
     *
     * @param rule Firewall rule to enable.
     *
     * @return Result of the firewall operation.
     */
    [[nodiscard]] LocalFirewallResult allow(
        const LocalFirewallRule &rule) override;

    /**
     * @brief Removes a Host-owned local firewall rule.
     *
     * @param rule Firewall rule to remove.
     *
     * @return Result of the firewall operation.
     */
    [[nodiscard]] LocalFirewallResult deny(
        const LocalFirewallRule &rule) override;

    /**
     * @brief Performs no direct release operation.
     *
     * @param rule Firewall rule.
     */
    void release(
        const LocalFirewallRule &rule) noexcept override
    {
      static_cast<void>(rule);
    }

  private:
    PrivilegedProcessRunner &runner_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PRIVILEGED_LOCAL_FIREWALL_HPP
