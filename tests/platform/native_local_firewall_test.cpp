#include "platform/FirewallHelper.hpp"
#include "platform/NativeLocalFirewall.hpp"
#include "platform/PrivilegedLocalFirewall.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace
{
  class Network final : public softadastra::Network
  {
  public:
    [[nodiscard]] bool is_available() const noexcept override { return true; }
    [[nodiscard]] bool is_connected() const noexcept override { return true; }
    [[nodiscard]] softadastra::NetworkCapability network_capability() const override
    {
      return {softadastra::NetworkState::Available, "192.168.1.10", "eth0",
              softadastra::NetworkInterfaceType::Ethernet,
              softadastra::LocalNetworkState::Existing,
              softadastra::ManagedNetworkCapability::Unavailable, subnet};
    }
    std::string subnet{"192.168.1.0/24"};
  };

  class UfwRunner final : public softadastra::UfwCommandRunner
  {
  public:
    [[nodiscard]] softadastra::UfwCommandResult run(const std::vector<std::string> &arguments) override
    {
      calls.push_back(arguments);
      if (next < results.size()) return results[next++];
      return {0, {}};
    }
    std::vector<std::vector<std::string>> calls;
    std::vector<softadastra::UfwCommandResult> results;
    std::size_t next{0};
  };

  class Runner final : public softadastra::PrivilegedProcessRunner
  {
  public:
    softadastra::PrivilegedProcessResult result;
    std::string executable;
    std::vector<std::string> arguments;
    softadastra::PrivilegedProcessResult run(const std::string &value, const std::vector<std::string> &values) override
    { executable = value; arguments = values; return result; }
  };

  softadastra::FirewallHelper helper(Network &network, softadastra::NativeLocalFirewall &firewall)
  { return softadastra::FirewallHelper(network, firewall); }

  TEST(PrivilegedLocalFirewallTest, UsesFixedCommandsAndParsesStructuredStatus)
  {
    Runner runner; softadastra::PrivilegedLocalFirewall firewall(runner);
    const softadastra::LocalFirewallRule rule{"softadastra:abc", "192.168.1.0/24", 8083};
    runner.result = {0, "allowed\n"}; EXPECT_EQ(firewall.status(rule), softadastra::LocalFirewallResult::Open);
    EXPECT_EQ(runner.executable, "pkexec");
    EXPECT_EQ(runner.arguments, (std::vector<std::string>{"/usr/libexec/softadastra-firewall-status", "8083", "192.168.1.0/24", "softadastra:abc"}));
    runner.result = {0, "blocked\n"}; EXPECT_EQ(firewall.status(rule), softadastra::LocalFirewallResult::PermissionRequired);
    runner.result = {0, "disabled\n"}; EXPECT_EQ(firewall.status(rule), softadastra::LocalFirewallResult::Open);
    runner.result = {0, "unsupported\n"}; EXPECT_EQ(firewall.status(rule), softadastra::LocalFirewallResult::Unsupported);
    runner.result = {0, "error\n"}; EXPECT_EQ(firewall.status(rule), softadastra::LocalFirewallResult::Failed);
    runner.result = {0, "ufw status\n"}; EXPECT_EQ(firewall.status(rule), softadastra::LocalFirewallResult::Failed);
    runner.result = {127, {}}; EXPECT_EQ(firewall.status(rule), softadastra::LocalFirewallResult::Failed);
  }

  TEST(FirewallHelperTest, RejectsInvalidStatusAndModifyInterfaces)
  {
    Network network; UfwRunner runner; softadastra::NativeLocalFirewall firewall(runner); auto value = helper(network, firewall); std::ostringstream output;
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Status, {"80", network.subnet}, output), 2);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Modify, {"allow-local-tcp", "80", network.subnet, "softadastra:a", "extra"}, output), 2);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Modify, {"shell;command", "80", network.subnet, "softadastra:a"}, output), 2);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Status, {"0", network.subnet, "softadastra:a"}, output), 2);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Status, {"65536", network.subnet, "softadastra:a"}, output), 2);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Status, {"80", "192.168.1.0/99", "softadastra:a"}, output), 2);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Status, {"80", "10.0.0.0/24", "softadastra:a"}, output), 2);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Status, {"80", network.subnet, "../tag"}, output), 2);
    EXPECT_TRUE(runner.calls.empty());
  }

  TEST(FirewallHelperTest, StatusIsReadOnlyAndOnlyPrintsStructuredState)
  {
    Network network; UfwRunner runner; runner.results = {{0, "Status: active\n8080/tcp ALLOW 192.168.1.0/24\n"}};
    softadastra::NativeLocalFirewall firewall(runner); auto value = helper(network, firewall); std::ostringstream output;
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Status, {"8080", network.subnet, "softadastra:abc"}, output), 0);
    EXPECT_EQ(output.str(), "allowed\n");
    ASSERT_EQ(runner.calls.size(), 1U); EXPECT_EQ(runner.calls[0], (std::vector<std::string>{"status"}));
  }

  TEST(FirewallHelperTest, AllowPreservesExistingUserAndOwnedRules)
  {
    Network network; UfwRunner runner;
    runner.results = {{0, "Status: active\n8080/tcp ALLOW 192.168.1.0/24\n"}, {0, "Status: active\n8080/tcp ALLOW 192.168.1.0/24 # softadastra:abc\n"}};
    softadastra::NativeLocalFirewall firewall(runner); auto value = helper(network, firewall); std::ostringstream output;
    const std::vector<std::string> args{"allow-local-tcp", "8080", network.subnet, "softadastra:abc"};
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Modify, args, output), 0);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Modify, args, output), 0);
    ASSERT_EQ(runner.calls.size(), 2U); EXPECT_EQ(runner.calls[0], (std::vector<std::string>{"status"}));
    EXPECT_EQ(runner.calls[1], (std::vector<std::string>{"status"}));
  }

  TEST(FirewallHelperTest, AllowUsesExactUfwArgumentsWithoutForce)
  {
    Network network; UfwRunner runner; runner.results = {{0, "Status: active\n"}, {0, {}}};
    softadastra::NativeLocalFirewall firewall(runner); auto value = helper(network, firewall); std::ostringstream output;
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Modify, {"allow-local-tcp", "8080", network.subnet, "softadastra:abc"}, output), 0);
    ASSERT_EQ(runner.calls.size(), 2U);
    EXPECT_EQ(runner.calls[1], (std::vector<std::string>{"allow", "from", "192.168.1.0/24", "to", "any", "port", "8080", "proto", "tcp", "comment", "softadastra:abc"}));
  }

  TEST(FirewallHelperTest, DenyDeletesOnlyExactTagInDescendingOrderAndIsIdempotent)
  {
    Network network; UfwRunner runner;
    runner.results = {{0, "[ 3] 8080/tcp ALLOW 192.168.1.0/24 # softadastra:abc\n[ 7] 8081/tcp ALLOW 192.168.1.0/24 # softadastra:abc\n[ 9] 8082/tcp ALLOW 192.168.1.0/24 # softadastra:abcd\n[10] 8083/tcp ALLOW 192.168.1.0/24\n"}, {0, {}}, {0, {}}, {0, "Status: active\n"}};
    softadastra::NativeLocalFirewall firewall(runner); auto value = helper(network, firewall); std::ostringstream output;
    const std::vector<std::string> args{"deny-owned-local-tcp", "8080", network.subnet, "softadastra:abc"};
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Modify, args, output), 0);
    EXPECT_EQ(value.run(softadastra::FirewallHelperCommand::Modify, args, output), 0);
    ASSERT_EQ(runner.calls.size(), 4U);
    EXPECT_EQ(runner.calls[1], (std::vector<std::string>{"--force", "delete", "7"}));
    EXPECT_EQ(runner.calls[2], (std::vector<std::string>{"--force", "delete", "3"}));
    EXPECT_EQ(runner.calls[3], (std::vector<std::string>{"status", "numbered"}));
  }
}
