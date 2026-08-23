/**
 *
 *  @file LocalControlServer.hpp
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

#ifndef SOFTADASTRA_CONTROL_LOCAL_CONTROL_SERVER_HPP
#define SOFTADASTRA_CONTROL_LOCAL_CONTROL_SERVER_HPP

#include "control/ControlServer.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace softadastra
{
  /**
   * @brief Serves Host control operations through a local platform channel.
   */
  class LocalControlServer
  {
  public:
    /**
     * @brief Creates a local control server for a Host control surface.
     */
    LocalControlServer(
        ControlServer &server,
        std::filesystem::path path) noexcept;

    ~LocalControlServer();

    LocalControlServer(const LocalControlServer &) = delete;
    LocalControlServer &operator=(const LocalControlServer &) = delete;

    /**
     * @brief Opens the local control endpoint.
     */
    [[nodiscard]] bool start();

    /**
     * @brief Serves all currently pending local control requests.
     */
    [[nodiscard]] bool process_pending();

    /**
     * @brief Closes the local control endpoint.
     */
    void stop() noexcept;

  private:
    [[nodiscard]] std::string handle(std::string_view request);

    ControlServer &server_;
    std::filesystem::path path_;
    int descriptor_{-1};
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_LOCAL_CONTROL_SERVER_HPP
