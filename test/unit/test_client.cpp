// client_test.h
//
// Unit tests for the client class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2020-2025 Frank Pagliughi <fpagliughi@mindspring.com>
 * Copyright (c) 2017 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
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
 *    Guilherme M. Ferreira
 *        - initial implementation and documentation
 *    Frank Pagliughi
 *        - updated tests for modified v1.0 client API
 *        - Converted to use Catch2
 *******************************************************************************/

#define UNIT_TESTS

#include "catch2_version.h"
#include "mock_action_listener.h"
#include "mock_callback.h"
#include "mock_persistence.h"
#include "mqtt/client.h"

using namespace std::chrono;
using namespace mqtt;

/////////////////////////////////////////////////////////////////////////////

// NOTE: This test case requires network access. It uses one of
//  	 the public available MQTT brokers
#if defined(TEST_EXTERNAL_SERVER)
static const std::string GOOD_SERVER_URI{"tcp://mqtt.eclipseprojects.io:1883"};
#else
static const std::string GOOD_SERVER_URI{"tcp://localhost:1883"};
#endif
static const std::string BAD_SERVER_URI{"one://invalid.address"};
static const std::string CLIENT_ID{"client_test"};
static const std::string TOPIC{"TOPIC"};

static const persistence_type PERSISTENCE_DIR{"persist"};

static const int GOOD_QOS{0};
static const int BAD_QOS{3};

static mqtt::string_collection TOPIC_COLL{"TOPIC0", "TOPIC1", "TOPIC2"};
static mqtt::client::qos_collection GOOD_QOS_COLL{0, 1, 2};
static mqtt::client::qos_collection BAD_QOS_COLL{BAD_QOS};

static const std::string PAYLOAD{"PAYLOAD"};
// const int TIMEOUT { 1000 };
// int CONTEXT { 4 };
static mock_action_listener listener;
static const bool RETAINED{false};

//----------------------------------------------------------------------
// Test constructors client::client()
//----------------------------------------------------------------------

TEST_CASE("client user constructor 2 string args", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};

    REQUIRE(GOOD_SERVER_URI == cli.get_server_uri());
    REQUIRE(CLIENT_ID == cli.get_client_id());
}

TEST_CASE("client user constructor 2 string args failure", "[client]")
{
    int return_code = MQTTASYNC_SUCCESS;
    try {
        mqtt::client cli{BAD_SERVER_URI, CLIENT_ID};
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_BAD_PROTOCOL == return_code);
}

TEST_CASE("client user constructor 3 string args", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID, PERSISTENCE_DIR};

    REQUIRE(GOOD_SERVER_URI == cli.get_server_uri());
    REQUIRE(CLIENT_ID == cli.get_client_id());
}

TEST_CASE("client user constructor 3 args", "[client]")
{
    mock_persistence cp;
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID, &cp};

    REQUIRE(GOOD_SERVER_URI == cli.get_server_uri());
    REQUIRE(CLIENT_ID == cli.get_client_id());

    mqtt::client cli_no_persistence{GOOD_SERVER_URI, CLIENT_ID, nullptr};

    REQUIRE(GOOD_SERVER_URI == cli_no_persistence.get_server_uri());
    REQUIRE(CLIENT_ID == cli_no_persistence.get_client_id());
}

//----------------------------------------------------------------------
// Test client::connect()
//----------------------------------------------------------------------

TEST_CASE("client connect 0 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());
}

TEST_CASE("client connect 1 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mqtt::connect_options co;
    cli.connect(co);
    REQUIRE(cli.is_connected());
}

TEST_CASE("client connect 1 arg failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mqtt::connect_options co;
    mqtt::will_options wo;
    wo.set_qos(BAD_QOS);  // Invalid QoS causes connection failure
    co.set_will(wo);
    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.connect(co);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(!cli.is_connected());
    REQUIRE(MQTTASYNC_BAD_QOS == return_code);
}

//----------------------------------------------------------------------
// Test client::disconnect()
//----------------------------------------------------------------------

