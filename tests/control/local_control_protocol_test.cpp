/**
 *
 *  @file local_control_protocol_test.cpp
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

#include "control/LocalControlProtocol.hpp"

#include <gtest/gtest.h>

namespace
{
  TEST(LocalControlProtocolTest, PreservesEmptyAndWhitespaceFields)
  {
    const auto empty = softadastra::LocalControlProtocol::decode(
        softadastra::LocalControlProtocol::encode(""));
    const auto text = softadastra::LocalControlProtocol::decode(
        softadastra::LocalControlProtocol::encode("argument with spaces"));

    ASSERT_TRUE(empty.has_value());
    ASSERT_TRUE(text.has_value());
    EXPECT_TRUE(empty->empty());
    EXPECT_EQ(text.value(), "argument with spaces");
  }

  TEST(LocalControlProtocolTest, RejectsInvalidEncodedField)
  {
    EXPECT_FALSE(
        softadastra::LocalControlProtocol::decode("not-hex").has_value());
  }
} // namespace
