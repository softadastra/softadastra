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

#if defined(__linux__)

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

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
  }

  bool HostInstanceLock::acquire(const std::filesystem::path &directory) noexcept
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
    static_cast<void>(directory);
    return true;
#endif
  }

  bool HostInstanceLock::is_held(const std::filesystem::path &directory) noexcept
  {
    HostInstanceLock probe;
    return !probe.acquire(directory);
  }

} // namespace softadastra
