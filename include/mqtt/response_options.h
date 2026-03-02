/////////////////////////////////////////////////////////////////////////////
/// @file response_options.h
/// Implementation of the class 'response_options'
/// @date 26-Aug-2019
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2019-2025 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_response_options_h
#define __mqtt_response_options_h

#include "MQTTAsync.h"
#include "mqtt/delivery_token.h"
#include "mqtt/token.h"
#include "subscribe_options.h"

namespace mqtt {

class token_test;

/////////////////////////////////////////////////////////////////////////////
//							response_options
/////////////////////////////////////////////////////////////////////////////

/**
 * The response options for various asynchronous calls.
 *
 * This is an internal data structure, only used within the library.
 * Therefore it is not totally fleshed out, but rather only exposes the
 * functionality currently required by the library.
 *
 * Note, too, in the C lib, this became a place to add MQTT v5 options for
 * the outgoing calls without breaking the API, so is also referred to as the
 * "call options".
 */
class response_options
{
    /** The underlying C structure */
    MQTTAsync_responseOptions opts_ MQTTAsync_responseOptions_initializer;

    /** The token to which we are connected */
    token::weak_ptr_t tok_;

    /** Packet Properties (Subscribe/Unsubscribe) */
    properties props_;

    /** A list of subscription options for subscribe-many */
    std::vector<MQTTSubscribe_options> subOpts_;

    /** The client has special access */
    friend class async_client;

