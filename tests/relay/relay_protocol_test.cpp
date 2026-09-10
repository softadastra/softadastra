
/**
 *
 *  @file relay_protocol_test.cpp
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

#include "relay/RelayProtocol.hpp"
#include <gtest/gtest.h>

#include <cstddef>
#include <string>

namespace
{
  TEST(RelayProtocolTest, EncodesAndDecodesFrameHeaders)
  {
    const auto header =
        softadastra::RelayProtocol::encode_frame_header(258);

    ASSERT_TRUE(header.has_value());
    ASSERT_EQ(header->size(), 4);
    EXPECT_EQ(
        static_cast<unsigned char>((*header)[0]),
        0);
    EXPECT_EQ(
        static_cast<unsigned char>((*header)[1]),
        0);
    EXPECT_EQ(
        static_cast<unsigned char>((*header)[2]),
        1);
    EXPECT_EQ(
        static_cast<unsigned char>((*header)[3]),
        2);

    const auto decoded =
        softadastra::RelayProtocol::decode_frame_header(*header);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded.value(), 258U);
  }

  TEST(RelayProtocolTest, EncodesCompleteFrames)
  {
    const auto frame =
        softadastra::RelayProtocol::encode_frame("abc");

    ASSERT_TRUE(frame.has_value());
    ASSERT_EQ(frame->size(), 7);
    EXPECT_EQ(
        static_cast<unsigned char>((*frame)[0]),
        0);
    EXPECT_EQ(
        static_cast<unsigned char>((*frame)[1]),
        0);
    EXPECT_EQ(
        static_cast<unsigned char>((*frame)[2]),
        0);
    EXPECT_EQ(
        static_cast<unsigned char>((*frame)[3]),
        3);
    EXPECT_EQ(frame->substr(4), "abc");
  }

  TEST(RelayProtocolTest, RoundTripsHttpRequests)
  {
    softadastra::RelayHttpRequest request;
    request.software = "inventory";
    request.method = "POST";
    request.target = "/items?limit=10";
    request.headers.emplace_back("Host", "inventory.softadastra");
    request.headers.emplace_back("Content-Type", "application/json");
    request.body = R"({"name":"milk"})";

    const auto encoded =
        softadastra::RelayProtocol::encode_request(request);

    ASSERT_TRUE(encoded.has_value());
    ASSERT_FALSE(encoded->empty());
    EXPECT_EQ(
        static_cast<unsigned char>((*encoded)[0]),
        1);

    const auto decoded =
        softadastra::RelayProtocol::decode_request(*encoded);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->software, request.software);
    EXPECT_EQ(decoded->method, request.method);
    EXPECT_EQ(decoded->target, request.target);
    EXPECT_EQ(decoded->headers, request.headers);
    EXPECT_EQ(decoded->body, request.body);
  }

  TEST(RelayProtocolTest, RoundTripsRawHttpResponses)
  {
    const std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "OK";

    const auto encoded =
        softadastra::RelayProtocol::encode_response(response);

    ASSERT_TRUE(encoded.has_value());
    ASSERT_FALSE(encoded->empty());
    EXPECT_EQ(
        static_cast<unsigned char>((*encoded)[0]),
        2);

    const auto decoded =
        softadastra::RelayProtocol::decode_response(*encoded);

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded.value(), response);
  }

  TEST(RelayProtocolTest, RejectsInvalidFrameSizes)
  {
    EXPECT_FALSE(
        softadastra::RelayProtocol::valid_payload_size(0));
    EXPECT_FALSE(
        softadastra::RelayProtocol::encode_frame("").has_value());
    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_frame_header(
            std::string(4, '\0'))
            .has_value());
    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_frame_header(
            std::string(2, '\0'))
            .has_value());

    const std::string oversized_header{
        '\0',
        '\x10',
        '\0',
        '\x01'};

    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_frame_header(oversized_header)
            .has_value());
  }

  TEST(RelayProtocolTest, RejectsMalformedRequestsAndResponses)
  {
    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_request("").has_value());
    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_request(std::string(1, 2))
            .has_value());
    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_request(std::string(1, 1))
            .has_value());
    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_response("").has_value());
    EXPECT_FALSE(
        softadastra::RelayProtocol::decode_response(std::string(1, 1))
            .has_value());
  }

  TEST(RelayProtocolTest, RejectsTooManyHeaders)
  {
    softadastra::RelayHttpRequest request;
    request.software = "app";
    request.method = "GET";
    request.target = "/";

    for (std::size_t index = 0;
         index <= softadastra::RelayProtocol::maximum_headers;
         ++index)
    {
      request.headers.emplace_back(
          "X-Test",
          "ok");
    }

    EXPECT_FALSE(
        softadastra::RelayProtocol::encode_request(request).has_value());
  }

  TEST(RelayProtocolTest, RejectsFieldsLargerThanTheProtocolLimit)
  {
    softadastra::RelayHttpRequest request;
    request.software = std::string(
        softadastra::RelayProtocol::maximum_field_size + 1,
        'a');
    request.method = "GET";
    request.target = "/";

    EXPECT_FALSE(
        softadastra::RelayProtocol::encode_request(request).has_value());
  }
} // namespace