TEST_CASE("client disconnect 0 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client disconnect 1 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.disconnect(0);
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client disconnect 1 arg failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.disconnect(0);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(!cli.is_connected());
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

//----------------------------------------------------------------------
// Test client::get_timeout() and client::set_timeout() using ints
//----------------------------------------------------------------------

TEST_CASE("client timeout int", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int timeout{std::numeric_limits<int>::min()};
    cli.set_timeout(timeout);
    REQUIRE(timeout == (int)cli.get_timeout().count());

    timeout = 0;
    cli.set_timeout(timeout);
    REQUIRE(timeout == (int)cli.get_timeout().count());

    timeout = std::numeric_limits<int>::max();
    cli.set_timeout(timeout);
    REQUIRE(timeout == (int)cli.get_timeout().count());
}

//----------------------------------------------------------------------
// Test client::get_timeout() and client::set_timeout() using durations
//----------------------------------------------------------------------

TEST_CASE("client timeout duration", "[client]")
{
    const int TIMEOUT_SEC = 120;
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};

    std::chrono::seconds timeout{TIMEOUT_SEC};
    cli.set_timeout(timeout);
    REQUIRE(timeout == cli.get_timeout());
    REQUIRE(TIMEOUT_SEC * 1000 == (int)cli.get_timeout().count());
}

//----------------------------------------------------------------------
// Test client::get_topic()
//----------------------------------------------------------------------

TEST_CASE("client get topic", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mqtt::topic t{cli.get_topic(TOPIC)};
    REQUIRE(TOPIC == t.get_name());
}

//----------------------------------------------------------------------
// Test client::publish()
//----------------------------------------------------------------------

TEST_CASE("client publish pointer 2 args", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    mqtt::message_ptr msg{mqtt::message::create(TOPIC, PAYLOAD)};
    cli.publish(msg);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client publish pointer 2 args failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        mqtt::message_ptr msg{mqtt::message::create(TOPIC, PAYLOAD)};
        cli.publish(msg);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("client publish reference 2 args", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    mqtt::message msg{TOPIC, PAYLOAD};
    cli.publish(msg);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client publish 5 args", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    const void* payload{PAYLOAD.c_str()};
    const size_t payload_size{PAYLOAD.size()};
    cli.publish(TOPIC, payload, payload_size, GOOD_QOS, RETAINED);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

//----------------------------------------------------------------------
// Test client::set_callback()
//----------------------------------------------------------------------

TEST_CASE("client set callback", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mock_callback cb;
    cli.set_callback(cb);
}

//----------------------------------------------------------------------
// Test client::subscribe()
//----------------------------------------------------------------------

TEST_CASE("client subscribe single topic 1 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.subscribe(TOPIC);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client subscribe single topic 1 arg failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.subscribe(TOPIC);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("client subscribe single topic 2 args", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.subscribe(TOPIC, GOOD_QOS);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client subscribe single topic 2 args failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.subscribe(TOPIC, BAD_QOS);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("client subscribe many topics 1 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.subscribe(TOPIC_COLL);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client subscribe many topics 1 arg failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.subscribe(TOPIC_COLL);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("client subscribe many topics 2 args", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client subscribe many topics 2 args failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    try {
        cli.subscribe(TOPIC_COLL, BAD_QOS_COLL);
    }
    catch (std::invalid_argument&) {
    }

    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

//----------------------------------------------------------------------
// Test client::unsubscribe()
//----------------------------------------------------------------------

TEST_CASE("client unsubscribe single topic 1 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.subscribe(TOPIC, 1);
    cli.unsubscribe(TOPIC);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client unsubscribe single topic 1 arg failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.unsubscribe(TOPIC);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("client unsubscribe many topics 1 arg", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    cli.connect();
    REQUIRE(cli.is_connected());

    cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL);
    cli.unsubscribe(TOPIC_COLL);

    cli.disconnect();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("client unsubscribe many topics 1 arg failure", "[client]")
{
    mqtt::client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        cli.unsubscribe(TOPIC_COLL);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}
