// test_async_client.cpp
//
// Unit tests for the async_client class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2020-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Guilherme M. Ferreira - initial implementation and documentation
 *    Frank Pagliughi - Converted to Catch2
 *******************************************************************************/
#define UNIT_TESTS

#include "catch2_version.h"
#include "mock_action_listener.h"
#include "mock_callback.h"
#include "mock_persistence.h"
#include "mqtt/async_client.h"
#include "mqtt/iasync_client.h"

using namespace mqtt;

/////////////////////////////////////////////////////////////////////////////

// NOTE: This test case requires network access. It uses one of
//  	 the public available MQTT brokers
#if defined(TEST_EXTERNAL_SERVER)
static const std::string GOOD_SERVER_URI{"tcp://mqtt.eclipseprojects.io:1883"};
static const std::string GOOD_SSL_SERVER_URI{"ssl://mqtt.eclipseprojects.io:1885"};
#else
static const std::string GOOD_SERVER_URI{"tcp://localhost:1883"};
static const std::string GOOD_SSL_SERVER_URI{"ssl://localhost:18885"};
#endif

static const std::string BAD_SERVER_URI{"one://invalid.address"};
static const std::string CLIENT_ID{"test_async_client"};
static const std::string TOPIC{"topic"};

static const persistence_type PERSISTENCE_DIR{"persist"};

static const int GOOD_QOS{0};
static const int BAD_QOS{3};
static const_string_collection_ptr TOPIC_COLL{
    string_collection::create({"topic0", "topic1", "topic2"})
};

static iasync_client::qos_collection GOOD_QOS_COLL{0, 1, 2};
static iasync_client::qos_collection BAD_QOS_COLL{BAD_QOS, 1, 2};

static const std::string PAYLOAD{"some payload"};
static const int TIMEOUT{1000};
static int CONTEXT{4};
static mock_action_listener listener;
static const bool RETAINED{false};

// Note: We could someday use this to generate client ID's to run
//    tests in parallel, keeping unique client ID's for each test
/*
static inline std::string test_client_id() {
    auto s = Catch::getResultCapture().getCurrentTestName();
    std::replace(s.begin(), s.end(), ' ', '_');
    return s.substr(0,22);
}
*/

//----------------------------------------------------------------------
// Test constructors async_client::async_client()
//----------------------------------------------------------------------

TEST_CASE("async_client user constructor 2 string_args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};

    REQUIRE(GOOD_SERVER_URI == cli.get_server_uri());
    REQUIRE(CLIENT_ID == cli.get_client_id());
}

TEST_CASE("async_client user constructor 2 string args failure", "[client]")
{
    int return_code = MQTTASYNC_SUCCESS;
    try {
        async_client cli{BAD_SERVER_URI, CLIENT_ID};
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_BAD_PROTOCOL == return_code);
}

TEST_CASE("async_client user constructor 3 string args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID, PERSISTENCE_DIR};

    REQUIRE(GOOD_SERVER_URI == cli.get_server_uri());
    REQUIRE(CLIENT_ID == cli.get_client_id());
}

TEST_CASE("async_client user constructor 3 args", "[client]")
{
    mock_persistence cp;
    async_client cli{GOOD_SERVER_URI, CLIENT_ID, &cp};

    REQUIRE(GOOD_SERVER_URI == cli.get_server_uri());
    REQUIRE(CLIENT_ID == cli.get_client_id());

    async_client cli_no_persistence{GOOD_SERVER_URI, CLIENT_ID, nullptr};

    REQUIRE(GOOD_SERVER_URI == cli_no_persistence.get_server_uri());
    REQUIRE(CLIENT_ID == cli_no_persistence.get_client_id());
}

//----------------------------------------------------------------------
// Test async_client::connect()
//----------------------------------------------------------------------

TEST_CASE("async_client connect 0 arg", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    try {
        token_ptr conn_tok = cli.connect();
        REQUIRE(conn_tok);
        conn_tok->wait();
        REQUIRE(cli.is_connected());
    }
    catch (const std::exception& exc) {
        FAIL(std::string("Connection failure: ") + exc.what());
    }
}

TEST_CASE("async_client connect 1 arg", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    connect_options co;
    token_ptr conn_tok{cli.connect(co)};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());
}

