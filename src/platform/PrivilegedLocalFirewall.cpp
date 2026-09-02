/**
 *
 *  @file PrivilegedLocalFirewall.cpp
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

#include "platform/PrivilegedLocalFirewall.hpp"

#include "platform/InstallPaths.hpp"

#include <utility>

#if defined(__linux__)

#include <array>
#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

namespace softadastra
{
  PrivilegedProcessResult NativePrivilegedProcessRunner::run(
      const std::string &program,
      const std::vector<std::string> &arguments)
  {
#if !defined(__linux__)

    static_cast<void>(program);
    static_cast<void>(arguments);

    return {127, {}};

#else

    int pipefd[2]{};

    if (::pipe(pipefd) != 0)
    {
      return {-1, {}};
    }

    const pid_t child =
        ::fork();

    if (child < 0)
    {
      ::close(pipefd[0]);
      ::close(pipefd[1]);

      return {-1, {}};
    }

    if (child == 0)
    {
      ::dup2(
          pipefd[1],
          STDOUT_FILENO);

      ::dup2(
          pipefd[1],
          STDERR_FILENO);

      ::close(pipefd[0]);
      ::close(pipefd[1]);

      std::vector<char *> values;

      values.reserve(
          arguments.size() + 2);

      values.push_back(
          const_cast<char *>(
              program.c_str()));

      for (const auto &argument : arguments)
      {
        values.push_back(
            const_cast<char *>(
                argument.c_str()));
      }

      values.push_back(
          nullptr);

      ::execvp(
          program.c_str(),
          values.data());

      ::_exit(127);
    }

    ::close(pipefd[1]);

    std::string output;
    std::array<char, 256> buffer{};

    ssize_t count{};

    while ((count = ::read(
                pipefd[0],
                buffer.data(),
                buffer.size())) > 0)
    {
      output.append(
          buffer.data(),
          static_cast<std::size_t>(count));
    }

    ::close(pipefd[0]);

    int status{};

    while (::waitpid(
               child,
               &status,
               0) < 0 &&
           errno == EINTR)
    {
    }

    return {
        WIFEXITED(status)
            ? WEXITSTATUS(status)
            : -1,
        std::move(output)};

#endif
  }

  LocalFirewallResult PrivilegedLocalFirewall::ensure(
      const LocalFirewallRule &rule)
  {
    return status(rule);
  }

#if defined(__linux__)

  LocalFirewallResult PrivilegedLocalFirewall::status(
      const LocalFirewallRule &rule)
  {
    auto result =
        runner_.run(
            "pkexec",
            {install_paths::firewall_status_helper,
             std::to_string(rule.port),
             rule.subnet,
             rule.owner});

    if (result.output == "allowed\n" ||
        result.output == "disabled\n")
    {
      return LocalFirewallResult::Open;
    }

    if (result.output == "unsupported\n")
    {
      return LocalFirewallResult::Unsupported;
    }

    return result.output == "blocked\n"
               ? LocalFirewallResult::PermissionRequired
               : LocalFirewallResult::Failed;
  }

  LocalFirewallResult PrivilegedLocalFirewall::allow(
      const LocalFirewallRule &rule)
  {
    auto result =
        runner_.run(
            "pkexec",
            {install_paths::firewall_modify_helper,
             "allow-local-tcp",
             std::to_string(rule.port),
             rule.subnet,
             rule.owner});

    return result.exit_code == 0
               ? LocalFirewallResult::Open
           : result.exit_code == 126
               ? LocalFirewallResult::PermissionRequired
               : LocalFirewallResult::Failed;
  }

  LocalFirewallResult PrivilegedLocalFirewall::deny(
      const LocalFirewallRule &rule)
  {
    auto result =
        runner_.run(
            "pkexec",
            {install_paths::firewall_modify_helper,
             "deny-owned-local-tcp",
             std::to_string(rule.port),
             rule.subnet,
             rule.owner});

    return result.exit_code == 0
               ? LocalFirewallResult::Open
           : result.exit_code == 126
               ? LocalFirewallResult::PermissionRequired
               : LocalFirewallResult::Failed;
  }

#else

  LocalFirewallResult PrivilegedLocalFirewall::status(
      const LocalFirewallRule &rule)
  {
    static_cast<void>(rule);

    return LocalFirewallResult::Unsupported;
  }

  LocalFirewallResult PrivilegedLocalFirewall::allow(
      const LocalFirewallRule &rule)
  {
    static_cast<void>(rule);

    return LocalFirewallResult::Unsupported;
  }

  LocalFirewallResult PrivilegedLocalFirewall::deny(
      const LocalFirewallRule &rule)
  {
    static_cast<void>(rule);

    return LocalFirewallResult::Unsupported;
  }

#endif

} // namespace softadastra
