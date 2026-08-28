/**
 *
 *  @file NativeSocket.hpp
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

#ifndef SOFTADASTRA_HOST_NATIVE_SOCKET_HPP
#define SOFTADASTRA_HOST_NATIVE_SOCKET_HPP

#if defined(_WIN32)

#include <winsock2.h>

#endif

namespace softadastra
{
#if defined(_WIN32)

  /**
   * @brief Native socket type used by the Host on Windows.
   */
  using NativeSocket = SOCKET;

  /**
   * @brief Invalid native socket value on Windows.
   */
  constexpr NativeSocket InvalidSocket = INVALID_SOCKET;

#else

  /**
   * @brief Native socket type used by the Host on POSIX platforms.
   */
  using NativeSocket = int;

  /**
   * @brief Invalid native socket value on POSIX platforms.
   */
  constexpr NativeSocket InvalidSocket = -1;

#endif

} // namespace softadastra

#endif // SOFTADASTRA_HOST_NATIVE_SOCKET_HPP
