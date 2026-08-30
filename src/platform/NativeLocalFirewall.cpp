/** @file NativeLocalFirewall.cpp */
#include "platform/NativeLocalFirewall.hpp"

#if defined(__linux__)
#include <array>
#include <cerrno>
#include <string_view>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#endif

namespace
{
#if defined(__linux__)
  struct CommandResult { int exit_code; std::string output; };

  CommandResult run_ufw(const std::vector<std::string> &arguments)
  {
    int pipefd[2]{};
    if (::pipe(pipefd) != 0)
      return {-1, {}};

    const pid_t child = ::fork();
    if (child < 0)
    {
      ::close(pipefd[0]);
      ::close(pipefd[1]);
      return {-1, {}};
    }
    if (child == 0)
    {
      ::dup2(pipefd[1], STDOUT_FILENO);
      ::dup2(pipefd[1], STDERR_FILENO);
      ::close(pipefd[0]);
      ::close(pipefd[1]);
      std::vector<char *> values;
      values.reserve(arguments.size() + 2);
      values.push_back(const_cast<char *>("ufw"));
      for (const auto &argument : arguments)
        values.push_back(const_cast<char *>(argument.c_str()));
      values.push_back(nullptr);
      ::execvp("ufw", values.data());
      ::_exit(127);
    }
    ::close(pipefd[1]);
    std::string output;
    std::array<char, 256> buffer{};
    ssize_t count = 0;
    while ((count = ::read(pipefd[0], buffer.data(), buffer.size())) > 0)
      output.append(buffer.data(), static_cast<std::size_t>(count));
    ::close(pipefd[0]);
    int status = 0;
    while (::waitpid(child, &status, 0) < 0 && errno == EINTR) {}
    return {WIFEXITED(status) ? WEXITSTATUS(status) : -1, std::move(output)};
  }

  bool matches_rule(const std::string &status,
                    const softadastra::LocalFirewallRule &rule)
  {
    return status.find(std::to_string(rule.port) + "/tcp") != std::string::npos &&
           status.find(rule.subnet) != std::string::npos;
  }
#endif
}

namespace softadastra
{
  LocalFirewallResult NativeLocalFirewall::ensure(const LocalFirewallRule &rule)
  {
#if !defined(__linux__)
    static_cast<void>(rule);
    return LocalFirewallResult::Unsupported;
#else
    if (rule.subnet.empty() || rule.port == 0)
      return LocalFirewallResult::Failed;
    const auto status = run_ufw({"status"});
    if (status.exit_code == 127)
      return LocalFirewallResult::Unsupported;
    if (status.exit_code != 0)
      return ::geteuid() == 0 ? LocalFirewallResult::Failed
                              : LocalFirewallResult::PermissionRequired;
    if (status.output.find("Status: inactive") != std::string::npos ||
        matches_rule(status.output, rule))
      return LocalFirewallResult::Open;

    return LocalFirewallResult::PermissionRequired;
#endif
  }

  void NativeLocalFirewall::release(const LocalFirewallRule &rule) noexcept
  {
#if defined(__linux__)
    static_cast<void>(rule);
#else
    static_cast<void>(rule);
#endif
  }
}
