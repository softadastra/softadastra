/**
 *
 *  @file host_identity_test.cpp
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

#include "host/HostIdentity.hpp"

#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <openssl/evp.h>

namespace
{
  std::string hexadecimal(const unsigned char *bytes, std::size_t size)
  {
    constexpr char digits[] = "0123456789abcdef";
    std::string value;

    for (std::size_t index = 0; index < size; ++index)
    {
      value += digits[bytes[index] >> 4U];
      value += digits[bytes[index] & 0x0fU];
    }

    return value;
  }

  std::array<unsigned char, 32> decode_public_key(const std::string &value)
  {
    std::array<unsigned char, 32> result{};

    for (std::size_t index = 0; index < result.size(); ++index)
    {
      const auto digit = [](char character)
      {
        return character >= 'a' ? character - 'a' + 10 : character - '0';
      };
      result[index] = static_cast<unsigned char>(
          (digit(value[index * 2]) << 4) | digit(value[index * 2 + 1]));
    }

    return result;
  }

  TEST(HostIdentityTest, PersistsCryptographicIdentityAndSeparateSecret)
  {
    const auto directory = std::filesystem::temp_directory_path() / "softadastra-host-identity-test";
    const auto path = directory / "identity";
    std::filesystem::remove_all(directory);

    softadastra::HostIdentity first(path);
    ASSERT_TRUE(first.load_or_create());
    const auto id = first.id();
    const auto secret = first.secret();
    const auto public_key = first.public_key();
    const auto public_key_bytes = decode_public_key(public_key);

    std::array<unsigned char, 32> digest{};
    std::size_t digest_size = digest.size();
    ASSERT_EQ(
        EVP_Q_digest(
            nullptr,
            "SHA256",
            nullptr,
            public_key_bytes.data(),
            public_key_bytes.size(),
            digest.data(),
            &digest_size),
        1);
    EXPECT_EQ(id, hexadecimal(digest.data(), digest_size));

    softadastra::HostIdentity second(path);
    ASSERT_TRUE(second.load_or_create());
    EXPECT_EQ(second.id(), id);
    EXPECT_EQ(second.public_key(), public_key);
    EXPECT_EQ(second.secret(), secret);
    EXPECT_NE(id, secret);
    EXPECT_NE(public_key, secret);
    std::filesystem::remove_all(directory);
  }

  TEST(HostIdentityTest, MigratesLegacyRandomIdentityToEd25519)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-host-identity-migration-test";
    const auto path = directory / "identity";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    std::ofstream output(path);
    output << "legacy-id\nlegacy-secret\n";
    output.close();

    softadastra::HostIdentity identity(path);
    ASSERT_TRUE(identity.load_or_create());
    EXPECT_EQ(identity.id().size(), 64U);
    EXPECT_EQ(identity.public_key().size(), 64U);
    EXPECT_NE(identity.id(), "legacy-id");

    std::filesystem::remove_all(directory);
  }
}
