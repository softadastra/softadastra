#ifndef SOFTADASTRA_HOST_NATIVE_SOCKET_HPP
#define SOFTADASTRA_HOST_NATIVE_SOCKET_HPP

#if defined(_WIN32)
#include <winsock2.h>
namespace softadastra { using NativeSocket = SOCKET; constexpr NativeSocket InvalidSocket = INVALID_SOCKET; }
#else
namespace softadastra { using NativeSocket = int; constexpr NativeSocket InvalidSocket = -1; }
#endif

#endif
