// test_will_options.cpp
//
// Unit tests for the will_options class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2016-2020 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#define UNIT_TESTS

#include <cstring>

#include "catch2_version.h"
#include "mock_async_client.h"
#include "mqtt/will_options.h"

using namespace mqtt;

static const std::string EMPTY_STR;
static const int DFLT_QOS = will_options::DFLT_QOS;
static const bool DFLT_RETAINED = will_options::DFLT_RETAINED;

// C struct signature/eyecatcher
static const char* CSIG = "MQTW";
static const size_t CSIG_LEN = std::strlen(CSIG);

static const std::string TOPIC = "hello";
static const char* BUF = "Hello there";
static const size_t N = std::strlen(BUF);
static const std::string PAYLOAD = std::string(BUF);
static const int QOS = 1;
static const bool RETAINED = true;

// ----------------------------------------------------------------------
// Test the default constructor
// ----------------------------------------------------------------------

TEST_CASE("will_options default ctor", "[options]")
{
    mqtt::will_options opts;

    REQUIRE(EMPTY_STR == opts.get_topic());
    REQUIRE(EMPTY_STR == opts.get_payload_str());
    REQUIRE(DFLT_QOS == opts.get_qos());
    REQUIRE(DFLT_RETAINED == opts.is_retained());

    // Test the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(c_struct.topicName != nullptr);
    REQUIRE(size_t(0) == strlen(c_struct.topicName));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(0 == c_struct.payload.len);
    REQUIRE(c_struct.payload.data == nullptr);
}

// ----------------------------------------------------------------------
// Test the raw buffer (void*) constructor
// ----------------------------------------------------------------------

TEST_CASE("will_options string buf ctor", "[options]")
{
    mock_async_client cli;
    mqtt::topic topic{cli, TOPIC};

    mqtt::will_options opts(topic, BUF, N, QOS, true);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Test the C struct
    // Remember we now fill payload fields, not message
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(N == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(BUF, c_struct.payload.data, N));
}

// ----------------------------------------------------------------------
// Test the raw buffer (void*) constructor
// ----------------------------------------------------------------------

TEST_CASE("will_options topic buf ctor", "[options]")
{
    mqtt::will_options opts(TOPIC, BUF, N, QOS, true);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Test the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(N == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(BUF, c_struct.payload.data, N));
}

// ----------------------------------------------------------------------
// Test the string payload constructor
// ----------------------------------------------------------------------

TEST_CASE("will_options string string ctor", "[options]")
{
    mqtt::will_options opts(TOPIC, PAYLOAD, QOS, true);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Test the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(PAYLOAD.size() == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload.data, PAYLOAD.size()));
}

// ----------------------------------------------------------------------
// Test the message payload constructor
// ----------------------------------------------------------------------

TEST_CASE("will_options string message ctor", "[options]")
{
    mqtt::message msg(TOPIC, PAYLOAD, QOS, true);
    mqtt::will_options opts(msg);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Test the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(PAYLOAD.size() == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload.data, PAYLOAD.size()));
}

// ----------------------------------------------------------------------
// Test the copy constructor
// ----------------------------------------------------------------------

TEST_CASE("will_options copy ctor", "[options]")
{
    auto orgOpts = mqtt::will_options(TOPIC, BUF, N, QOS, RETAINED);

    mqtt::will_options opts(orgOpts);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Check the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(N == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(BUF, c_struct.payload.data, N));

    // Make sure it's a true copy, not linked to the original
    orgOpts.set_topic(EMPTY_STR);
    orgOpts.set_payload(EMPTY_STR);
    orgOpts.set_qos(DFLT_QOS);
    orgOpts.set_retained(false);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());
}

// ----------------------------------------------------------------------
// Test the move constructor
// ----------------------------------------------------------------------

