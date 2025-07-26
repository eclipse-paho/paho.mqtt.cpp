// mock_async_client.h
//
// Dummy/mock implementation of mqtt::iasync_client for unit tests.
//

/*******************************************************************************
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
 * Copyright (c) 2020 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Guilherme M. Ferreira - initial implementation
 *    Frank Pagliughi - updated and renamed
 *******************************************************************************/

#ifndef __mqtt_test_mock_async_client_h
#define __mqtt_test_mock_async_client_h

#include <vector>

#include "mqtt/connect_options.h"
#include "mqtt/iasync_client.h"
#include "mqtt/token.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

class mock_async_client : public virtual mqtt::iasync_client
{
public:
    void remove_token(mqtt::token* tok) override {}

    static void succeed(mqtt::token* tok, MQTTAsync_successData* rsp)
    {
        token::on_success(tok, rsp);
    }

    static void succeed5(mqtt::token* tok, MQTTAsync_successData5* rsp)
    {
        token::on_success5(tok, rsp);
    }

    static void fail(mqtt::token* tok, MQTTAsync_failureData* rsp)
    {
        token::on_failure(tok, rsp);
    }

    static void fail5(mqtt::token* tok, MQTTAsync_failureData5* rsp)
    {
        token::on_failure5(tok, rsp);
    }

    // iface

    mqtt::token_ptr connect() override { return mqtt::token_ptr{}; }

    mqtt::token_ptr connect(mqtt::connect_options options) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr connect(
        mqtt::connect_options options, void* userContext, mqtt::iaction_listener& cb
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr connect(void* userContext, mqtt::iaction_listener& cb) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr reconnect() override { return mqtt::token_ptr{}; }

    mqtt::token_ptr disconnect() override { return mqtt::token_ptr{}; }

    mqtt::token_ptr disconnect(disconnect_options) override { return mqtt::token_ptr{}; }

    mqtt::token_ptr disconnect(int timeout) override { return mqtt::token_ptr{}; }

    mqtt::token_ptr disconnect(int timeout, void* userContext, mqtt::iaction_listener& cb)
        override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr disconnect(void* userContext, mqtt::iaction_listener& cb) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::delivery_token_ptr get_pending_delivery_token(int msgID) const override
    {
        return mqtt::delivery_token_ptr{};
    }

    std::vector<mqtt::delivery_token_ptr> get_pending_delivery_tokens() const override
    {
        return std::vector<mqtt::delivery_token_ptr>{};
    };

    int get_buffered_messages_num() const override
    {
        return 0;
    }

    std::string get_client_id() const override { return std::string{}; };

    std::string get_server_uri() const override { return std::string{}; };

    bool is_connected() const override { return true; };

    mqtt::delivery_token_ptr publish(
        string_ref topic, const void* payload, size_t n, int qos, bool retained,
		const properties &props=properties()
    ) override
    {
        auto msg = mqtt::message::create(topic, payload, n, qos, retained);
        return publish(msg);
    };

    mqtt::delivery_token_ptr publish(string_ref topic, const void* payload, size_t n) override
    {
        auto msg = mqtt::message::create(topic, payload, n);
        return publish(msg);
    };

    mqtt::delivery_token_ptr publish(
        string_ref topic, binary_ref payload, int qos, bool retained,
		const properties &props=properties()
    ) override
    {
        auto msg = mqtt::message::create(topic, payload, qos, retained);
        return publish(msg);
    };

    mqtt::delivery_token_ptr publish(string_ref topic, binary_ref payload) override
    {
        auto msg = mqtt::message::create(topic, payload);
        return publish(msg);
    };

    mqtt::delivery_token_ptr publish(
        string_ref topic, const void* payload, size_t n, int qos, bool retained,
        void* userContext, mqtt::iaction_listener& cb
    ) override
    {
        return mqtt::delivery_token_ptr{};
    }

    mqtt::delivery_token_ptr publish(mqtt::const_message_ptr msg) override
    {
        return mqtt::delivery_token::create(*this, msg);
    }

    mqtt::delivery_token_ptr publish(
        mqtt::const_message_ptr msg, void* userContext, mqtt::iaction_listener& cb
    ) override
    {
        return mqtt::delivery_token_ptr{};
    }

    void set_callback(mqtt::callback& cb) override {}
    void disable_callbacks() override {}

    mqtt::token_ptr subscribe(
        const string& topicFilter, int qos,
        const subscribe_options& opts = subscribe_options(),
        const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr subscribe(
        const string& topicFilter, int qos, void* userContext, iaction_listener& callback,
        const subscribe_options& opts = subscribe_options(),
        const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr subscribe(
        const_string_collection_ptr topicFilters, const qos_collection& qos,
        const std::vector<subscribe_options>& opts = std::vector<subscribe_options>(),
        const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr subscribe(
        const_string_collection_ptr topicFilters, const qos_collection& qos,
        void* userContext, iaction_listener& callback,
        const std::vector<subscribe_options>& opts = std::vector<subscribe_options>(),
        const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr unsubscribe(
        const string& topicFilter, const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr unsubscribe(
        const_string_collection_ptr topicFilters, const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr unsubscribe(
        const_string_collection_ptr topicFilters, void* userContext,
        mqtt::iaction_listener& cb, const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    mqtt::token_ptr unsubscribe(
        const string& topicFilter, void* userContext, mqtt::iaction_listener& cb,
        const properties& props = properties()
    ) override
    {
        return mqtt::token_ptr{};
    }

    void start_consuming() override {}
    void stop_consuming() override {}

    const_message_ptr consume_message() override { return const_message_ptr{}; }

    bool try_consume_message(const_message_ptr*) override { return false; }

    event consume_event() override { return const_message_ptr{}; }
    bool try_consume_event(event* evt) override { return false; }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  //  __mqtt_test_mock_async_client_h
