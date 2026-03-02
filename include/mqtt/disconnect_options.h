/////////////////////////////////////////////////////////////////////////////
/// @file disconnect_options.h
/// Implementation of the class 'disconnect_options'
/// @date 26-Aug-2016
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
 * Copyright (c) 2016-2017 Frank Pagliughi <fpagliughi@mindspring.com>
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
 ***************************************************************************/

#ifndef __mqtt_disconnect_options_h
#define __mqtt_disconnect_options_h

#include <chrono>

#include "MQTTAsync.h"
#include "mqtt/properties.h"
#include "mqtt/token.h"
#include "mqtt/types.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * Options for disconnecting from an MQTT broker.
 */
class disconnect_options
{
    /** The default C struct */
    static constexpr MQTTAsync_disconnectOptions DFLT_C_STRUCT
        MQTTAsync_disconnectOptions_initializer;

    /** The default C struct */
    static constexpr MQTTAsync_disconnectOptions DFLT_C_STRUCT5
        MQTTAsync_disconnectOptions_initializer5;

    /** The underlying C disconnect options */
    MQTTAsync_disconnectOptions opts_{DFLT_C_STRUCT};

    /** Shared token pointer for context, if any */
    token_ptr tok_;

    /** Disconnect message properties */
    properties props_;

    /** The client has special access */
    friend class async_client;

    /** The options builder has special access */
    friend class disconnect_options_builder;

    /**
     * Updates the underlying C structure to match our cached data.
     */
    void update_c_struct();

    /** Construct options from a C struct */
    disconnect_options(const MQTTAsync_disconnectOptions& copts) : opts_{copts} {}

public:
    /**
     * Create an empty delivery response object.
     */
    disconnect_options() {}
    /**
     * Creates disconnect options tied to the specific token.
     * @param timeout The timeout (in milliseconds).
     */
    disconnect_options(int timeout) : disconnect_options() { set_timeout(timeout); }
    /**
     * Creates disconnect options tied to the specific token.
     * @param to The timeout.
     */
    template <class Rep, class Period>
    disconnect_options(const std::chrono::duration<Rep, Period>& to) : disconnect_options() {
        set_timeout(to);
    }
    /**
     * Copy constructor.
     * @param opt Another object to copy.
     */
    disconnect_options(const disconnect_options& opt);
    /**
     * Move constructor.
     * @param opt Another object to move into this new one.
     */
    disconnect_options(disconnect_options&& opt);
    /**
     * Creates default options for an MQTT v3.x connection.
     * @return Default options for an MQTT v3.x connection.
     */
    static disconnect_options v3() { return disconnect_options{DFLT_C_STRUCT}; }
    /**
     * Creates default options for an MQTT v5 connection.
     * @return Default options for an MQTT v5 connection.
     */
    static disconnect_options v5() { return disconnect_options{DFLT_C_STRUCT5}; }
    /**
     * Copy assignment.
     * @param opt Another object to copy.
     */
    disconnect_options& operator=(const disconnect_options& opt);
    /**
     * Move assignment.
     * @param opt Another object to move into this new one.
     */
    disconnect_options& operator=(disconnect_options&& opt);
/**
 * Expose the underlying C struct for the unit tests.
 */
#if defined(UNIT_TESTS)
    const MQTTAsync_disconnectOptions& c_struct() const { return opts_; }
#endif
    /**
     * Gets the timeout used for disconnecting.
     * @return The timeout for disconnecting (in milliseconds).
     */
    std::chrono::milliseconds get_timeout() const {
        return std::chrono::milliseconds(opts_.timeout);
    }
    /**
     * Sets the disconnect timeout, in milliseconds.
     * This allows for any remaining in-flight messages to be delivered.
     * @param timeout The disconnect timeout (in milliseconds).
     */
    void set_timeout(int timeout) { opts_.timeout = timeout; }
    /**
     * Sets the disconnect timeout with a duration.
     * This allows for any remaining in-flight messages to be delivered.
     * @param to The disconnect connect timeout.
     */
    template <class Rep, class Period>
    void set_timeout(const std::chrono::duration<Rep, Period>& to) {
        // TODO: check range
        set_timeout((int)to_milliseconds_count(to));
    }
    /**
     * Sets the callback context to a delivery token.
     * @param tok The delivery token to be used as the callback context.
     * @param mqttVersion The version of MQTT we're using for the
     *  				  connection.
     */
    void set_token(const token_ptr& tok, int mqttVersion);
    /**
     * Gets the callback context to a delivery token.
     * @return The delivery token to be used as the callback context.
     */
    token_ptr get_token() const { return tok_; }
    /**
     * Gets the disconnect properties.
     * @return A const reference to the properties for the disconnect.
     */
    const properties& get_properties() const { return props_; }
    /**
     * Gets a mutable reference to the disconnect properties.
     * @return A mutable reference to the properties for the disconnect.
     */
    properties& get_properties() { return props_; }
    /**
     * Sets the properties for the connect.
     * @param props The properties to place into the message.
     */
    void set_properties(const properties& props) {
        props_ = props;
        opts_.properties = props_.c_struct();
    }
    /**
     * Moves the properties for the connect.
     * @param props The properties to move into the connect object.
     */
    void set_properties(properties&& props) {
        props_ = std::move(props);
        opts_.properties = props_.c_struct();
    }
    /**
     * Gets the reason code for the disconnect.
     * @return The reason code for the disconnect.
     */
    ReasonCode get_reason_code() const { return ReasonCode(opts_.reasonCode); }
    /**
     * Sets the reason code for the disconnect.
     * @param code The reason code for the disconnect.
     */
    void set_reason_code(ReasonCode code) { opts_.reasonCode = MQTTReasonCodes(code); }
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Class to build connect options.
 */
class disconnect_options_builder
{
    /** The underlying options */
    disconnect_options opts_;

    /** Construct options builder from a C struct */
    disconnect_options_builder(const MQTTAsync_disconnectOptions& copts) : opts_{copts} {}

public:
    /** This class */
    using self = disconnect_options_builder;
    /**
     * Default constructor.
     */
    disconnect_options_builder() {}
    /**
     * Creates default options builder for an MQTT v3.x connection.
     * @return Default options builder for an MQTT v3.x connection.
     */
    static disconnect_options_builder v3() {
        return disconnect_options_builder{disconnect_options::DFLT_C_STRUCT};
    }
    /**
     * Creates default options builder for an MQTT v5 connection.
     * @return Default options builder for an MQTT v5 connection.
     */
    static disconnect_options_builder v5() {
        return disconnect_options_builder{disconnect_options::DFLT_C_STRUCT5};
    }
    /**
     * Sets the properties for the disconnect message.
     * @param props The properties for the disconnect message.
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
     * Sets the disconnect connect timeout.
     * This allows for any remaining in-flight messages to be delivered.
     * @param to The disconnect timeout.
     */
    template <class Rep, class Period>
    auto timeout(const std::chrono::duration<Rep, Period>& to) -> self& {
        opts_.set_timeout(to);
        return *this;
    }
    /**
     * Sets the reason code for the disconnect.
     * @param code The reason code for the disconnect.
     */
    auto reason_code(ReasonCode code) -> self& {
        opts_.set_reason_code(code);
        return *this;
    }
    /**
     * Finish building the options and return them.
     * @return The option struct as built.
     */
    disconnect_options finalize() { return opts_; }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_disconnect_options_h
