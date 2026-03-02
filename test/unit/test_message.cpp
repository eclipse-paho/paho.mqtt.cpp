// message_test.cpp
//
// Unit tests for the message class in the Paho MQTT C++ library.
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
#include "mqtt/message.h"

using namespace mqtt;

static const std::string EMPTY_STR;
static const int DFLT_QOS = 0;
static const bool DFLT_RETAINED = false;
static const bool DFLT_DUP = false;

static const std::string TOPIC{"hello"};
static const char* BUF = "Hello there";
static const size_t N = std::strlen(BUF);
static const std::string PAYLOAD{BUF};
;
static const int QOS = 1;
static const std::string RESPONSE_TOPIC{"replies"};
static const properties PROPS{{property::RESPONSE_TOPIC, RESPONSE_TOPIC}};

// --------------------------------------------------------------------------
// Test the default constructor
// --------------------------------------------------------------------------

TEST_CASE("default constructor", "[message]")
{
    mqtt::message msg;

    REQUIRE(msg.get_payload_str().empty());
    REQUIRE(DFLT_QOS == msg.get_qos());
    REQUIRE(DFLT_RETAINED == msg.is_retained());
    REQUIRE(DFLT_DUP == msg.is_duplicate());
    REQUIRE(msg.get_properties().empty());

    const auto& c_struct = msg.c_struct();

    REQUIRE(0 == c_struct.payloadlen);
    REQUIRE(nullptr == c_struct.payload);
    REQUIRE(DFLT_QOS == c_struct.qos);
    REQUIRE(DFLT_RETAINED == (c_struct.retained != 0));
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));
    REQUIRE(0 == c_struct.properties.count);
}

// --------------------------------------------------------------------------
// Test the raw buffer (void*) and length constructor
// --------------------------------------------------------------------------

TEST_CASE("buf len constructor", "[message]")
{
    mqtt::message msg(TOPIC, BUF, N);

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(DFLT_QOS == msg.get_qos());
    REQUIRE(DFLT_RETAINED == msg.is_retained());
    REQUIRE(DFLT_DUP == msg.is_duplicate());

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(N) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(BUF, c_struct.payload, N));
    REQUIRE(DFLT_QOS == c_struct.qos);
    REQUIRE(DFLT_RETAINED == (c_struct.retained != 0));
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));
}

// --------------------------------------------------------------------------
// Test the raw buffer (void*) constructor
// --------------------------------------------------------------------------

TEST_CASE("buf constructor", "[message]")
{
    mqtt::message msg(TOPIC, BUF, N, QOS, true, PROPS);

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());
    REQUIRE(DFLT_DUP == msg.is_duplicate());

    const auto& props = msg.get_properties();
    REQUIRE(1 == props.count(property::RESPONSE_TOPIC));
    REQUIRE(RESPONSE_TOPIC == get<std::string>(props, property::RESPONSE_TOPIC));

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(N) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(BUF, c_struct.payload, N));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));
}

// --------------------------------------------------------------------------
// Test the string buffer constructor
// --------------------------------------------------------------------------

TEST_CASE("string constructor", "[message]")
{
    mqtt::message msg(TOPIC, PAYLOAD);

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(DFLT_QOS == msg.get_qos());
    REQUIRE(!msg.is_retained());
    REQUIRE(DFLT_DUP == msg.is_duplicate());
    REQUIRE(msg.get_properties().empty());

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(PAYLOAD.size()) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload, PAYLOAD.size()));
    REQUIRE(DFLT_QOS == c_struct.qos);
    REQUIRE(DFLT_RETAINED == (c_struct.retained != 0));
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));
}

// --------------------------------------------------------------------------
// Test the string buffer with QoS constructor
// --------------------------------------------------------------------------

TEST_CASE("string qos constructor", "[message]")
{
    mqtt::message msg(TOPIC, PAYLOAD, QOS, true, PROPS);

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());
    REQUIRE(DFLT_DUP == msg.is_duplicate());

    const auto& props = msg.get_properties();
    REQUIRE(1 == props.count(property::RESPONSE_TOPIC));
    REQUIRE(RESPONSE_TOPIC == get<std::string>(props, property::RESPONSE_TOPIC));

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(PAYLOAD.size()) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload, PAYLOAD.size()));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));
}

// --------------------------------------------------------------------------
// Test the initialization by C struct
// --------------------------------------------------------------------------

TEST_CASE("c struct constructor", "[message]")
{
    MQTTAsync_message c_msg = MQTTAsync_message_initializer;

    c_msg.payload = const_cast<char*>(BUF);
    c_msg.payloadlen = int(N);
    c_msg.qos = QOS;
    c_msg.retained = 1;
    c_msg.dup = 1;

    // TODO: Test extracting v5 properties

    mqtt::message msg(TOPIC, c_msg);

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());
    REQUIRE(msg.is_duplicate());

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(N) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(BUF, c_struct.payload, N));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(c_struct.dup != 0);
}

// --------------------------------------------------------------------------
// Test the copy constructor
// --------------------------------------------------------------------------

TEST_CASE("copy constructor", "[message]")
{
    auto orgMsg = mqtt::message(TOPIC, PAYLOAD, QOS, true, PROPS);

    mqtt::message msg(orgMsg);

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());

    const auto& props = msg.get_properties();
    REQUIRE(1 == props.count(property::RESPONSE_TOPIC));
    REQUIRE(RESPONSE_TOPIC == get<std::string>(props, property::RESPONSE_TOPIC));

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(PAYLOAD.size()) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload, PAYLOAD.size()));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));

    // Make sure it's a true copy, not linked to the original
    orgMsg.set_payload(EMPTY_STR);
    orgMsg.set_qos(DFLT_QOS);
    orgMsg.set_retained(false);

    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());
}

