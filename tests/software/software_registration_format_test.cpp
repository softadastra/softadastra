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

  TEST(SoftwareRegistrationFormatTest, PreservesProjectIdentityAndWorkingDirectory)
  {
    const std::vector<softadastra::SoftwareEntry> entries{
        softadastra::SoftwareEntry(
            softadastra::SoftwareId("moved-project"),
            softadastra::ProcessSpec("./build/app", {}, "/old/project"),
            softadastra::ProjectIdentity("opaque-project-id"))};

    const auto restored = softadastra::SoftwareRegistrationFormat::deserialize(
        softadastra::SoftwareRegistrationFormat::serialize(entries));

    ASSERT_TRUE(restored.has_value());
    ASSERT_EQ(restored->size(), 1U);
    ASSERT_TRUE(restored->front().project_identity().has_value());
    EXPECT_EQ(restored->front().project_identity()->value(), "opaque-project-id");
    EXPECT_EQ(restored->front().process_spec().working_directory(), "/old/project");
  }

  TEST(SoftwareRegistrationFormatTest, PreservesDeclaredCommandAndMultipleAccessPoints)
  {
    const auto http = softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080);
    const auto websocket = softadastra::AccessPoint::create(softadastra::AccessProtocol::Ws, 9090);
    ASSERT_TRUE(http); ASSERT_TRUE(websocket);
    const std::vector<softadastra::SoftwareEntry> entries{softadastra::SoftwareEntry(
        softadastra::SoftwareId("cloud"), softadastra::ProcessSpec("/bin/sh", {"-lc", "vix run"}, "/project"),
        std::nullopt, {*http, *websocket}, "vix run", "Cloud")};
    const auto restored = softadastra::SoftwareRegistrationFormat::deserialize(
        softadastra::SoftwareRegistrationFormat::serialize(entries));
    ASSERT_TRUE(restored); ASSERT_EQ(restored->size(), 1U);
    EXPECT_EQ(restored->front().declared_command(), "vix run");
    ASSERT_EQ(restored->front().access_points().size(), 2U);
    EXPECT_EQ(restored->front().access_points()[1].protocol(), softadastra::AccessProtocol::Ws);
  }

  TEST(SoftwareRegistrationFormatTest, ReadsVersionSixSingleAccessRegistrations)
  {
    const auto access = softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080);
    ASSERT_TRUE(access);
    const std::vector<softadastra::SoftwareEntry> entries{softadastra::SoftwareEntry(
        softadastra::SoftwareId("legacy"), softadastra::ProcessSpec("app"), std::nullopt, access, "app", "Legacy")};
    auto serialized = softadastra::SoftwareRegistrationFormat::serialize(entries);
    serialized.replace(0, std::string("softadastra-registrations 7\n").size(), "softadastra-registrations 6\n");
    const auto restored = softadastra::SoftwareRegistrationFormat::deserialize(serialized);
    ASSERT_TRUE(restored); ASSERT_EQ(restored->front().access_points().size(), 1U);
    EXPECT_EQ(restored->front().access_points().front().port(), 8080);
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