TEST_CASE("async_client connect 1 arg failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok;  //{ nullptr };
    connect_options co;
    will_options wo;
    wo.set_qos(BAD_QOS);  // Invalid QoS causes connection failure
    co.set_will(wo);
    int return_code = MQTTASYNC_SUCCESS;
    try {
        conn_tok = cli.connect(co);
        REQUIRE(conn_tok);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(nullptr == conn_tok);
    REQUIRE(!cli.is_connected());
    REQUIRE(MQTTASYNC_BAD_QOS == return_code);
}

TEST_CASE("async_client connect 2 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mock_action_listener listener;
    token_ptr conn_tok{cli.connect(&CONTEXT, listener)};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());
    REQUIRE(CONTEXT == *static_cast<int*>(conn_tok->get_user_context()));
    REQUIRE(listener.succeeded());
}

TEST_CASE("async_client connect 3 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    connect_options co;
    mock_action_listener listener;
    token_ptr conn_tok{cli.connect(co, &CONTEXT, listener)};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());
    REQUIRE(CONTEXT == *static_cast<int*>(conn_tok->get_user_context()));
    REQUIRE(listener.succeeded());
}

TEST_CASE("async_client connect 3 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok;  //{ nullptr };
    connect_options co;
    will_options wo;
    wo.set_qos(BAD_QOS);  // Invalid QoS causes connection failure
    co.set_will(wo);
    mock_action_listener listener;
    int reasonCode = MQTTASYNC_SUCCESS;
    try {
        conn_tok = cli.connect(co, &CONTEXT, listener);
        REQUIRE(conn_tok);
        conn_tok->wait();
    }
    catch (mqtt::exception& ex) {
        reasonCode = ex.get_return_code();
    }
    REQUIRE(nullptr == conn_tok);
    REQUIRE(!cli.is_connected());
    REQUIRE(MQTTASYNC_BAD_QOS == reasonCode);
    // TODO Why listener.on_failure() is not called?
    // REQUIRE(listener.failed());
}

// An improperly initialized SSL connect request should fail gracefully
TEST_CASE("async_client connect uninitialized ssl", "[client]")
{
    int reasonCode = MQTTASYNC_SUCCESS;
    try {
        // Compiled against a non-SSL library should throw here.
        async_client cli{GOOD_SSL_SERVER_URI, CLIENT_ID};

        connect_options opts;
        opts.set_keep_alive_interval(10);
        opts.set_clean_session(true);
        // Note that we're not setting SSL options.

        token_ptr tok;

        // Compiled against the SSL library should throw here
        tok = cli.connect(opts);
        tok->wait();
    }
    catch (mqtt::exception& ex) {
        reasonCode = ex.get_return_code();
    }
    REQUIRE(reasonCode != MQTTASYNC_SUCCESS);
}

//----------------------------------------------------------------------
// Test async_client::disconnect()
//----------------------------------------------------------------------