    /** Update the underlying C struct to match our data */
    void update_c_struct();

public:
    /**
     * Create an empty response object.
     * @param mqttVersion The MQTT version for the response.
     */
    explicit response_options(int mqttVersion = MQTTVERSION_DEFAULT) {
        set_mqtt_version(mqttVersion);
    }
    /**
     * Creates a response object with the specified callbacks.
     * @param tok A token to be used as the context.
     * @param mqttVersion The MQTT version for the response.
     */
    response_options(const token_ptr& tok, int mqttVersion = MQTTVERSION_DEFAULT);
    /**
     * Copy constructor.
     * @param other The other options to copy to this one.
     */
    response_options(const response_options& other);
    /**
     * Move constructor.
     * @param other The other options to move into this one.
     */
    response_options(response_options&& other);
    /**
     * Copy operator.
     * @param rhs The other options to copy to this one.
     */
    response_options& operator=(const response_options& rhs);
    /**
     * Move operator.
     * @param rhs The other options to move into this one.
     */
    response_options& operator=(response_options&& rhs);
/**
 * Expose the underlying C struct for the unit tests.
 */
#if defined(UNIT_TESTS)
    const auto& c_struct() const { return opts_; }
#endif
    /**
     * Sets the MQTT protocol version used for the response.
     * This sets up proper callbacks for MQTT v5 or versions prior to that.
     * @param mqttVersion The MQTT version used by the connection.
     */
    void set_mqtt_version(int mqttVersion);
    /**
     * Sets the callback context to a generic token.
     * @param tok The token to be used as the callback context.
     */
    void set_token(const token_ptr& tok);
    /**
     * Gets the properties for the response options.
     */
    const properties& get_properties() const { return props_; }
    /**
     * Sets the properties for the response options.
     * @param props The properties for the response options.
     */
    void set_properties(const properties& props) {
        props_ = props;
        opts_.properties = props_.c_struct();
    }
    /**
     * Moves the properties for the response options.
     * @param props The properties to move into the response options.
     */
    void set_properties(properties&& props) {
        props_ = std::move(props);
        opts_.properties = props_.c_struct();
    }
    /**
     * Gets the options for a single topic subscription.
     * @return The subscribe options.
     */
    subscribe_options get_subscribe_options() const {
        return subscribe_options{opts_.subscribeOptions};
    }
    /**
     * Sets the options for a multi-topic subscription.
     * @return The vector of the subscribe options.
     */
    std::vector<subscribe_options> get_subscribe_many_options() const;
    /**
     * Sets the options for a single topic subscription.
     * @param opts The subscribe options.
     */
    void set_subscribe_options(const subscribe_options& opts);
    /**
     * Sets the options for a multi-topic subscription.
     * @param opts A vector of the subscribe options.
     */
    void set_subscribe_many_options(const std::vector<subscribe_options>& opts);
    /**
     * Sets the options for a multi-topic subscription.
     * @param opts A vector of the subscribe options.
     * @sa set_subscribe_options
     */
    void set_subscribe_options(const std::vector<subscribe_options>& opts) {
        set_subscribe_many_options(opts);
    }
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Class to build response options.
 */
class response_options_builder
{
    /** The underlying options */
    response_options opts_;

public:
    /** This class */
    using self = response_options_builder;
    /**
     * Default constructor.
     */
    explicit response_options_builder(int mqttVersion = MQTTVERSION_DEFAULT)
        : opts_(mqttVersion) {}
    /**
     * Sets the MQTT protocol version used for the response.
     * This sets up proper callbacks for MQTT v5 or versions prior to that.
     * @param mqttVersion The MQTT version used by the connection.
     */
    auto mqtt_version(int mqttVersion) -> self& {
        opts_.set_mqtt_version(mqttVersion);
        return *this;
    }
    /**
     * Sets the callback context to a generic token.
     * @param tok The token to be used as the callback context.
     */
    auto token(const token_ptr& tok) -> self& {
        opts_.set_token(tok);
        return *this;
    }
    /**
     * Sets the properties for the response options.
     * @param props The properties for the response options.
     */
    auto properties(mqtt::properties&& props) -> self& {
        opts_.set_properties(std::move(props));
        return *this;
    }
    /**
     * Sets the properties for the disconnect message.
     * @param props The properties for the disconnect message.
     */
    auto properties(const mqtt::properties& props) -> self& {
        opts_.set_properties(props);
        return *this;
    }
    /**
     * Sets the options for a single topic subscription.
     * @param opts The subscribe options.
     */
    auto subscribe_opts(const subscribe_options& opts) -> self& {
        opts_.set_subscribe_options(opts);
        return *this;
    }
    /**
     * Sets the options for a multi-topic subscription.
     * @param opts A vector of the subscribe options.
     */
    auto subscribe_many_opts(const std::vector<subscribe_options>& opts) -> self& {
        opts_.set_subscribe_options(opts);
        return *this;
    }
    /**
     * Sets the options for a multi-topic subscription.
     * @param opts A vector of the subscribe options.
     */
    auto subscribe_opts(const std::vector<subscribe_options>& opts) -> self& {
        opts_.set_subscribe_options(opts);
        return *this;
    }
    /**
     * Finish building the response options and return them.
     * @return The response option struct as built.
     */
    response_options finalize() { return opts_; }
};

/////////////////////////////////////////////////////////////////////////////
//						delivery_response_options
/////////////////////////////////////////////////////////////////////////////

/**
 * The response options for asynchronous calls targeted at delivery.
 * Each of these objects is tied to a specific delivery_token.
 */
class delivery_response_options
{
    /** The underlying C structure */
    MQTTAsync_responseOptions opts_;

    /** The delivery token to which we are connected */
    delivery_token::weak_ptr_t dtok_;

    /** The client has special access */
    friend class async_client;

public:
    /**
     * Create an empty delivery response object.
     */
    delivery_response_options(int mqttVersion = MQTTVERSION_DEFAULT);
    /**
     * Creates a response object tied to the specific delivery token.
     * @param dtok A delivery token to be used as the context.
     * @param mqttVersion The MQTT version for the response
     */
    delivery_response_options(
        const delivery_token_ptr& dtok, int mqttVersion = MQTTVERSION_DEFAULT
    );
/**
 * Expose the underlying C struct for the unit tests.
 */
#if defined(UNIT_TESTS)
    const MQTTAsync_responseOptions& c_struct() const { return opts_; }
#endif
    /**
     * Sets the callback context to a delivery token.
     * @param dtok The delivery token to be used as the callback context.
     */
    void set_token(const delivery_token_ptr& dtok) {
        dtok_ = dtok;
        opts_.context = dtok.get();
    }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_response_options_h
