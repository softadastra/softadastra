/**
 *
 *  @file mdns_publisher_test.cpp
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

#include "platform/MdnsPublisher.hpp"

#include <gtest/gtest.h>

namespace
{
  TEST(MdnsPublisherTest, DerivesStableCollisionResistantLocalName)
  {
    softadastra::MdnsPublisher publisher(
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");

    EXPECT_EQ(publisher.name(), "softadastra-01234567.local");
    EXPECT_FALSE(publisher.start(""));
  }
} // namespace