TEST_CASE("async_client disconnect 0 arg", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client disconnect 1 arg", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    token_ptr disconn_tok{cli.disconnect(0)};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client disconnect 1 arg failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr disconn_tok;  //{ nullptr };
    int return_code = MQTTASYNC_SUCCESS;
    try {
        disconn_tok = cli.disconnect(0);
        REQUIRE(disconn_tok);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(!cli.is_connected());
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client disconnect 2 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    mock_action_listener listener;
    token_ptr disconn_tok{cli.disconnect(&CONTEXT, listener)};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
    REQUIRE(CONTEXT == *static_cast<int*>(disconn_tok->get_user_context()));
}

TEST_CASE("async_client disconnect 3 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    mock_action_listener listener;
    token_ptr disconn_tok{cli.disconnect(0, &CONTEXT, listener)};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
    REQUIRE(CONTEXT == *static_cast<int*>(disconn_tok->get_user_context()));
}

TEST_CASE("async_client disconnect 3 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr disconn_tok;  //{ nullptr };
    mock_action_listener listener;
    int return_code = MQTTASYNC_SUCCESS;
    try {
        disconn_tok = cli.disconnect(0, &CONTEXT, listener);
        REQUIRE(disconn_tok);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(!cli.is_connected());
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

//----------------------------------------------------------------------
// Test async_client::get_pending_delivery_token()
//----------------------------------------------------------------------

TEST_CASE("async_client get pending delivery token", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    REQUIRE(0 == GOOD_QOS_COLL[0]);
    REQUIRE(1 == GOOD_QOS_COLL[1]);
    REQUIRE(2 == GOOD_QOS_COLL[2]);

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    // NOTE: async_client::publish() is the only method that adds
    // delivery_token via async_client::add_token(delivery_token_ptr tok).
    // The other functions add token async_client::add_token(token_ptr tok).

    delivery_token_ptr token_pub;      // { nullptr };
    delivery_token_ptr token_pending;  // { nullptr };

    // NOTE: message IDs are 16-bit numbers sequentially incremented, from
    // 1 to 65535 (MAX_MSG_ID). See MQTTAsync_assignMsgId() at Paho MQTT C.
    int message_id = 1;

    // NOTE: All of the MQTT messages that require a response/acknowledge
    // should have a non-zero 16-bit message ID. This mainly applies to a
    // message with QOS=1 or QOS=2. The C++ library keeps a collection of
    // pointers to token objects for all of these messages that are in
    // flight. When the acknowledge comes back from the broker, the C++
    // library can look up the token from the msgID and signal it, indicating
    // completion.

    // Messages with QOS=2 are kept by the library
    message_ptr msg2{message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[2], RETAINED)};
    token_pub = cli.publish(msg2);
    REQUIRE(token_pub);
    token_pending = cli.get_pending_delivery_token(message_id++);
    REQUIRE(token_pending);

    // Messages with QOS=1 are kept by the library
    message_ptr msg1{message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[1], RETAINED)};
    token_pub = cli.publish(msg1);
    REQUIRE(token_pub);
    token_pending = cli.get_pending_delivery_token(message_id++);
    REQUIRE(token_pending);

    // NOTE: Messages with QOS=0 are fire-and-forget. These just get sent
    // to the broker without any tracking. Their tokens are signaled as
    // "complete" in the send function (by the calling thread).  So, as
    // soon as send returns, the message is considered completed. These
    // have a msgID that is always zero.

    // Messages with QOS=0 are NOT kept by the library
    message_ptr msg0{message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[0], RETAINED)};
    token_pub = cli.publish(msg0);
    REQUIRE(token_pub);
    token_pending = cli.get_pending_delivery_token(message_id++);
    REQUIRE(!token_pending);

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client get pending delivery tokens", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    REQUIRE(0 == GOOD_QOS_COLL[0]);
    REQUIRE(1 == GOOD_QOS_COLL[1]);
    REQUIRE(2 == GOOD_QOS_COLL[2]);

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    delivery_token_ptr token_pub;  // { nullptr };

    // NOTE: async_client::publish() is the only method that adds
    // delivery_token via async_client::add_token(delivery_token_ptr tok).
    // The other functions add token async_client::add_token(token_ptr tok).

    // Messages with QOS=0 are NOT kept by the library
    message_ptr msg0{message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[0], RETAINED)};
    token_pub = cli.publish(msg0);
    REQUIRE(token_pub);

    // Messages with QOS=1 are kept by the library
    message_ptr msg1{message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[1], RETAINED)};
    token_pub = cli.publish(msg1);
    REQUIRE(token_pub);

    // Messages with QOS=2 are kept by the library
    message_ptr msg2{message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[2], RETAINED)};
    token_pub = cli.publish(msg2);
    REQUIRE(token_pub);

    // NOTE: Only tokens for messages with QOS=1 and QOS=2 are kept. That's
    // why the vector's size does not account for QOS=0 message tokens
    std::vector<delivery_token_ptr> tokens_pending{cli.get_pending_delivery_tokens()};
    REQUIRE(2 == static_cast<int>(tokens_pending.size()));

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

//----------------------------------------------------------------------
// Test async_client::publish()
//----------------------------------------------------------------------

