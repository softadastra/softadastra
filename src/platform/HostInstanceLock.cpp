/**
 *
 *  @file HostInstanceLock.cpp
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

#include "platform/HostInstanceLock.hpp"

#include <functional>

#if defined(__linux__)

#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#endif

#if defined(_WIN32)

#include <windows.h>

#endif

namespace softadastra
{
  HostInstanceLock::~HostInstanceLock()
  {
#if defined(__linux__)

    if (descriptor_ >= 0)
    {
      static_cast<void>(::close(descriptor_));
    }

#endif

#if defined(_WIN32)

    if (mutex_ != nullptr)
    {
      ::CloseHandle(mutex_);
    }

#endif
  }

  bool HostInstanceLock::acquire(
      const std::filesystem::path &directory) noexcept
  {
#if defined(__linux__)

    if (descriptor_ >= 0)
    {
      return true;
    }

    const int descriptor = ::open(
        (directory / "host.lock").c_str(),
        O_CREAT | O_RDWR | O_CLOEXEC,
        0600);

    if (descriptor < 0)
    {
      return false;
    }

    if (::flock(descriptor, LOCK_EX | LOCK_NB) != 0)
    {
      static_cast<void>(::close(descriptor));
      return false;
    }

    descriptor_ = descriptor;

    return true;

#else

#if defined(_WIN32)

    if (mutex_ != nullptr)
    {
      return true;
    }

    const std::wstring name =
        L"Local\\SoftadastraHost-" +
        std::to_wstring(
            std::hash<std::wstring>{}(directory.wstring()));

    mutex_ = ::CreateMutexW(
        nullptr,
        TRUE,
        name.c_str());

    if (mutex_ == nullptr)
    {
      return false;
    }

    if (::GetLastError() == ERROR_ALREADY_EXISTS)
    {
      ::CloseHandle(mutex_);
      mutex_ = nullptr;

      return false;
    }

    return true;

#else

    static_cast<void>(directory);

    return false;

#endif
#endif
  }

  bool HostInstanceLock::is_held(
      const std::filesystem::path &directory) noexcept
  {
    return probe(directory) == HostInstanceLockState::Held;
  }

  HostInstanceLockState HostInstanceLock::probe(
      const std::filesystem::path &directory) noexcept
  {
#if defined(__linux__)
    const int descriptor = ::open(
        (directory / "host.lock").c_str(),
        O_CREAT | O_RDWR | O_CLOEXEC,
        0600);

    if (descriptor < 0)
    {
      return errno == ENOENT
                 ? HostInstanceLockState::Free
                 : HostInstanceLockState::Error;
    }

    if (::flock(descriptor, LOCK_EX | LOCK_NB) == 0)
    {
      static_cast<void>(::close(descriptor));
      return HostInstanceLockState::Free;
    }

    const int error = errno;
    static_cast<void>(::close(descriptor));

    return error == EWOULDBLOCK || error == EAGAIN
               ? HostInstanceLockState::Held
               : HostInstanceLockState::Error;

#elif defined(_WIN32)
    const std::wstring name =
        L"Local\\SoftadastraHost-" +
        std::to_wstring(
            std::hash<std::wstring>{}(directory.wstring()));

    HANDLE mutex = ::CreateMutexW(nullptr, TRUE, name.c_str());

    if (mutex == nullptr)
    {
      return HostInstanceLockState::Error;
    }

    const DWORD error = ::GetLastError();
    ::CloseHandle(mutex);

    return error == ERROR_ALREADY_EXISTS
               ? HostInstanceLockState::Held
               : HostInstanceLockState::Free;

#else
    static_cast<void>(directory);
    return HostInstanceLockState::Error;
#endif
  }

} // namespace softadastra
