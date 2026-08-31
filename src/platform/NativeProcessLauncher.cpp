/**
 *
 *  @file NativeProcessLauncher.cpp
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

#include "platform/NativeProcessLauncher.hpp"

#include "platform/Environment.hpp"
#include "platform/NativeProcess.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <vix/error/ErrorCode.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/Spawn.hpp>

#if defined(__linux__)

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

#if defined(_WIN32)

#include <windows.h>

#endif

namespace softadastra
{
  namespace
  {
#if defined(_WIN32)

    std::wstring wide(const std::string &value)
    {
      if (value.empty())
      {
        return {};
      }

      const int size = ::MultiByteToWideChar(
          CP_UTF8,
          0,
          value.data(),
          static_cast<int>(value.size()),
          nullptr,
          0);

      std::wstring result(
          size,
          L'\0');

      ::MultiByteToWideChar(
          CP_UTF8,
          0,
          value.data(),
          static_cast<int>(value.size()),
          result.data(),
          size);

      return result;
    }

    std::wstring quote(const std::string &value)
    {
      const std::wstring argument = wide(value);

      if (!argument.empty() &&
          argument.find_first_of(L" \t\"") == std::wstring::npos)
      {
        return argument;
      }

      std::wstring result(L"\"");
      std::size_t backslashes = 0;

      for (const wchar_t character : argument)
      {
        if (character == L'\\')
        {
          ++backslashes;
          continue;
        }

        if (character == L'\"')
        {
          result.append(backslashes * 2 + 1, L'\\');
          result += character;
          backslashes = 0;
          continue;
        }

        result.append(backslashes, L'\\');
        backslashes = 0;
        result += character;
      }

      result.append(backslashes * 2, L'\\');
      result += L'\"';

      return result;
    }

#endif

    bool executable_exists(const std::string &executable)
    {
      const std::filesystem::path path(executable);

      if (path.has_parent_path())
      {
        return std::filesystem::exists(path);
      }

      const auto environment_path =
          environment_value("PATH");

      if (!environment_path)
      {
        return false;
      }

#if defined(_WIN32)

      constexpr char separator = ';';

#else

      constexpr char separator = ':';

#endif

      std::string_view directories(*environment_path);

      while (!directories.empty())
      {
        const auto position =
            directories.find(separator);

        const auto directory =
            directories.substr(0, position);

        if (std::filesystem::exists(
                std::filesystem::path(directory) / path))
        {
          return true;
        }

        if (position == std::string_view::npos)
        {
          break;
        }

        directories.remove_prefix(position + 1);
      }

      return false;
    }

  } // namespace

  ProcessLaunchResult NativeProcessLauncher::launch(
      const ProcessSpec &spec)
  {
    if (spec.executable().empty())
    {
      return ProcessLaunchError::LaunchFailed;
    }

    if (!executable_exists(spec.executable()))
    {
      return ProcessLaunchError::ExecutableNotFound;
    }

#if defined(__linux__)

    if (spec.output_file().has_value())
    {
      const pid_t pid = ::fork();

      if (pid < 0)
      {
        return ProcessLaunchError::LaunchFailed;
      }

      if (pid == 0)
      {
        ::setsid();

        // The Host blocks its own termination signals so its main loop can
        // handle them synchronously. Managed software must not inherit that
        // mask: NativeProcess::stop() terminates its process group with
        // SIGTERM.
        sigset_t signals;
        sigemptyset(&signals);
        sigaddset(&signals, SIGINT);
        sigaddset(&signals, SIGTERM);

        static_cast<void>(
            pthread_sigmask(
                SIG_UNBLOCK,
                &signals,
                nullptr));

        const int log = ::open(
            spec.output_file()->c_str(),
            O_WRONLY | O_CREAT | O_APPEND,
            0644);

        if (log < 0)
        {
          _exit(126);
        }

        ::dup2(log, STDOUT_FILENO);
        ::dup2(log, STDERR_FILENO);

        if (log > STDERR_FILENO)
        {
          ::close(log);
        }

        if (spec.working_directory().has_value() &&
            ::chdir(spec.working_directory()->c_str()) != 0)
        {
          ::dprintf(
              STDERR_FILENO,
              "softadastra: cannot use working directory: %s\n",
              std::strerror(errno));

          _exit(126);
        }

        std::vector<char *> args;

        args.push_back(
            const_cast<char *>(spec.executable().c_str()));

        for (const auto &argument : spec.arguments())
        {
          args.push_back(
              const_cast<char *>(argument.c_str()));
        }

        args.push_back(nullptr);

        ::execvp(args[0], args.data());

        ::dprintf(
            STDERR_FILENO,
            "softadastra: cannot start command: %s\n",
            std::strerror(errno));

        _exit(127);
      }

      return std::make_unique<NativeProcess>(pid);
    }

#endif

#if defined(_WIN32)

    std::wstring command =
        quote(spec.executable());

    if (spec.executable() == "cmd.exe" &&
        spec.arguments().size() == 2 &&
        spec.arguments()[0] == "/C")
    {
      // cmd.exe parses its command tail itself, rather than using the usual
      // Windows argv rules. In particular, it does not treat \" as an
      // escaped quote. /S removes only these outer quotes and preserves the
      // quoted executable and arguments inside the command.
      command +=
          L" /D /S /C \"" +
          wide(spec.arguments()[1]) +
          L"\"";
    }
    else
    {
      for (const auto &argument : spec.arguments())
      {
        command += L" " + quote(argument);
      }
    }

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);

    PROCESS_INFORMATION information{};

    HANDLE log = nullptr;

    if (spec.output_file())
    {
      log = ::CreateFileW(
          wide(*spec.output_file()).c_str(),
          FILE_APPEND_DATA,
          FILE_SHARE_READ | FILE_SHARE_WRITE,
          nullptr,
          OPEN_ALWAYS,
          FILE_ATTRIBUTE_NORMAL,
          nullptr);

      if (log == INVALID_HANDLE_VALUE)
      {
        return ProcessLaunchError::LaunchFailed;
      }

      startup.dwFlags |= STARTF_USESTDHANDLES;
      startup.hStdOutput = log;
      startup.hStdError = log;
      startup.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
    }

    std::wstring cwd =
        spec.working_directory()
            ? wide(*spec.working_directory())
            : std::wstring{};

    if (!::CreateProcessW(
            nullptr,
            command.data(),
            nullptr,
            nullptr,
            log != nullptr,
            CREATE_NEW_PROCESS_GROUP,
            nullptr,
            cwd.empty() ? nullptr : cwd.c_str(),
            &startup,
            &information))
    {
      if (log)
      {
        ::CloseHandle(log);
      }

      const auto error = ::GetLastError();

      return error == ERROR_FILE_NOT_FOUND
                 ? ProcessLaunchError::ExecutableNotFound
             : error == ERROR_ACCESS_DENIED
                 ? ProcessLaunchError::PermissionDenied
                 : ProcessLaunchError::LaunchFailed;
    }

    if (log)
    {
      ::CloseHandle(log);
    }

    ::CloseHandle(information.hThread);

    HANDLE job =
        ::CreateJobObjectW(
            nullptr,
            nullptr);

    if (job != nullptr)
    {
      JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};

      limits.BasicLimitInformation.LimitFlags =
          JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

      static_cast<void>(
          ::SetInformationJobObject(
              job,
              JobObjectExtendedLimitInformation,
              &limits,
              sizeof(limits)));

      if (!::AssignProcessToJobObject(
              job,
              information.hProcess))
      {
        ::CloseHandle(job);
        job = nullptr;
      }
    }

    return std::make_unique<NativeProcess>(
        information.hProcess,
        job,
        information.dwProcessId);

#else

    vix::process::Command command(spec.executable());

    command.args(spec.arguments());

    if (spec.working_directory().has_value())
    {
      command.cwd(
          spec.working_directory().value());
    }

    command.search_in_path(true);
    command.detach(true);

#if defined(__linux__)

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);

    sigset_t previous_signals;

    const bool reset_signals =
        pthread_sigmask(
            SIG_UNBLOCK,
            &signals,
            &previous_signals) == 0;

#endif

    auto result =
        vix::process::spawn(
            std::move(command));

#if defined(__linux__)

    if (reset_signals)
    {
      pthread_sigmask(
          SIG_SETMASK,
          &previous_signals,
          nullptr);
    }

#endif

    if (!result)
    {
      switch (result.error().code())
      {
      case vix::error::ErrorCode::NotFound:
        return ProcessLaunchError::ExecutableNotFound;

      case vix::error::ErrorCode::PermissionDenied:
        return ProcessLaunchError::PermissionDenied;

      default:
        return ProcessLaunchError::LaunchFailed;
      }
    }

    vix::process::Child child =
        std::move(result.value());

    if (!child.valid())
    {
      return ProcessLaunchError::LaunchFailed;
    }

    return std::make_unique<NativeProcess>(
        std::move(child));

#endif
  }

} // namespace softadastra
