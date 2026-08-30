/**
 *
 *  @file LocalControlEndpoint.hpp
 *  @brief Stable platform-local identities for the Host control endpoint.
 */

#ifndef SOFTADASTRA_CONTROL_LOCAL_CONTROL_ENDPOINT_HPP
#define SOFTADASTRA_CONTROL_LOCAL_CONTROL_ENDPOINT_HPP

#include <filesystem>

#if defined(_WIN32)

#include <string>

namespace softadastra
{
  /**
   * @brief Derives the named-pipe name for a local control endpoint.
   *
   * The supplied path is resolved to a long, absolute, case-normalized
   * Windows path before the endpoint identity is derived.
   */
  [[nodiscard]] std::wstring local_control_pipe_name(
      const std::filesystem::path &path);
} // namespace softadastra

#endif

#endif // SOFTADASTRA_CONTROL_LOCAL_CONTROL_ENDPOINT_HPP