TEST_CASE("async_client publish 2 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    message_ptr msg{message::create(TOPIC, PAYLOAD)};
    delivery_token_ptr token_pub{cli.publish(msg)};
    REQUIRE(token_pub);
    token_pub->wait_for(TIMEOUT);

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client publish 2 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        message_ptr msg{message::create(TOPIC, PAYLOAD)};
        delivery_token_ptr token_pub{cli.publish(msg)};
        REQUIRE(token_pub);
        token_pub->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client publish 4 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    message_ptr msg{message::create(TOPIC, PAYLOAD)};
    mock_action_listener listener;
    delivery_token_ptr token_pub{cli.publish(msg, &CONTEXT, listener)};
    REQUIRE(token_pub);
    token_pub->wait_for(TIMEOUT);
    REQUIRE(CONTEXT == *static_cast<int*>(token_pub->get_user_context()));

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client publish 4 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        message_ptr msg{message::create(TOPIC, PAYLOAD)};
        mock_action_listener listener;
        delivery_token_ptr token_pub{cli.publish(msg, &CONTEXT, listener)};
        REQUIRE(token_pub);
        token_pub->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client publish 5 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    const void* payload{PAYLOAD.data()};
    const size_t payload_size{PAYLOAD.size()};
    delivery_token_ptr token_pub{
        cli.publish(TOPIC, payload, payload_size, GOOD_QOS, RETAINED)
    };
    REQUIRE(token_pub);
    token_pub->wait_for(TIMEOUT);

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client publish 7 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    const void* payload{PAYLOAD.c_str()};
    const size_t payload_size{PAYLOAD.size()};
    mock_action_listener listener;
    delivery_token_ptr token_pub{
        cli.publish(TOPIC, payload, payload_size, GOOD_QOS, RETAINED, &CONTEXT, listener)
    };
    REQUIRE(token_pub);
    token_pub->wait_for(TIMEOUT);
    REQUIRE(CONTEXT == *static_cast<int*>(token_pub->get_user_context()));

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

//----------------------------------------------------------------------
// Test async_client::set_callback()
//----------------------------------------------------------------------

TEST_CASE("async_client set callback", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mock_callback cb;
    cli.set_callback(cb);

    // REQUIRE(cb.delivery_complete_called);
}

//----------------------------------------------------------------------
// Test async_client::subscribe()
//----------------------------------------------------------------------

TEST_CASE("async_client subscribe single topic 2 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    token_ptr sub_tok{cli.subscribe(TOPIC, GOOD_QOS)};
    REQUIRE(sub_tok);
    sub_tok->wait_for(TIMEOUT);

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client subscribe single topic 2 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        token_ptr sub_tok{cli.subscribe(TOPIC, BAD_QOS)};
        REQUIRE(sub_tok);
        sub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client subscribe single topic 4 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    mock_action_listener listener;
    token_ptr sub_tok{cli.subscribe(TOPIC, GOOD_QOS, &CONTEXT, listener)};
    REQUIRE(sub_tok);
    sub_tok->wait_for(TIMEOUT);
    REQUIRE(CONTEXT == *static_cast<int*>(sub_tok->get_user_context()));

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client subscribe single topic 4 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        mock_action_listener listener;
        token_ptr sub_tok{cli.subscribe(TOPIC, BAD_QOS, &CONTEXT, listener)};
        REQUIRE(sub_tok);
        sub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client subscribe many topics 2 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    cli.connect()->wait();
    REQUIRE(cli.is_connected());

    try {
        cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL)->wait_for(TIMEOUT);
    }
    catch (const mqtt::exception& exc) {
        FAIL(exc.what());
    }

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

// There was an odd failure when subscribe_many was given a single topic.
TEST_CASE("async_client subscribe many topics 2 args_single", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    cli.connect()->wait();
    REQUIRE(cli.is_connected());

    mqtt::const_string_collection_ptr TOPIC_1_COLL{
        mqtt::string_collection::create({"TOPIC0"})
    };
    iasync_client::qos_collection GOOD_QOS_1_COLL{0};
    try {
        cli.subscribe(TOPIC_1_COLL, GOOD_QOS_1_COLL)->wait_for(TIMEOUT);
    }
    catch (const mqtt::exception& exc) {
        FAIL(exc.what());
    }

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client subscribe many topics 2 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    try {
        token_ptr sub_tok{cli.subscribe(TOPIC_COLL, BAD_QOS_COLL)};
        REQUIRE(sub_tok);
        sub_tok->wait_for(TIMEOUT);
    }
    catch (const mqtt::exception& /*ex*/) {
        // REQUIRE(MQTTASYNC_BAD_QOS == ex.get_return_code());
    }

    int return_code = MQTTASYNC_SUCCESS;
    try {
        token_ptr sub_tok{cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL)};
        REQUIRE(sub_tok);
        sub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client subscribe many topics 4 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    mock_action_listener listener;
    token_ptr sub_tok{cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL, &CONTEXT, listener)};
    REQUIRE(sub_tok);
    sub_tok->wait_for(TIMEOUT);
    REQUIRE(CONTEXT == *static_cast<int*>(sub_tok->get_user_context()));

    token_ptr disconn_tok{cli.disconnect()};
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client subscribe many topics 4 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mock_action_listener listener;

    try {
        cli.subscribe(TOPIC_COLL, BAD_QOS_COLL, &CONTEXT, listener)->wait_for(TIMEOUT);
    }
    catch (const mqtt::exception& /*ex*/) {
        // REQUIRE(MQTTASYNC_BAD_QOS == ex.get_return_code());
    }

    int return_code = MQTTASYNC_SUCCESS;
    try {
        token_ptr sub_tok{cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL, &CONTEXT, listener)};
        REQUIRE(sub_tok);
        sub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

//----------------------------------------------------------------------
// Test async_client::unsubscribe()
//----------------------------------------------------------------------

TEST_CASE("async_client unsubscribe single topic 1 arg", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok = cli.connect();
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    token_ptr sub_tok = cli.subscribe(TOPIC, 1);
    REQUIRE(sub_tok);
    sub_tok->wait_for(TIMEOUT);

    token_ptr unsub_tok = cli.unsubscribe(TOPIC);
    REQUIRE(unsub_tok);
    sub_tok->wait_for(TIMEOUT);

    token_ptr disconn_tok = cli.disconnect();
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client unsubscribe single topic 1 arg failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        token_ptr unsub_tok{cli.unsubscribe(TOPIC)};
        REQUIRE(unsub_tok);
        unsub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client unsubscribe single topic 3 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok = cli.connect();
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    token_ptr sub_tok = cli.subscribe(TOPIC, 1);
    REQUIRE(sub_tok);
    sub_tok->wait_for(TIMEOUT);

    mock_action_listener listener;
    token_ptr unsub_tok = cli.unsubscribe(TOPIC, &CONTEXT, listener);
    REQUIRE(unsub_tok);
    unsub_tok->wait_for(TIMEOUT);
    REQUIRE(CONTEXT == *static_cast<int*>(unsub_tok->get_user_context()));

    token_ptr disconn_tok = cli.disconnect();
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client unsubscribe single topic 3 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        mock_action_listener listener;
        token_ptr unsub_tok{cli.unsubscribe(TOPIC, &CONTEXT, listener)};
        REQUIRE(unsub_tok);
        unsub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client unsubscribe many topics 1 arg", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok = cli.connect();
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    token_ptr sub_tok = cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL);
    REQUIRE(sub_tok);
    sub_tok->wait_for(TIMEOUT);

    token_ptr unsub_tok = cli.unsubscribe(TOPIC_COLL);
    REQUIRE(unsub_tok);
    unsub_tok->wait_for(TIMEOUT);

    token_ptr disconn_tok = cli.disconnect();
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client unsubscribe many topics 1 arg_failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    int return_code = MQTTASYNC_SUCCESS;
    try {
        token_ptr unsub_tok{cli.unsubscribe(TOPIC_COLL)};
        REQUIRE(unsub_tok);
        unsub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client unsubscribe many topics 3 args", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    token_ptr conn_tok = cli.connect();
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());

    token_ptr sub_tok = cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL);
    REQUIRE(sub_tok);
    sub_tok->wait_for(TIMEOUT);

    mock_action_listener listener;
    token_ptr unsub_tok = cli.unsubscribe(TOPIC_COLL, &CONTEXT, listener);
    REQUIRE(unsub_tok);
    unsub_tok->wait_for(TIMEOUT);
    REQUIRE(CONTEXT == *static_cast<int*>(unsub_tok->get_user_context()));

    token_ptr disconn_tok = cli.disconnect();
    REQUIRE(disconn_tok);
    disconn_tok->wait();
    REQUIRE(!cli.is_connected());
}