// --------------------------------------------------------------------------
// Test the move constructor
// --------------------------------------------------------------------------

TEST_CASE("move constructor", "[message]")
{
    auto orgMsg = mqtt::message(TOPIC, PAYLOAD, QOS, true, PROPS);

    mqtt::message msg(std::move(orgMsg));

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());

    const auto& props = msg.get_properties();
    REQUIRE(1 == props.count(property::RESPONSE_TOPIC));
    REQUIRE(RESPONSE_TOPIC == get<std::string>(props, property::RESPONSE_TOPIC));

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(PAYLOAD.size()) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload, PAYLOAD.size()));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));

    // Check that the original was moved
    REQUIRE(size_t(0) == orgMsg.get_payload().size());
    REQUIRE(orgMsg.get_properties().empty());
}

// --------------------------------------------------------------------------
// Test the copy assignment operator=(const&)
// --------------------------------------------------------------------------

TEST_CASE("copy assignment", "[message]")
{
    auto orgMsg = mqtt::message(TOPIC, PAYLOAD, QOS, true, PROPS);

    mqtt::message msg;
    msg = orgMsg;

    // Make sure it's a true copy, not linked to the original
    orgMsg.set_payload(EMPTY_STR);
    orgMsg.set_qos(DFLT_QOS);
    orgMsg.set_retained(false);
    orgMsg.set_properties({});

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());

    const auto& props = msg.get_properties();
    REQUIRE(1 == props.count(property::RESPONSE_TOPIC));
    REQUIRE(RESPONSE_TOPIC == get<std::string>(props, property::RESPONSE_TOPIC));

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(PAYLOAD.size()) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload, PAYLOAD.size()));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));

    // Self assignment should cause no harm
    msg = msg;

    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());
}

// --------------------------------------------------------------------------
// Test the move assignment, operator=(&&)
// --------------------------------------------------------------------------

TEST_CASE("move assignment", "[message]")
{
    auto orgMsg = mqtt::message(TOPIC, PAYLOAD, QOS, true, PROPS);

    mqtt::message msg;
    msg = std::move(orgMsg);

    REQUIRE(TOPIC == msg.get_topic());
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());

    const auto& props = msg.get_properties();
    REQUIRE(1 == props.count(property::RESPONSE_TOPIC));
    REQUIRE(RESPONSE_TOPIC == get<std::string>(props, property::RESPONSE_TOPIC));

    const auto& c_struct = msg.c_struct();

    REQUIRE(int(PAYLOAD.size()) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload, PAYLOAD.size()));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));

    // Check that the original was moved
    REQUIRE(size_t(0) == orgMsg.get_payload().size());
    REQUIRE(orgMsg.get_properties().empty());

// Self assignment should cause no harm
// (clang++ is smart enough to warn about this)
#if !defined(__clang__)
    msg = std::move(msg);
    REQUIRE(PAYLOAD == msg.get_payload_str());
    REQUIRE(QOS == msg.get_qos());
    REQUIRE(msg.is_retained());
#endif
}

// --------------------------------------------------------------------------
// Test the validate_qos()
// --------------------------------------------------------------------------

TEST_CASE("validate qos", "[message]")
{
    REQUIRE_THROWS_AS(mqtt::message::validate_qos(-1), mqtt::exception);
    REQUIRE_THROWS_AS(mqtt::message::validate_qos(3), mqtt::exception);

    REQUIRE_NOTHROW(mqtt::message::validate_qos(0));
}

/////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// Test the default builder
// --------------------------------------------------------------------------

TEST_CASE("default builder", "[message]")
{
    auto msg = mqtt::message_ptr_builder().finalize();

    REQUIRE(EMPTY_STR == msg->get_payload_str());
    REQUIRE(DFLT_QOS == msg->get_qos());
    REQUIRE(DFLT_RETAINED == msg->is_retained());
    REQUIRE(DFLT_DUP == msg->is_duplicate());

    const auto& c_struct = msg->c_struct();

    REQUIRE(0 == c_struct.payloadlen);
    REQUIRE(nullptr == c_struct.payload);
    REQUIRE(DFLT_QOS == c_struct.qos);
    REQUIRE(DFLT_RETAINED == (c_struct.retained != 0));
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));
}

// --------------------------------------------------------------------------
// Test a complete builder
// --------------------------------------------------------------------------

TEST_CASE("builder", "[message]")
{
    auto msg = mqtt::message_ptr_builder()
                   .topic(TOPIC)
                   .payload(PAYLOAD)
                   .qos(QOS)
                   .retained(true)
                   .properties(PROPS)
                   .finalize();

    REQUIRE(TOPIC == msg->get_topic());
    REQUIRE(PAYLOAD == msg->get_payload_str());
    REQUIRE(QOS == msg->get_qos());
    REQUIRE(msg->is_retained());
    REQUIRE(DFLT_DUP == msg->is_duplicate());

    const auto& props = msg->get_properties();
    REQUIRE(1 == props.count(property::RESPONSE_TOPIC));
    REQUIRE(RESPONSE_TOPIC == get<std::string>(props, property::RESPONSE_TOPIC));

    const auto& c_struct = msg->c_struct();

    REQUIRE(int(PAYLOAD.size()) == c_struct.payloadlen);
    REQUIRE(0 == memcmp(PAYLOAD.data(), c_struct.payload, PAYLOAD.size()));
    REQUIRE(QOS == c_struct.qos);
    REQUIRE(c_struct.retained != 0);
    REQUIRE(DFLT_DUP == (c_struct.dup != 0));
}
