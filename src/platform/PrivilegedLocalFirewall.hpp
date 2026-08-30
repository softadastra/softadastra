#ifndef SOFTADASTRA_PLATFORM_PRIVILEGED_LOCAL_FIREWALL_HPP
#define SOFTADASTRA_PLATFORM_PRIVILEGED_LOCAL_FIREWALL_HPP
#include "platform/LocalFirewall.hpp"
#include <string>
#include <vector>
namespace softadastra {
struct PrivilegedProcessResult { int exit_code{-1}; std::string output; };
class PrivilegedProcessRunner { public: virtual ~PrivilegedProcessRunner()=default; [[nodiscard]] virtual PrivilegedProcessResult run(const std::string&, const std::vector<std::string>&)=0; };
class NativePrivilegedProcessRunner final : public PrivilegedProcessRunner {
public:
  [[nodiscard]] PrivilegedProcessResult run(const std::string&, const std::vector<std::string>&) override;
};
class PrivilegedLocalFirewall final : public LocalFirewall {
public: explicit PrivilegedLocalFirewall(PrivilegedProcessRunner& runner) noexcept : runner_(runner) {} [[nodiscard]] LocalFirewallResult ensure(const LocalFirewallRule&) override; [[nodiscard]] LocalFirewallResult status(const LocalFirewallRule&) override; [[nodiscard]] LocalFirewallResult allow(const LocalFirewallRule&) override; [[nodiscard]] LocalFirewallResult deny(const LocalFirewallRule&) override; void release(const LocalFirewallRule&) noexcept override {}
private: PrivilegedProcessRunner& runner_;
}; }
#endif