TEST_CASE("will_options move ctor", "[options]")
{
    auto orgOpts = mqtt::will_options(TOPIC, BUF, N, QOS, RETAINED);

    mqtt::will_options opts(std::move(orgOpts));

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Check the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(N == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(BUF, c_struct.payload.data, N));

    // Check that the original was moved
    REQUIRE(EMPTY_STR == orgOpts.get_topic());
    REQUIRE(EMPTY_STR == orgOpts.get_payload_str());
}

// ----------------------------------------------------------------------
// Test the copy assignment operator=(const&)
// ----------------------------------------------------------------------

TEST_CASE("will_options copy assignment", "[options]")
{
    auto orgOpts = mqtt::will_options(TOPIC, BUF, N, QOS, RETAINED);

    mqtt::will_options opts;
    opts = orgOpts;

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Check the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(N == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(BUF, c_struct.payload.data, N));

    // Make sure it's a true copy, not linked to the original
    orgOpts.set_topic(EMPTY_STR);
    orgOpts.set_payload(EMPTY_STR);
    orgOpts.set_qos(DFLT_QOS);
    orgOpts.set_retained(false);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Self assignment should cause no harm
    opts = opts;

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());
}

// ----------------------------------------------------------------------
// Test the move assignment, operator=(&&)
// ----------------------------------------------------------------------

TEST_CASE("will_options move assignment", "[options]")
{
    auto orgOpts = mqtt::will_options(TOPIC, BUF, N, QOS, RETAINED);

    mqtt::will_options opts;
    opts = std::move(orgOpts);

    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());

    // Check the C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));
    REQUIRE(c_struct.message == nullptr);
    REQUIRE(N == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(BUF, c_struct.payload.data, N));

    // Check that the original was moved
    REQUIRE(EMPTY_STR == orgOpts.get_topic());
    REQUIRE(EMPTY_STR == orgOpts.get_payload_str());

// Self assignment should cause no harm
// (clang++ is smart enough to warn about this)
#if !defined(__clang__)
    opts = std::move(opts);
    REQUIRE(TOPIC == opts.get_topic());
    REQUIRE(PAYLOAD == opts.get_payload_str());
    REQUIRE(QOS == opts.get_qos());
    REQUIRE(RETAINED == opts.is_retained());
#endif
}

// ----------------------------------------------------------------------
// Test setting the (text) topic
// ----------------------------------------------------------------------

TEST_CASE("will_options set_topic_str", "[options]")
{
    mqtt::will_options opts;

    opts.set_topic(TOPIC);
    REQUIRE(TOPIC == opts.get_topic());

    const auto& c_struct = opts.c_struct();
    REQUIRE(0 == strcmp(c_struct.topicName, TOPIC.c_str()));

    // Setting empty string should _not_ create nullptr entry, in
    // C struct, rather a valid zero-length string.
    opts.set_topic(EMPTY_STR);

    REQUIRE(EMPTY_STR == opts.get_topic());
    REQUIRE(c_struct.topicName != nullptr);
    REQUIRE(size_t(0) == strlen(c_struct.topicName));
}

// ----------------------------------------------------------------------
// Test setting the (binary) payload
// ----------------------------------------------------------------------

TEST_CASE("will_options set_payload", "[options]")
{
    mqtt::will_options opts;

    opts.set_payload(PAYLOAD);
    REQUIRE(PAYLOAD == opts.get_payload_str());

    const auto& c_struct = opts.c_struct();

    REQUIRE(PAYLOAD.size() == size_t(c_struct.payload.len));
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload.data, PAYLOAD.size()));

    // Setting empty string set a valid, but zero-len payload
    // TODO: We need to check what the C lib now accepts.
    opts.set_payload(EMPTY_STR);

    REQUIRE(EMPTY_STR == opts.get_payload_str());
    REQUIRE(size_t(0) == opts.get_payload().size());

    REQUIRE(0 == c_struct.payload.len);
    REQUIRE(c_struct.payload.data != nullptr);
}
