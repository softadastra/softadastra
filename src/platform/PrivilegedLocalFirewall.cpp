#include "platform/PrivilegedLocalFirewall.hpp"
#if defined(__linux__)
#include <array>
#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
namespace softadastra {
PrivilegedProcessResult NativePrivilegedProcessRunner::run(const std::string &program, const std::vector<std::string> &arguments) {
#if !defined(__linux__)
  static_cast<void>(program); static_cast<void>(arguments); return {127, {}};
#else
  int pipefd[2]{};
  if (::pipe(pipefd) != 0) return {-1, {}};
  const pid_t child = ::fork();
  if (child < 0) { ::close(pipefd[0]); ::close(pipefd[1]); return {-1, {}}; }
  if (child == 0) {
    ::dup2(pipefd[1], STDOUT_FILENO); ::dup2(pipefd[1], STDERR_FILENO);
    ::close(pipefd[0]); ::close(pipefd[1]);
    std::vector<char *> values; values.reserve(arguments.size() + 2);
    values.push_back(const_cast<char *>(program.c_str()));
    for (const auto &argument : arguments) values.push_back(const_cast<char *>(argument.c_str()));
    values.push_back(nullptr); ::execvp(program.c_str(), values.data()); ::_exit(127);
  }
  ::close(pipefd[1]); std::string output; std::array<char, 256> buffer{}; ssize_t count{};
  while ((count = ::read(pipefd[0], buffer.data(), buffer.size())) > 0) output.append(buffer.data(), static_cast<std::size_t>(count));
  ::close(pipefd[0]); int status{}; while (::waitpid(child, &status, 0) < 0 && errno == EINTR) {}
  return {WIFEXITED(status) ? WEXITSTATUS(status) : -1, std::move(output)};
#endif
}
LocalFirewallResult PrivilegedLocalFirewall::ensure(const LocalFirewallRule&r) {
return status(r);
}
#if defined(__linux__)
LocalFirewallResult PrivilegedLocalFirewall::status(const LocalFirewallRule&r) {
auto x=runner_.run("pkexec",{"/usr/libexec/softadastra-firewall-status",std::to_string(r.port),r.subnet,r.owner}); if(x.exit_code==127)return LocalFirewallResult::Unsupported; if(x.output=="allowed\n"||x.output=="disabled\n")return LocalFirewallResult::Open; if(x.output=="unsupported\n")return LocalFirewallResult::Unsupported; return x.output=="blocked\n"?LocalFirewallResult::PermissionRequired:LocalFirewallResult::Failed;
}
LocalFirewallResult PrivilegedLocalFirewall::allow(const LocalFirewallRule&r) {
auto x=runner_.run("pkexec",{"/usr/libexec/softadastra-firewall-modify","allow-local-tcp",std::to_string(r.port),r.subnet,r.owner});return x.exit_code==0?LocalFirewallResult::Open:x.exit_code==126?LocalFirewallResult::PermissionRequired:LocalFirewallResult::Failed;
}
LocalFirewallResult PrivilegedLocalFirewall::deny(const LocalFirewallRule&r) {
auto x=runner_.run("pkexec",{"/usr/libexec/softadastra-firewall-modify","deny-owned-local-tcp",std::to_string(r.port),r.subnet,r.owner});return x.exit_code==0?LocalFirewallResult::Open:x.exit_code==126?LocalFirewallResult::PermissionRequired:LocalFirewallResult::Failed;
}
#else
LocalFirewallResult PrivilegedLocalFirewall::status(const LocalFirewallRule&) {
return LocalFirewallResult::Unsupported;
}
LocalFirewallResult PrivilegedLocalFirewall::allow(const LocalFirewallRule&) {
return LocalFirewallResult::Unsupported;
}
LocalFirewallResult PrivilegedLocalFirewall::deny(const LocalFirewallRule&) {
return LocalFirewallResult::Unsupported;
}
#endif
}
