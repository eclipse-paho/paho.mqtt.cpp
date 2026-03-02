// test_token.h
//
//  Unit tests for the token class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
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
 *    Guilherme M. Ferreira
 *        - initial implementation and documentation
 *    Frank Pagliughi
 *        - additional tests. Made this test a friend of token.
 *        - updated for token::Type
 *        - Converted to use Catch2
 *******************************************************************************/

#define UNIT_TESTS

#include <cstring>

#include "catch2_version.h"
#include "mock_action_listener.h"
#include "mock_async_client.h"
#include "mqtt/token.h"

using namespace mqtt;

using milliseconds = std::chrono::milliseconds;
using steady_clock = std::chrono::steady_clock;

static mock_async_client cli;

static constexpr token::Type TYPE = token::Type::CONNECT;

// ----------------------------------------------------------------------
// Test user constructor (iasync_client)
// ----------------------------------------------------------------------

TEST_CASE("token user constructor client", "[token]")
{
    mqtt::token tok{TYPE, cli};
    REQUIRE(0 == tok.get_message_id());
    REQUIRE(dynamic_cast<mqtt::iasync_client*>(&cli) == tok.get_client());
    REQUIRE(nullptr == tok.get_user_context());
    REQUIRE(nullptr == tok.get_action_callback());
    REQUIRE(!tok.is_complete());
    REQUIRE(nullptr == tok.get_topics());
}

// ----------------------------------------------------------------------
// Test user constructor (iasync_client, MQTTAsync_token)
// ----------------------------------------------------------------------

TEST_CASE("token user constructor client token", "[token]")
{
    MQTTAsync_token id{2};
    mqtt::token tok{TYPE, cli, id};
    REQUIRE(id == tok.get_message_id());
    REQUIRE(dynamic_cast<mqtt::iasync_client*>(&cli) == tok.get_client());
    REQUIRE(nullptr == tok.get_user_context());
    REQUIRE(nullptr == tok.get_action_callback());
    REQUIRE(!tok.is_complete());
    REQUIRE(nullptr == tok.get_topics());
}

// ----------------------------------------------------------------------
// Test user constructor (iasync_client, string)
// ----------------------------------------------------------------------

TEST_CASE("token user constructor client string", "[token]")
{
    std::string topic{"topic"};
    mqtt::token tok{TYPE, cli, topic};
    REQUIRE(0 == tok.get_message_id());
    REQUIRE(dynamic_cast<mqtt::iasync_client*>(&cli) == tok.get_client());
    REQUIRE(nullptr == tok.get_user_context());
    REQUIRE(nullptr == tok.get_action_callback());
    REQUIRE(!tok.is_complete());
    REQUIRE(nullptr != tok.get_topics());
    REQUIRE(size_t(1) == tok.get_topics()->size());
    REQUIRE(topic == (*tok.get_topics())[0]);
}

// ----------------------------------------------------------------------
// Test user constructor (iasync_client, vector<string>)
// ----------------------------------------------------------------------

TEST_CASE("token user constructor client vector", "[token]")
{
    auto topics = string_collection::create({"topic1", "topic2"});
    mqtt::token tok{TYPE, cli, topics};
    REQUIRE(0 == tok.get_message_id());
    REQUIRE(dynamic_cast<mqtt::iasync_client*>(&cli) == tok.get_client());
    REQUIRE(static_cast<void*>(nullptr) == tok.get_user_context());
    REQUIRE(static_cast<mqtt::iaction_listener*>(nullptr) == tok.get_action_callback());
    REQUIRE(!tok.is_complete());
    REQUIRE(nullptr != tok.get_topics());
    REQUIRE(size_t(2) == tok.get_topics()->size());
    REQUIRE((*topics)[0] == (*tok.get_topics())[0]);
    REQUIRE((*topics)[1] == (*tok.get_topics())[1]);
}

// ----------------------------------------------------------------------
// Test on success with data
// ----------------------------------------------------------------------

TEST_CASE("token on success with data", "[token]")
{
    mqtt::token tok{TYPE, cli};

    constexpr int MESSAGE_ID = 12;
    MQTTAsync_successData data{};

    data.token = MESSAGE_ID;
    data.alt.connect.serverURI = const_cast<char*>("tcp://some_server.com");

    REQUIRE(!tok.is_complete());
    mock_async_client::succeed(&tok, &data);
    REQUIRE(tok.is_complete());
    REQUIRE(MESSAGE_ID == tok.get_message_id());
}

// ----------------------------------------------------------------------
// Test on success without data
// ----------------------------------------------------------------------

TEST_CASE("token on success without data", "[token]")
{
    mqtt::token tok{TYPE, cli};

    REQUIRE(!tok.is_complete());
    mock_async_client::succeed(&tok, nullptr);
    REQUIRE(tok.is_complete());
}

// ----------------------------------------------------------------------
// Test on failure with data
// ----------------------------------------------------------------------

TEST_CASE("token on failure with data", "[token]")
{
    mqtt::token tok{TYPE, cli};

    REQUIRE(!tok.is_complete());
    constexpr int MESSAGE_ID = 12;
    MQTTAsync_failureData data{};

    data.token = MESSAGE_ID;
    data.code = 13;
    data.message = nullptr;

    mock_async_client::fail(&tok, &data);
    REQUIRE(tok.is_complete());
    REQUIRE(MESSAGE_ID == tok.get_message_id());
}

// ----------------------------------------------------------------------
// Test on failure without data
// ----------------------------------------------------------------------

