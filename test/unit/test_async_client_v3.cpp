// async_client_v3_test.h
//
// Unit tests for the MQTT v3 async_client class in the Paho MQTT C++
// library.

/*******************************************************************************
 * Copyright (c) 2017 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
 * Copyright (c) 2019 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi - updates
 *******************************************************************************/

#ifndef __mqtt_async_client_v3_test_h
#define __mqtt_async_client_v3_test_h

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#include <stdexcept>
#include <vector>

#include "dummy_action_listener.h"
#include "dummy_callback.h"
#include "dummy_client_persistence.h"
#include "mqtt/async_client.h"
#include "mqtt/iasync_client.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

class async_client_v3_test : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(async_client_test);

    CPPUNIT_TEST(test_user_constructor_2_string_args);
    CPPUNIT_TEST(test_user_constructor_3_string_args);
    CPPUNIT_TEST(test_user_constructor_3_args);

    CPPUNIT_TEST(test_connect_0_arg);
    CPPUNIT_TEST(test_connect_1_arg);
    CPPUNIT_TEST(test_connect_1_arg_failure);
    CPPUNIT_TEST(test_connect_2_args);
    CPPUNIT_TEST(test_connect_3_args);
    CPPUNIT_TEST(test_connect_3_args_failure);
    CPPUNIT_TEST(test_connect_uninitialized_ssl);

    CPPUNIT_TEST(test_disconnect_0_arg);
    CPPUNIT_TEST(test_disconnect_1_arg);
    CPPUNIT_TEST(test_disconnect_1_arg_failure);
    CPPUNIT_TEST(test_disconnect_2_args);
    CPPUNIT_TEST(test_disconnect_3_args);
    CPPUNIT_TEST(test_disconnect_3_args_failure);

    CPPUNIT_TEST(test_get_pending_delivery_token);
    CPPUNIT_TEST(test_get_pending_delivery_tokens);

    CPPUNIT_TEST(test_publish_2_args);
    CPPUNIT_TEST(test_publish_2_args_failure);
    CPPUNIT_TEST(test_publish_4_args);
    CPPUNIT_TEST(test_publish_4_args_failure);
    CPPUNIT_TEST(test_publish_5_args);
    CPPUNIT_TEST(test_publish_7_args);

    CPPUNIT_TEST(test_set_callback);

    CPPUNIT_TEST(test_subscribe_single_topic_2_args);
    CPPUNIT_TEST(test_subscribe_single_topic_2_args_failure);
    CPPUNIT_TEST(test_subscribe_single_topic_4_args);
    CPPUNIT_TEST(test_subscribe_single_topic_4_args_failure);
    CPPUNIT_TEST(test_subscribe_many_topics_2_args);
    CPPUNIT_TEST(test_subscribe_many_topics_2_args_failure);
    CPPUNIT_TEST(test_subscribe_many_topics_4_args);
    CPPUNIT_TEST(test_subscribe_many_topics_4_args_failure);

    CPPUNIT_TEST(test_unsubscribe_single_topic_1_arg);
    CPPUNIT_TEST(test_unsubscribe_single_topic_1_arg_failure);
    CPPUNIT_TEST(test_unsubscribe_single_topic_3_args);
    CPPUNIT_TEST(test_unsubscribe_single_topic_3_args_failure);
    CPPUNIT_TEST(test_unsubscribe_many_topics_1_arg);
    CPPUNIT_TEST(test_unsubscribe_many_topics_1_arg_failure);
    CPPUNIT_TEST(test_unsubscribe_many_topics_3_args);
    CPPUNIT_TEST(test_unsubscribe_many_topics_3_args_failure);

    CPPUNIT_TEST_SUITE_END();

// NOTE: This test case requires network access. It uses one of
//  	 the public available MQTT brokers
#if defined(TEST_EXTERNAL_SERVER)
    const std::string GOOD_SERVER_URI{"tcp://mqtt.eclipseprojects.io:1883"};
#else
    const std::string GOOD_SERVER_URI{"tcp://localhost:1883"};
    const std::string GOOD_SSL_SERVER_URI{"ssl://localhost:18885"};
