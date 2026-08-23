/**
 *
 *  @file software_id_test.cpp
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

#include "software/SoftwareId.hpp"
#include <gtest/gtest.h>

TEST(SoftwareIdTest, PreservesIdentifierValue)
{
  const softadastra::SoftwareId id("inventory");

  EXPECT_EQ(id.value(), "inventory");
}

TEST(SoftwareIdTest, EqualValuesProduceEqualIdentifiers)
{
  const softadastra::SoftwareId first("inventory");
  const softadastra::SoftwareId second("inventory");

  EXPECT_EQ(first, second);
}

TEST(SoftwareIdTest, DifferentValuesProduceDifferentIdentifiers)
{
  const softadastra::SoftwareId first("inventory");
  const softadastra::SoftwareId second("accounting");

  EXPECT_NE(first, second);
}
