/**
 *
 *  @file software_registration_format_test.cpp
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

#include "software/SoftwareRegistrationFormat.hpp"
#include "software/AccessPoint.hpp"

#include <gtest/gtest.h>

namespace
{
  TEST(SoftwareRegistrationFormatTest, PreservesMultipleRegistrations)
  {
    const std::vector<softadastra::SoftwareEntry> entries{
        softadastra::SoftwareEntry(
            softadastra::SoftwareId("first software"),
            softadastra::ProcessSpec("/usr/bin/first app", {"", "two words"})),
        softadastra::SoftwareEntry(
            softadastra::SoftwareId("second"),
            softadastra::ProcessSpec("second", {"--port", "8080"}))};

    const auto parsed = softadastra::SoftwareRegistrationFormat::deserialize(
        softadastra::SoftwareRegistrationFormat::serialize(entries));

    ASSERT_TRUE(parsed.has_value());
    ASSERT_EQ(parsed->size(), 2U);
    EXPECT_EQ(parsed->at(0).id().value(), "first software");
    EXPECT_EQ(parsed->at(0).process_spec().executable(), "/usr/bin/first app");
    EXPECT_EQ(parsed->at(0).process_spec().arguments().at(0), "");
    EXPECT_EQ(parsed->at(0).process_spec().arguments().at(1), "two words");
  }

  TEST(SoftwareRegistrationFormatTest, PreservesAccessPoint)
  {
    const auto protocol = softadastra::AccessPoint::protocol("http");
    ASSERT_TRUE(protocol.has_value());
    const auto access_point = softadastra::AccessPoint::create(protocol.value(), 8080);
    ASSERT_TRUE(access_point.has_value());

    const std::vector<softadastra::SoftwareEntry> entries{
        softadastra::SoftwareEntry(
            softadastra::SoftwareId("phone-test"),
            softadastra::ProcessSpec("python3", {}),
            access_point)};
    const auto restored = softadastra::SoftwareRegistrationFormat::deserialize(
        softadastra::SoftwareRegistrationFormat::serialize(entries));

    ASSERT_TRUE(restored.has_value());
    ASSERT_EQ(restored->size(), 1U);
    ASSERT_TRUE(restored->front().access_point().has_value());
    EXPECT_EQ(restored->front().access_point()->port(), 8080);
    EXPECT_EQ(restored->front().access_point()->protocol(), softadastra::AccessProtocol::Http);
  }

  TEST(SoftwareRegistrationFormatTest, RejectsInvalidInput)
  {
    EXPECT_FALSE(softadastra::SoftwareRegistrationFormat::deserialize("invalid").has_value());
  }

  TEST(SoftwareRegistrationFormatTest, RejectsInvalidRegistrations)
  {
    EXPECT_FALSE(softadastra::SoftwareRegistrationFormat::deserialize(
        "softadastra-registrations 1\n1\n0\n\n1\na\n0\n").has_value());
    EXPECT_FALSE(softadastra::SoftwareRegistrationFormat::deserialize(
        "softadastra-registrations 1\n2\n1\na\n1\nx\n0\n1\na\n1\ny\n0\n").has_value());
    EXPECT_FALSE(softadastra::SoftwareRegistrationFormat::deserialize(
        "softadastra-registrations 1\n18446744073709551615\n").has_value());
    EXPECT_FALSE(softadastra::SoftwareRegistrationFormat::deserialize(
        "softadastra-registrations 1\n1\n1\na\n1\nx\n18446744073709551615\n").has_value());
  }
} // namespace