TEST_CASE("token on failure without data", "[token]")
{
    mqtt::token tok{TYPE, cli};

    REQUIRE(!tok.is_complete());
    mock_async_client::fail(&tok, nullptr);
    REQUIRE(tok.is_complete());
    REQUIRE(0 == tok.get_message_id());
}

// ----------------------------------------------------------------------
// Test set callbacks
// ----------------------------------------------------------------------

TEST_CASE("token action callback", "[token]")
{
    mock_action_listener listener;
    mqtt::token tok{TYPE, cli};
    tok.set_action_callback(listener);
    REQUIRE(dynamic_cast<mqtt::iaction_listener*>(&listener) == tok.get_action_callback());

    REQUIRE(!listener.succeeded());
    mock_async_client::succeed(&tok, nullptr);
    REQUIRE(listener.succeeded());

    REQUIRE(!listener.failed());
    mock_async_client::fail(&tok, nullptr);
    REQUIRE(listener.failed());
}

// ----------------------------------------------------------------------
// Test wait for completion on success case
// All wait's should succeed immediately on successful completion.
// ----------------------------------------------------------------------

TEST_CASE("token wait success", "[token]")
{
    const auto TIMEOUT = milliseconds(10);

    mqtt::token tok{TYPE, cli};

    // NOTE: Make sure the complete flag is already true and the return
    // code (rc) is MQTTASYNC_SUCCESS, so the token::wait()
    // returns immediately. Otherwise we will get stuck in a single thread
    // that can't change the complete flag.
    mock_async_client::succeed(&tok, nullptr);
    REQUIRE(tok.is_complete());

    // A wait does not reset the "complete" flag.

    try {
        tok.wait();
        REQUIRE(tok.is_complete());
    }
    catch (...) {
        FAIL("token::wait() should not throw on success");
    }

    // try_wait()
    try {
        REQUIRE(tok.try_wait());
        REQUIRE(tok.is_complete());
    }
    catch (...) {
        FAIL("token::wait() should not throw on success");
    }

    // wait_for()
    try {
        REQUIRE(tok.wait_for(TIMEOUT));
        REQUIRE(tok.is_complete());
    }
    catch (...) {
        FAIL("token::wait_for() should not throw on success");
    }

    // wait_until()
    const auto TO = steady_clock::now() + TIMEOUT;
    try {
        REQUIRE(tok.wait_until(TO));
        REQUIRE(tok.is_complete());
    }
    catch (...) {
        FAIL("token::wait_until() should not throw on success");
    }
}

// ----------------------------------------------------------------------
// Test wait for completion on failure case
// All wait's should throw if the action failed
// ----------------------------------------------------------------------

TEST_CASE("token wait failure", "[token]")
{
    const auto TIMEOUT = milliseconds(10);

    mqtt::token tok{TYPE, cli};

    // NOTE: Make sure the complete flag is already true and the return
    // code (rc) is MQTTASYNC_FAILURE, so the token::wait()
    // returns immediately. Otherwise we will get stuck in a single thread
    // that can't change the complete flag.
    constexpr int MESSAGE_ID = 12;
    MQTTAsync_failureData data{};

    data.token = MESSAGE_ID;
    data.code = MQTTASYNC_FAILURE;
    data.message = nullptr;

    mock_async_client::fail(&tok, &data);

    REQUIRE(tok.is_complete());
    REQUIRE(MESSAGE_ID == tok.get_message_id());
    REQUIRE(MQTTASYNC_FAILURE == tok.get_return_code());

    try {
        tok.wait();
        FAIL("token::wait() should throw on failure");
    }
    catch (mqtt::exception& ex) {
        REQUIRE(MQTTASYNC_FAILURE == ex.get_return_code());
    }

    try {
        tok.try_wait();
        FAIL("token::try_wait() should throw on failure");
    }
    catch (mqtt::exception& ex) {
        REQUIRE(MQTTASYNC_FAILURE == ex.get_return_code());
    }

    try {
        tok.wait_for(TIMEOUT);
        FAIL("token::wait_for() should throw on failure");
    }
    catch (mqtt::exception& ex) {
        REQUIRE(MQTTASYNC_FAILURE == ex.get_return_code());
    }

    try {
        tok.wait_until(steady_clock::now() + TIMEOUT);
        FAIL("token::wait_until() should throw on failure");
    }
    catch (mqtt::exception& ex) {
        REQUIRE(MQTTASYNC_FAILURE == ex.get_return_code());
    }
}

// ----------------------------------------------------------------------
// Test wait for completion on failure due timeout case
// All waits should return false, but not throw, on a timeout
// ----------------------------------------------------------------------

TEST_CASE("token wait for timeout", "[token]")
{
    const auto TIMEOUT = milliseconds(10);

    mqtt::token tok{TYPE, cli};

    // Test for timeout on non-signaled token.
    REQUIRE(!tok.is_complete());

    // try_wait()
    try {
        REQUIRE(!tok.try_wait());
    }
    catch (...) {
        FAIL("token::try_wait() should not throw");
    }

    // wait_for()
    REQUIRE(!tok.is_complete());
    try {
        REQUIRE(!tok.wait_for(TIMEOUT));
    }
    catch (...) {
        FAIL("token::wait_for() should not throw on timeout");
    }

    // wait_until()
    const auto TO = steady_clock::now() + TIMEOUT;
    REQUIRE(!tok.is_complete());
    try {
        REQUIRE(!tok.wait_until(TO));
    }
    catch (...) {
        FAIL("token::wait_until() should not throw on timeout");
    }
}
