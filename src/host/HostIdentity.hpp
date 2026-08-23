/**
 *
 *  @file HostIdentity.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_IDENTITY_HPP
#define SOFTADASTRA_HOST_HOST_IDENTITY_HPP

#include <filesystem>
#include <string>

namespace softadastra
{
  /**
   * @brief Stores the persistent cryptographic Host identity and admin secret.
   */
  class HostIdentity
  {
  public:
    /** @brief Creates identity storage at an explicit local path. */
    explicit HostIdentity(std::filesystem::path path) noexcept;

    /** @brief Loads an existing identity or creates it once. */
    [[nodiscard]] bool load_or_create();

    /** @brief Returns the SHA-256 fingerprint of the Ed25519 public key. */
    [[nodiscard]] const std::string &id() const noexcept;

    /** @brief Returns the persistent Ed25519 public key as hexadecimal text. */
    [[nodiscard]] const std::string &public_key() const noexcept;

    /**
     * @brief Writes an Ed25519 self-signed TLS certificate for this identity.
     *
     * The certificate public key is the key from which HostId is derived.
     */
    [[nodiscard]] bool write_tls_certificate(
        const std::filesystem::path &certificate_path,
        const std::filesystem::path &private_key_path) const;

    /** @brief Returns the remote administration secret. */
    [[nodiscard]] const std::string &secret() const noexcept;

  private:
    std::filesystem::path path_;
    std::string id_;
    std::string secret_;
    std::string public_key_;
    std::string private_key_;
  };
}

#endif // SOFTADASTRA_HOST_HOST_IDENTITY_HPP
