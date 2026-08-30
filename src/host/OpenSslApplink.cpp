/**
 * OpenSSL's Windows file BIOs need this bridge when OpenSSL is consumed from
 * vcpkg/MSVC.  Define it exactly once in the executable's linked sources.
 */

#include <openssl/applink.c>
