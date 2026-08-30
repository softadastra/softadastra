/** @file NativeLocalFirewall.cpp */
#include "platform/NativeLocalFirewall.hpp"

#if defined(__linux__)
#include <algorithm>
#include <array>
#include <cerrno>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace
{
#if defined(__linux__)
  softadastra::UfwCommandResult run_ufw(const std::vector<std::string> &arguments)
  {
    int pipefd[2]{};
    if (::pipe(pipefd) != 0) return {-1, {}};
    const pid_t child = ::fork();
    if (child < 0) { ::close(pipefd[0]); ::close(pipefd[1]); return {-1, {}}; }
    if (child == 0)
    {
      ::dup2(pipefd[1], STDOUT_FILENO); ::dup2(pipefd[1], STDERR_FILENO);
      ::close(pipefd[0]); ::close(pipefd[1]);
      std::vector<char *> values; values.reserve(arguments.size() + 2);
      values.push_back(const_cast<char *>("ufw"));
      for (const auto &argument : arguments) values.push_back(const_cast<char *>(argument.c_str()));
      values.push_back(nullptr); ::execvp("ufw", values.data()); ::_exit(127);
    }
    ::close(pipefd[1]); std::string output; std::array<char, 256> buffer{}; ssize_t count{};
    while ((count = ::read(pipefd[0], buffer.data(), buffer.size())) > 0)
      output.append(buffer.data(), static_cast<std::size_t>(count));
    ::close(pipefd[0]); int status{};
    while (::waitpid(child, &status, 0) < 0 && errno == EINTR) {}
    return {WIFEXITED(status) ? WEXITSTATUS(status) : -1, std::move(output)};
  }

  bool allows_rule(const std::string &output, const softadastra::LocalFirewallRule &rule)
  {
    std::istringstream lines(output); std::string line;
    const auto port = std::to_string(rule.port) + "/tcp";
    while (std::getline(lines, line))
      if (line.find(port) != std::string::npos && line.find("ALLOW") != std::string::npos && line.find(rule.subnet) != std::string::npos)
        return true;
    return false;
  }

  bool has_exact_owner(const std::string &line, const std::string &owner)
  {
    const auto marker = line.rfind("# ");
    return marker != std::string::npos && line.substr(marker + 2) == owner;
  }

  std::vector<int> owned_rule_numbers(const std::string &output, const std::string &owner)
  {
    std::vector<int> numbers; std::istringstream lines(output); std::string line;
    while (std::getline(lines, line))
    {
      const auto left = line.find('['); const auto right = line.find(']');
      if (left == std::string::npos || right == std::string::npos || !has_exact_owner(line, owner)) continue;
      try { numbers.push_back(std::stoi(line.substr(left + 1, right - left - 1))); }
      catch (const std::exception &) {}
    }
    std::sort(numbers.rbegin(), numbers.rend());
    return numbers;
  }
#endif
}

namespace softadastra
{
  NativeLocalFirewall::NativeLocalFirewall(UfwCommandRunner &runner) noexcept : runner_(&runner) {}

  std::vector<std::string> NativeLocalFirewall::allow_arguments(const LocalFirewallRule &rule)
  {
    return {"allow", "from", rule.subnet, "to", "any", "port", std::to_string(rule.port), "proto", "tcp", "comment", rule.owner};
  }

  std::vector<std::string> NativeLocalFirewall::delete_arguments(const int number)
  {
    return {"--force", "delete", std::to_string(number)};
  }

  LocalFirewallResult NativeLocalFirewall::status(const LocalFirewallRule &rule)
  {
#if !defined(__linux__)
    static_cast<void>(rule); return LocalFirewallResult::Unsupported;
#else
    if (rule.subnet.empty() || rule.port == 0) return LocalFirewallResult::Failed;
    const auto result = runner_ ? runner_->run({"status"}) : run_ufw({"status"});
    if (result.exit_code == 127) return LocalFirewallResult::Unsupported;
    if (result.exit_code != 0) return LocalFirewallResult::Failed;
    if (result.output.find("Status: inactive") != std::string::npos) return LocalFirewallResult::Disabled;
    return allows_rule(result.output, rule) ? LocalFirewallResult::Open : LocalFirewallResult::PermissionRequired;
#endif
  }

  LocalFirewallResult NativeLocalFirewall::ensure(const LocalFirewallRule &rule)
  {
    const auto result = status(rule);
    return result == LocalFirewallResult::Disabled ? LocalFirewallResult::Open : result;
  }

  LocalFirewallResult NativeLocalFirewall::allow(const LocalFirewallRule &rule)
  {
#if !defined(__linux__)
    static_cast<void>(rule); return LocalFirewallResult::Unsupported;
#else
    const auto current = status(rule);
    if (current == LocalFirewallResult::Open || current == LocalFirewallResult::Disabled) return current;
    if (current != LocalFirewallResult::PermissionRequired) return current;
    const auto result = runner_ ? runner_->run(allow_arguments(rule)) : run_ufw(allow_arguments(rule));
    return result.exit_code == 0 ? LocalFirewallResult::Open : LocalFirewallResult::Failed;
#endif
  }

  void NativeLocalFirewall::release(const LocalFirewallRule &rule) noexcept
  {
#if defined(__linux__)
    const auto result = runner_ ? runner_->run({"status", "numbered"}) : run_ufw({"status", "numbered"});
    if (result.exit_code != 0) return;
    for (const int number : owned_rule_numbers(result.output, rule.owner))
      static_cast<void>(runner_ ? runner_->run(delete_arguments(number)) : run_ufw(delete_arguments(number)));
#else
    static_cast<void>(rule);
#endif
  }

  LocalFirewallResult NativeLocalFirewall::deny(const LocalFirewallRule &rule)
  {
#if !defined(__linux__)
    static_cast<void>(rule); return LocalFirewallResult::Unsupported;
#else
    const auto result = runner_ ? runner_->run({"status", "numbered"}) : run_ufw({"status", "numbered"});
    if (result.exit_code == 127) return LocalFirewallResult::Unsupported;
    if (result.exit_code != 0) return LocalFirewallResult::Failed;
    for (const int number : owned_rule_numbers(result.output, rule.owner))
    {
      const auto deleted = runner_ ? runner_->run(delete_arguments(number)) : run_ufw(delete_arguments(number));
      if (deleted.exit_code != 0) return LocalFirewallResult::Failed;
    }
    return LocalFirewallResult::Open;
#endif
  }
}