#endif
    const std::string BAD_SERVER_URI{"one://invalid.address"};
    const std::string CLIENT_ID{""};  // { "async_client_unit_test" };
    const std::string PERSISTENCE_DIR{"/tmp"};
    const std::string TOPIC{"TOPIC"};
    const int GOOD_QOS{0};
    const int BAD_QOS{3};
    const_string_collection_ptr TOPIC_COLL{
        string_collection::create({"TOPIC0", "TOPIC1", "TOPIC2"})
    };
    mqtt::iasync_client::qos_collection GOOD_QOS_COLL{0, 1, 2};
    mqtt::iasync_client::qos_collection BAD_QOS_COLL{BAD_QOS, 1, 2};
    const std::string PAYLOAD{"PAYLOAD"};
    const int TIMEOUT{1000};
    int CONTEXT{4};
    mqtt::test::dummy_action_listener listener;
    const bool RETAINED{false};

public:
    void setUp() {}
    void tearDown() {}

    //----------------------------------------------------------------------
    // Test constructors async_client::async_client()
    //----------------------------------------------------------------------

    void test_user_constructor_2_string_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};

        CPPUNIT_ASSERT_EQUAL(GOOD_SERVER_URI, cli.get_server_uri());
        CPPUNIT_ASSERT_EQUAL(CLIENT_ID, cli.get_client_id());
    }

    void test_user_constructor_2_string_args_failure()
    {
        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::async_client cli{BAD_SERVER_URI, CLIENT_ID};
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_BAD_PROTOCOL, reason_code);
    }

    void test_user_constructor_3_string_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID, PERSISTENCE_DIR};

        CPPUNIT_ASSERT_EQUAL(GOOD_SERVER_URI, cli.get_server_uri());
        CPPUNIT_ASSERT_EQUAL(CLIENT_ID, cli.get_client_id());
    }

    void test_user_constructor_3_args()
    {
        mqtt::test::dummy_client_persistence cp;
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID, &cp};

        CPPUNIT_ASSERT_EQUAL(GOOD_SERVER_URI, cli.get_server_uri());
        CPPUNIT_ASSERT_EQUAL(CLIENT_ID, cli.get_client_id());

        mqtt::async_client cli_no_persistence{GOOD_SERVER_URI, CLIENT_ID, nullptr};

        CPPUNIT_ASSERT_EQUAL(GOOD_SERVER_URI, cli_no_persistence.get_server_uri());
        CPPUNIT_ASSERT_EQUAL(CLIENT_ID, cli_no_persistence.get_client_id());
    }

    //----------------------------------------------------------------------
    // Test async_client::connect()
    //----------------------------------------------------------------------

    void test_connect_0_arg()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        try {
            mqtt::token_ptr token_conn = cli.connect();
            CPPUNIT_ASSERT(token_conn);
            token_conn->wait();
            CPPUNIT_ASSERT(cli.is_connected());
        }
        catch (const std::exception& exc) {
            CPPUNIT_FAIL(std::string("Connection failure: ") + exc.what());
        }
    }

    void test_connect_1_arg()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::connect_options co;
        mqtt::token_ptr token_conn{cli.connect(co)};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());
    }

    void test_connect_1_arg_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn;  //{ nullptr };
        mqtt::connect_options co;
        mqtt::will_options wo;
        wo.set_qos(BAD_QOS);  // Invalid QoS causes connection failure
        co.set_will(wo);
        int reason_code = MQTTASYNC_SUCCESS;
        try {
            token_conn = cli.connect(co);
            CPPUNIT_ASSERT(token_conn);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT(nullptr == token_conn);
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_BAD_QOS, reason_code);
    }

    void test_connect_2_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_conn{cli.connect(&CONTEXT, listener)};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_conn->get_user_context()));
        CPPUNIT_ASSERT(listener.on_success_called);
    }

    void test_connect_3_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::connect_options co;
        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_conn{cli.connect(co, &CONTEXT, listener)};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_conn->get_user_context()));
        CPPUNIT_ASSERT(listener.on_success_called);
    }

    void test_connect_3_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn;  //{ nullptr };
        mqtt::connect_options co;
        mqtt::will_options wo;
        wo.set_qos(BAD_QOS);  // Invalid QoS causes connection failure
        co.set_will(wo);
        mqtt::test::dummy_action_listener listener;
        int reasonCode = MQTTASYNC_SUCCESS;
        try {
            token_conn = cli.connect(co, &CONTEXT, listener);
            CPPUNIT_ASSERT(token_conn);
            token_conn->wait();
        }
        catch (mqtt::exception& ex) {
            reasonCode = ex.get_reason_code();
        }
        CPPUNIT_ASSERT(nullptr == token_conn);
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_BAD_QOS, reasonCode);
        // TODO Why listener.on_failure() is not called?
        // CPPUNIT_ASSERT(listener.on_failure_called);
    }

    // An improperly initialized SSL connect request should fail gracefully
    void test_connect_uninitialized_ssl()
    {
        int reasonCode = MQTTASYNC_SUCCESS;
        try {
            // Compiled against a non-SSL library should throw here.
            mqtt::async_client cli{GOOD_SSL_SERVER_URI, CLIENT_ID};

            mqtt::connect_options opts;
            opts.set_keep_alive_interval(10);
            opts.set_clean_session(true);
            // Note that we're not setting SSL options.

            mqtt::token_ptr tok;

            // Compiled against the SSL library should throw here
            tok = cli.connect(opts);
            tok->wait();
        }
        catch (mqtt::exception& ex) {
            reasonCode = ex.get_reason_code();
        }
        CPPUNIT_ASSERT(reasonCode != MQTTASYNC_SUCCESS);
    }

    //----------------------------------------------------------------------
    // Test async_client::disconnect()
    //----------------------------------------------------------------------

    void test_disconnect_0_arg()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_disconnect_1_arg()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::token_ptr token_disconn{cli.disconnect(0)};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_disconnect_1_arg_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_disconn;  //{ nullptr };
        int reason_code = MQTTASYNC_SUCCESS;
        try {
            token_disconn = cli.disconnect(0);
            CPPUNIT_ASSERT(token_disconn);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_disconnect_2_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_disconn{cli.disconnect(&CONTEXT, listener)};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_disconn->get_user_context()));
    }

    void test_disconnect_3_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_disconn{cli.disconnect(0, &CONTEXT, listener)};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_disconn->get_user_context()));
    }

    void test_disconnect_3_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_disconn;  //{ nullptr };
        mqtt::test::dummy_action_listener listener;
        int reason_code = MQTTASYNC_SUCCESS;
        try {
            token_disconn = cli.disconnect(0, &CONTEXT, listener);
            CPPUNIT_ASSERT(token_disconn);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    //----------------------------------------------------------------------
    // Test async_client::get_pending_delivery_token()
    //----------------------------------------------------------------------

    void test_get_pending_delivery_token()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        CPPUNIT_ASSERT_EQUAL(0, GOOD_QOS_COLL[0]);
        CPPUNIT_ASSERT_EQUAL(1, GOOD_QOS_COLL[1]);
        CPPUNIT_ASSERT_EQUAL(2, GOOD_QOS_COLL[2]);

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        // NOTE: async_client::publish() is the only method that adds
        // delivery_token via async_client::add_token(delivery_token_ptr tok).
        // The other functions add token async_client::add_token(token_ptr tok).

        mqtt::delivery_token_ptr token_pub;      // { nullptr };
        mqtt::delivery_token_ptr token_pending;  // { nullptr };

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
        mqtt::message_ptr msg2{
            mqtt::message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[2], RETAINED)
        };
        token_pub = cli.publish(msg2);
        CPPUNIT_ASSERT(token_pub);
        token_pending = cli.get_pending_delivery_token(message_id++);
        CPPUNIT_ASSERT(token_pending);

        // Messages with QOS=1 are kept by the library
        mqtt::message_ptr msg1{
            mqtt::message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[1], RETAINED)
        };
        token_pub = cli.publish(msg1);
        CPPUNIT_ASSERT(token_pub);
        token_pending = cli.get_pending_delivery_token(message_id++);
        CPPUNIT_ASSERT(token_pending);

        // NOTE: Messages with QOS=0 are fire-and-forget. These just get sent
        // to the broker without any tracking. Their tokens are signaled as
        // "complete" in the send function (by the calling thread).  So, as
        // soon as send returns, the message is considered completed. These
        // have a msgID that is always zero.

        // Messages with QOS=0 are NOT kept by the library
        mqtt::message_ptr msg0{
            mqtt::message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[0], RETAINED)
        };
        token_pub = cli.publish(msg0);
        CPPUNIT_ASSERT(token_pub);
        token_pending = cli.get_pending_delivery_token(message_id++);
        CPPUNIT_ASSERT(!token_pending);

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_get_pending_delivery_tokens()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        CPPUNIT_ASSERT_EQUAL(0, GOOD_QOS_COLL[0]);
        CPPUNIT_ASSERT_EQUAL(1, GOOD_QOS_COLL[1]);
        CPPUNIT_ASSERT_EQUAL(2, GOOD_QOS_COLL[2]);

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::delivery_token_ptr token_pub;  // { nullptr };

        // NOTE: async_client::publish() is the only method that adds
        // delivery_token via async_client::add_token(delivery_token_ptr tok).
        // The other functions add token async_client::add_token(token_ptr tok).

        // Messages with QOS=0 are NOT kept by the library
        mqtt::message_ptr msg0{
            mqtt::message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[0], RETAINED)
        };
        token_pub = cli.publish(msg0);
        CPPUNIT_ASSERT(token_pub);

        // Messages with QOS=1 are kept by the library
        mqtt::message_ptr msg1{
            mqtt::message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[1], RETAINED)
        };
        token_pub = cli.publish(msg1);
        CPPUNIT_ASSERT(token_pub);

        // Messages with QOS=2 are kept by the library
        mqtt::message_ptr msg2{
            mqtt::message::create(TOPIC, PAYLOAD, GOOD_QOS_COLL[2], RETAINED)
        };
        token_pub = cli.publish(msg2);
        CPPUNIT_ASSERT(token_pub);

        // NOTE: Only tokens for messages with QOS=1 and QOS=2 are kept. That's
        // why the vector's size does not account for QOS=0 message tokens
        std::vector<mqtt::delivery_token_ptr> tokens_pending{cli.get_pending_delivery_tokens()
        };
        CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(tokens_pending.size()));

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    //----------------------------------------------------------------------
    // Test async_client::publish()
    //----------------------------------------------------------------------

    void test_publish_2_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::message_ptr msg{mqtt::message::create(TOPIC, PAYLOAD)};
        mqtt::delivery_token_ptr token_pub{cli.publish(msg)};
        CPPUNIT_ASSERT(token_pub);
        token_pub->wait_for(TIMEOUT);

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_publish_2_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::message_ptr msg{mqtt::message::create(TOPIC, PAYLOAD)};
            mqtt::delivery_token_ptr token_pub{cli.publish(msg)};
            CPPUNIT_ASSERT(token_pub);
            token_pub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_publish_4_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::message_ptr msg{mqtt::message::create(TOPIC, PAYLOAD)};
        mqtt::test::dummy_action_listener listener;
        mqtt::delivery_token_ptr token_pub{cli.publish(msg, &CONTEXT, listener)};
        CPPUNIT_ASSERT(token_pub);
        token_pub->wait_for(TIMEOUT);
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_pub->get_user_context()));

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_publish_4_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::message_ptr msg{mqtt::message::create(TOPIC, PAYLOAD)};
            mqtt::test::dummy_action_listener listener;
            mqtt::delivery_token_ptr token_pub{cli.publish(msg, &CONTEXT, listener)};
            CPPUNIT_ASSERT(token_pub);
            token_pub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_publish_5_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        const void* payload{PAYLOAD.data()};
        const size_t payload_size{PAYLOAD.size()};
        mqtt::delivery_token_ptr token_pub{
            cli.publish(TOPIC, payload, payload_size, GOOD_QOS, RETAINED)
        };
        CPPUNIT_ASSERT(token_pub);
        token_pub->wait_for(TIMEOUT);

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_publish_7_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        const void* payload{PAYLOAD.c_str()};
        const size_t payload_size{PAYLOAD.size()};
        mqtt::test::dummy_action_listener listener;
        mqtt::delivery_token_ptr token_pub{
            cli.publish(TOPIC, payload, payload_size, GOOD_QOS, RETAINED, &CONTEXT, listener)
        };
        CPPUNIT_ASSERT(token_pub);
        token_pub->wait_for(TIMEOUT);
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_pub->get_user_context()));

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    //----------------------------------------------------------------------
    // Test async_client::set_callback()
    //----------------------------------------------------------------------

    void test_set_callback()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::test::dummy_callback cb;
        cli.set_callback(cb);

        // CPPUNIT_ASSERT(cb.delivery_complete_called);
    }

    //----------------------------------------------------------------------
    // Test async_client::subscribe()
    //----------------------------------------------------------------------

    void test_subscribe_single_topic_2_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::token_ptr token_sub{cli.subscribe(TOPIC, GOOD_QOS)};
        CPPUNIT_ASSERT(token_sub);
        token_sub->wait_for(TIMEOUT);

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_subscribe_single_topic_2_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::token_ptr token_sub{cli.subscribe(TOPIC, BAD_QOS)};
            CPPUNIT_ASSERT(token_sub);
            token_sub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_subscribe_single_topic_4_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_sub{cli.subscribe(TOPIC, GOOD_QOS, &CONTEXT, listener)};
        CPPUNIT_ASSERT(token_sub);
        token_sub->wait_for(TIMEOUT);
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_sub->get_user_context()));

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_subscribe_single_topic_4_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::test::dummy_action_listener listener;
            mqtt::token_ptr token_sub{cli.subscribe(TOPIC, BAD_QOS, &CONTEXT, listener)};
            CPPUNIT_ASSERT(token_sub);
            token_sub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_subscribe_many_topics_2_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        cli.connect()->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        try {
            cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL)->wait_for(TIMEOUT);
        }
        catch (const mqtt::exception& exc) {
            CPPUNIT_FAIL(exc.what());
        }

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_subscribe_many_topics_2_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        try {
            mqtt::token_ptr token_sub{cli.subscribe(TOPIC_COLL, BAD_QOS_COLL)};
            CPPUNIT_ASSERT(token_sub);
            token_sub->wait_for(TIMEOUT);
        }
        catch (const mqtt::exception& ex) {
            // CPPUNIT_ASSERT_EQUAL(MQTTASYNC_BAD_QOS, ex.get_reason_code());
        }

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::token_ptr token_sub{cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL)};
            CPPUNIT_ASSERT(token_sub);
            token_sub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_subscribe_many_topics_4_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_sub{cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL, &CONTEXT, listener)
        };
        CPPUNIT_ASSERT(token_sub);
        token_sub->wait_for(TIMEOUT);
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_sub->get_user_context()));

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_subscribe_many_topics_4_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::test::dummy_action_listener listener;

        try {
            cli.subscribe(TOPIC_COLL, BAD_QOS_COLL, &CONTEXT, listener)->wait_for(TIMEOUT);
        }
        catch (const mqtt::exception& ex) {
            // CPPUNIT_ASSERT_EQUAL(MQTTASYNC_BAD_QOS, ex.get_reason_code());
        }

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::token_ptr token_sub{
                cli.subscribe(TOPIC_COLL, GOOD_QOS_COLL, &CONTEXT, listener)
            };
            CPPUNIT_ASSERT(token_sub);
            token_sub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    //----------------------------------------------------------------------
    // Test async_client::unsubscribe()
    //----------------------------------------------------------------------

    void test_unsubscribe_single_topic_1_arg()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC)};
        CPPUNIT_ASSERT(token_unsub);
        token_unsub->wait_for(TIMEOUT);

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_unsubscribe_single_topic_1_arg_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC)};
            CPPUNIT_ASSERT(token_unsub);
            token_unsub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_unsubscribe_single_topic_3_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC, &CONTEXT, listener)};
        CPPUNIT_ASSERT(token_unsub);
        token_unsub->wait_for(TIMEOUT);
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_unsub->get_user_context()));

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_unsubscribe_single_topic_3_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::test::dummy_action_listener listener;
            mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC, &CONTEXT, listener)};
            CPPUNIT_ASSERT(token_unsub);
            token_unsub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_unsubscribe_many_topics_1_arg()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC_COLL)};
        CPPUNIT_ASSERT(token_unsub);
        token_unsub->wait_for(TIMEOUT);

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_unsubscribe_many_topics_1_arg_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC_COLL)};
            CPPUNIT_ASSERT(token_unsub);
            token_unsub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }

    void test_unsubscribe_many_topics_3_args()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::token_ptr token_conn{cli.connect()};
        CPPUNIT_ASSERT(token_conn);
        token_conn->wait();
        CPPUNIT_ASSERT(cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC_COLL, &CONTEXT, listener)};
        CPPUNIT_ASSERT(token_unsub);
        token_unsub->wait_for(TIMEOUT);
        CPPUNIT_ASSERT_EQUAL(CONTEXT, *static_cast<int*>(token_unsub->get_user_context()));

        mqtt::token_ptr token_disconn{cli.disconnect()};
        CPPUNIT_ASSERT(token_disconn);
        token_disconn->wait();
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());
    }

    void test_unsubscribe_many_topics_3_args_failure()
    {
        mqtt::async_client cli{GOOD_SERVER_URI, CLIENT_ID};
        CPPUNIT_ASSERT_EQUAL(false, cli.is_connected());

        mqtt::test::dummy_action_listener listener;
        int reason_code = MQTTASYNC_SUCCESS;
        try {
            mqtt::token_ptr token_unsub{cli.unsubscribe(TOPIC_COLL, &CONTEXT, listener)};
            CPPUNIT_ASSERT(token_unsub);
            token_unsub->wait_for(TIMEOUT);
        }
        catch (mqtt::exception& ex) {
            reason_code = ex.get_reason_code();
        }
        CPPUNIT_ASSERT_EQUAL(MQTTASYNC_DISCONNECTED, reason_code);
    }
};

/////////////////////////////////////////////////////////////////////////////
// end namespace mqtt
}  // namespace mqtt

#endif  //  __mqtt_async_client_v3_test_h
