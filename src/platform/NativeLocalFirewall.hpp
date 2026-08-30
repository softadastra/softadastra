/** @file NativeLocalFirewall.hpp */
#ifndef SOFTADASTRA_PLATFORM_NATIVE_LOCAL_FIREWALL_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_LOCAL_FIREWALL_HPP

#include "platform/LocalFirewall.hpp"

#include <string>
#include <vector>

namespace softadastra
{
  struct UfwCommandResult
  {
    int exit_code{-1};
    std::string output;
  };

  class UfwCommandRunner
  {
  public:
    virtual ~UfwCommandRunner() = default;
    [[nodiscard]] virtual UfwCommandResult run(
        const std::vector<std::string> &arguments) = 0;
  };

  /** Linux UFW implementation; unsupported platforms report Unsupported. */
  class NativeLocalFirewall final : public LocalFirewall
  {
  public:
    NativeLocalFirewall() = default;
    explicit NativeLocalFirewall(UfwCommandRunner &runner) noexcept;
    /** Returns the exact UFW arguments for a local, owned allow rule. */
    [[nodiscard]] static std::vector<std::string> allow_arguments(
        const LocalFirewallRule &rule);

    /** Returns the documented non-interactive UFW deletion by rule number. */
    [[nodiscard]] static std::vector<std::string> delete_arguments(
        int number);

    [[nodiscard]] LocalFirewallResult ensure(
        const LocalFirewallRule &rule) override;
    [[nodiscard]] LocalFirewallResult status(
        const LocalFirewallRule &rule) override;
    [[nodiscard]] LocalFirewallResult allow(
        const LocalFirewallRule &rule) override;
    void release(const LocalFirewallRule &rule) noexcept override;
    [[nodiscard]] LocalFirewallResult deny(
        const LocalFirewallRule &rule) override;

  private:
    UfwCommandRunner *runner_{nullptr};
  };
}
#endif