TEST_CASE("async_client unsubscribe many topics 3 args failure", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    REQUIRE(!cli.is_connected());

    mock_action_listener listener;
    int return_code = MQTTASYNC_SUCCESS;
    try {
        token_ptr unsub_tok{cli.unsubscribe(TOPIC_COLL, &CONTEXT, listener)};
        REQUIRE(unsub_tok);
        unsub_tok->wait_for(TIMEOUT);
    }
    catch (mqtt::exception& ex) {
        return_code = ex.get_return_code();
    }
    REQUIRE(MQTTASYNC_DISCONNECTED == return_code);
}

TEST_CASE("async_client consumer timeout", "[client]")
{
    // This just compiling shows #343 fixed.
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    cli.start_consuming();
    cli.try_consume_message_until(std::chrono::steady_clock::now());
}

TEST_CASE("async_client consumer queue size", "[client]")
{
    async_client cli{GOOD_SERVER_URI, CLIENT_ID};
    cli.start_consuming();
    REQUIRE(0 == cli.consumer_queue_size());

    token_ptr conn_tok{cli.connect()};
    REQUIRE(conn_tok);
    conn_tok->wait();
    REQUIRE(cli.is_connected());
    // expect connected_event to be in the queue now
    REQUIRE(1 == cli.consumer_queue_size());
    event e;
    REQUIRE(cli.try_consume_event(&e));
    REQUIRE(0 == cli.consumer_queue_size());

    cli.stop_consuming();
    cli.disconnect()->wait();
}
