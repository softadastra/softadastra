/**
 *
 *  @file NativeProcess.cpp
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

#include "platform/NativeProcess.hpp"

#include <limits>
#include <utility>

#include <vix/process/Status.hpp>
#include <vix/process/Terminate.hpp>
#include <vix/process/Wait.hpp>

#if defined(__linux__)

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

#if defined(_WIN32)

#include <windows.h>

#endif

namespace softadastra
{
  NativeProcess::NativeProcess(
      vix::process::Child child) noexcept
      : child_(std::move(child))
  {
  }

  NativeProcess::~NativeProcess()
  {
#if defined(_WIN32)

    if (process_ != nullptr)
    {
      ::CloseHandle(process_);
    }

    if (job_ != nullptr)
    {
      ::CloseHandle(job_);
    }

#endif
  }

#if defined(__linux__)

  NativeProcess::NativeProcess(pid_t pid) noexcept
      : native_pid_(pid)
  {
  }

  std::optional<pid_t> NativeProcess::native_pid() const noexcept
  {
    if (native_pid_ > 0)
    {
      return native_pid_;
    }

    if (!child_.valid())
    {
      return std::nullopt;
    }

    const auto identifier = child_.id();

    if (identifier >
        static_cast<Id>(
            std::numeric_limits<pid_t>::max()))
    {
      return std::nullopt;
    }

    return static_cast<pid_t>(identifier);
  }

#endif

#if defined(_WIN32)

  NativeProcess::NativeProcess(
      HANDLE process,
      HANDLE job,
      DWORD pid) noexcept
      : native_pid_(pid),
        process_(process),
        job_(job)
  {
  }

#endif

  bool NativeProcess::stop()
  {
#if defined(__linux__)

    if (native_pid_ > 0)
    {
      ::kill(-native_pid_, SIGTERM);

      int status = 0;

      if (::waitpid(native_pid_, &status, 0) < 0)
      {
        return false;
      }

      exit_code_ =
          WIFEXITED(status)
              ? WEXITSTATUS(status)
              : 1;

      return true;
    }

#endif

#if defined(_WIN32)

    if (process_ != nullptr)
    {
      if (!is_running())
      {
        return exit_code().has_value();
      }

      if (job_ != nullptr)
      {
        static_cast<void>(
            ::TerminateJobObject(job_, 1));
      }
      else
      {
        static_cast<void>(
            ::TerminateProcess(process_, 1));
      }

      static_cast<void>(
          ::WaitForSingleObject(
              process_,
              INFINITE));

      DWORD code = 1;

      if (::GetExitCodeProcess(process_, &code))
      {
        exit_code_ = static_cast<int>(code);
      }

      return exit_code_.has_value();
    }

#endif

    if (!child_.valid())
    {
      return false;
    }

    if (!is_running())
    {
      return exit_code().has_value();
    }

    const auto error =
        vix::process::terminate(child_);

    if (error.has_error())
    {
      return false;
    }

    const auto result =
        vix::process::wait(child_);

    if (!result)
    {
      return false;
    }

    exit_code_ = result.value();

    return true;
  }

  bool NativeProcess::is_running() const noexcept
  {
#if defined(__linux__)

    if (native_pid_ > 0)
    {
      int status = 0;

      const auto result =
          ::waitpid(
              native_pid_,
              &status,
              WNOHANG);

      if (result == 0)
      {
        return true;
      }

      if (result == native_pid_)
      {
        const_cast<NativeProcess *>(this)->exit_code_ =
            WIFEXITED(status)
                ? WEXITSTATUS(status)
                : 1;
      }

      return false;
    }

#endif

#if defined(_WIN32)

    if (process_ != nullptr)
    {
      DWORD code = STILL_ACTIVE;

      if (!::GetExitCodeProcess(process_, &code))
      {
        return false;
      }

      if (code == STILL_ACTIVE)
      {
        return true;
      }

      const_cast<NativeProcess *>(this)->exit_code_ =
          static_cast<int>(code);

      return false;
    }

#endif

    if (!child_.valid())
    {
      return false;
    }

    try
    {
      const auto result =
          vix::process::status(child_);

      if (!result)
      {
        return false;
      }

      return result.value();
    }
    catch (...)
    {
      return false;
    }
  }

  std::optional<int> NativeProcess::exit_code() noexcept
  {
#if defined(__linux__)

    if (native_pid_ > 0)
    {
      int status = 0;

      const auto result =
          ::waitpid(
              native_pid_,
              &status,
              WNOHANG);

      if (result == native_pid_)
      {
        exit_code_ =
            WIFEXITED(status)
                ? WEXITSTATUS(status)
                : 1;
      }

      return exit_code_;
    }

#endif

#if defined(_WIN32)

    if (process_ != nullptr)
    {
      DWORD code = STILL_ACTIVE;

      if (::GetExitCodeProcess(process_, &code) &&
          code != STILL_ACTIVE)
      {
        exit_code_ = static_cast<int>(code);
      }

      return exit_code_;
    }

#endif

    if (exit_code_.has_value())
    {
      return exit_code_;
    }

    if (!child_.valid())
    {
      return std::nullopt;
    }

    try
    {
      const auto status =
          vix::process::status(child_);

      if (!status)
      {
        return std::nullopt;
      }

      if (status.value())
      {
        return std::nullopt;
      }

      const auto result =
          vix::process::wait(child_);

      if (!result)
      {
        return std::nullopt;
      }

      exit_code_ = result.value();

      return exit_code_;
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  NativeProcess::Id NativeProcess::id() const noexcept
  {
    return child_.id();
  }

  std::optional<long> NativeProcess::pid() const noexcept
  {
#if defined(__linux__)

    if (native_pid_ > 0)
    {
      return native_pid_;
    }

#endif

#if defined(_WIN32)

    if (process_ != nullptr &&
        native_pid_ <=
            static_cast<DWORD>(
                std::numeric_limits<long>::max()))
    {
      return static_cast<long>(native_pid_);
    }

#endif

    return child_.valid()
               ? std::optional<long>(
                     static_cast<long>(child_.id()))
               : std::nullopt;
  }

} // namespace softadastra
