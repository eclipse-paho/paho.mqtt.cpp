/////////////////////////////////////////////////////////////////////////////
/// @file connect_options.h
/// Declaration of MQTT connect_options class
/// @date May 1, 2013
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2013-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_connect_options_h
#define __mqtt_connect_options_h

#include <chrono>
#include <map>
#include <vector>

#include "MQTTAsync.h"
#include "mqtt/message.h"
#include "mqtt/platform.h"
#include "mqtt/ssl_options.h"
#include "mqtt/string_collection.h"
#include "mqtt/token.h"
#include "mqtt/topic.h"
#include "mqtt/types.h"
#include "mqtt/will_options.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * Holds the set of options that control how the client connects to a
 * server.
 */
class connect_options
{
    /** The default C struct for non-WebSocket connections */
    static constexpr MQTTAsync_connectOptions DFLT_C_STRUCT
        MQTTAsync_connectOptions_initializer;

    /** The default C struct for non-Websocket MQTT v5 connections */
    static constexpr MQTTAsync_connectOptions DFLT_C_STRUCT5
        MQTTAsync_connectOptions_initializer5;

    /** The default C struct for WebSocket connections */
    static constexpr MQTTAsync_connectOptions DFLT_C_STRUCT_WS
        MQTTAsync_connectOptions_initializer_ws;

    /** The default C struct for Websocket MQTT v5 connections */
    static constexpr MQTTAsync_connectOptions DFLT_C_STRUCT5_WS
        MQTTAsync_connectOptions_initializer5_ws;

    /** The underlying C connection options */
    MQTTAsync_connectOptions opts_;

    /** The LWT options */
    will_options will_;

    /** The SSL options  */
    ssl_options ssl_;

    /** The user name to use for the connection. */
    string_ref userName_;

    /** The password to use for the connection. */
    binary_ref password_;

    /** Shared token pointer for context, if any */
    token_ptr tok_;

    /** Collection of server URIs, if any */
    const_string_collection_ptr serverURIs_;

    /** The connect properties */
    properties props_;

    /** HTTP Headers */
    name_value_collection httpHeaders_;

    /** HTTP proxy for websockets */
    string httpProxy_;

    /** Secure HTTPS proxy for websockets */
    string httpsProxy_;

    /** The client has special access */
    friend class async_client;

    /**
     * Gets a pointer to the C-language NUL-terminated strings for the
     * struct.
     * @note In the connect options, by default, the Paho C treats
     * nullptr char arrays as unset values, so we keep that semantic and
     * only set those char arrays if the string is non-empty.
     * @param sr The C++ string object.
     * @return Pointer to a NUL terminated string. This is only valid until
     *  	   the next time the string is updated.
     */
    const char* c_str(const string_ref& sr) { return sr.empty() ? nullptr : sr.c_str(); }
    const char* c_str(const string& s) { return s.empty() ? nullptr : s.c_str(); }
    /**
     * Updates the underlying C structure to match our strings.
     */
    void update_c_struct();

    /**
     * Creates the options from a C option struct.
     * @param copts The C options struct.
     */
    connect_options(const MQTTAsync_connectOptions& copts) : opts_(copts) {}

public:
    /** Smart/shared pointer to an object of this class. */
    using ptr_t = std::shared_ptr<connect_options>;
    /** Smart/shared pointer to a const object of this class. */
    using const_ptr_t = std::shared_ptr<const connect_options>;

    /**
     * Constructs a new object using the default values.
     *
     * @param ver The MQTT protocol version.
     */
    explicit connect_options(int ver = MQTTVERSION_DEFAULT);
    /**
     * Constructs a new object using the specified user name and password.
     * @param userName The name of the user for connecting to the server
     * @param password The password for connecting to the server
     * @param ver The MQTT protocol version.
     */
    connect_options(string_ref userName, binary_ref password, int ver = MQTTVERSION_DEFAULT);
    /**
     * Copy constructor.
     * @param opt Another object to copy.
     */
    connect_options(const connect_options& opt);
    /**
     * Move constructor.
     * @param opt Another object to move into this new one.
     */
    connect_options(connect_options&& opt);
    /**
     * Creates default options for an MQTT v3.x connection.
     * @return Default options for an MQTT v3.x connection.
     */
    static connect_options v3() { return connect_options(DFLT_C_STRUCT); }
    /**
     * Creates default options for an MQTT v5 connection.
     * @return Default options for an MQTT v5 connection.
     */
    static connect_options v5() { return connect_options(DFLT_C_STRUCT5); }
    /**
     * Creates default options for an MQTT v3.x connection using WebSockets.
     *
     * The keepalive interval is set to 45 seconds to avoid webserver 60
     * second inactivity timeouts.
     *
     * @return Default options for an MQTT v3.x connection using websockets.
     */
    static connect_options ws() { return connect_options(DFLT_C_STRUCT_WS); }
    /**
     * Creates default options for an MQTT v5 connection using WebSockets.
     *
     * The keepalive interval is set to 45 seconds to avoid webserver 60
     * second inactivity timeouts.
     *
     * @return Default options for an MQTT v5 connection using websockets.
     */
    static connect_options v5_ws() { return connect_options(DFLT_C_STRUCT5_WS); }
    /**
     * Copy assignment.
     * @param opt Another object to copy.
     */
    connect_options& operator=(const connect_options& opt);
    /**
     * Move assignment.
     * @param opt Another object to move into this new one.
     */
    connect_options& operator=(connect_options&& opt);
/**
 * Expose the underlying C struct for the unit tests.
 */
#if defined(UNIT_TESTS)
    const MQTTAsync_connectOptions& c_struct() const { return opts_; }
#endif
    /**
     * Gets the "keep alive" interval.
     * @return The keep alive interval in seconds.
     */
    std::chrono::seconds get_keep_alive_interval() const {
        return std::chrono::seconds(opts_.keepAliveInterval);
    }
    /**
     * Gets the connection timeout.
     * This is the amount of time the underlying library will wait for a
     * timeout before failing.
     * @return The connect timeout in seconds.
     */
    std::chrono::seconds get_connect_timeout() const {
        return std::chrono::seconds(opts_.connectTimeout);
    }
    /**
     * Gets the user name to use for the connection.
     * @return The user name to use for the connection.
     */
    string get_user_name() const { return userName_ ? userName_.to_string() : string(); }
    /**
     * Gets the password to use for the connection.
     * @return The password to use for the connection.
     */
    binary_ref get_password() const { return password_; }
    /**
     * Gets the password to use for the connection.
     * @return The password to use for the connection.
     */
    string get_password_str() const { return password_ ? password_.to_string() : string(); }
    /**
     * Gets the maximum number of messages that can be in-flight
     * simultaneously.
     * @return The maximum number of inflight messages.
     */
    int get_max_inflight() const { return opts_.maxInflight; }
    /**
     * Gets the topic to be used for last will and testament (LWT).
     * @return The topic to be used for last will and testament (LWT).
     */
    string get_will_topic() const { return will_.get_topic(); }
    /**
     * Gets the message to be sent as last will and testament (LWT).
     * @return The message to be sent as last will and testament (LWT).
     */
    const_message_ptr get_will_message() const { return will_.get_message(); }
    /**
     * Get the LWT options to use for the connection.
     * @return The LWT options to use for the connection.
     */
    const will_options& get_will_options() const { return will_; }
    /**
     * Get the SSL options to use for the connection.
     * @return The SSL options to use for the connection.
     */
    const ssl_options& get_ssl_options() const { return ssl_; }
    /**
     * Sets the SSL for the connection.
     * These will only have an effect if compiled against the SSL version of
     * the Paho C library, and using a secure connection, "ssl://" or
     * "wss://".
     * @param ssl The SSL options.
     */
    void set_ssl(const ssl_options& ssl);
    /**
     * Sets the SSL for the connection.
     * These will only have an effect if compiled against the SSL version of
     * the Paho C library, and using a secure connection, "ssl://" or
     * "wss://".
     * @param ssl The SSL options.
     */
    void set_ssl(ssl_options&& ssl);
    /**
     * Returns whether the server should remember state for the client
     * across reconnects. This only applies to MQTT v3.x connections.
     * @return @em true if requesting a clean session, @em false if not.
     */
    bool is_clean_session() const { return to_bool(opts_.cleansession); }
    /**
     * Returns whether the server should remember state for the client
     * across reconnects. This only applies to MQTT v5 connections.
     * @return @em true if requesting a clean start, @em false if not.
     */
    bool is_clean_start() const { return to_bool(opts_.cleanstart); }
    /**
     * Gets the token used as the callback context.
     * @return The delivery token used as the callback context.
     */
    token_ptr get_token() const { return tok_; }
    /**
     * Gets the list of servers to which the client will connect.
     * @return A collection of server URI's. Each entry should be of the
     *  	   form @em protocol://host:port where @em protocol must be tcp
     *  	   or @em ssl. For @em host, you can specify either an IP
     *  	   address or a domain name.
     */
    const_string_collection_ptr get_servers() const { return serverURIs_; }
    /**
     * Gets the version of MQTT to be used for the connection.
     * @return The version of MQTT to be used for the connection:
     * @li MQTTVERSION_DEFAULT (0) = default: start with 3.1.1, and if that
     *     fails, fall back to 3.1
     * @li MQTTVERSION_3_1 (3) = only try version 3.1
     * @li MQTTVERSION_3_1_1 (4) = only try version 3.1.1
     */
    int get_mqtt_version() const { return opts_.MQTTVersion; }
    /**
     * Determines if the options have been configured for automatic
     * reconnect.
     * @return @em true if configured for automatic reconnect, @em false if
     *  	   not.
     */
    bool get_automatic_reconnect() const { return to_bool(opts_.automaticReconnect); }
    /**
     * Gets the minimum retry interval for automatic reconnect.
     * @return The minimum retry interval for automatic reconnect, in
     *  	   seconds.
     */
    std::chrono::seconds get_min_retry_interval() const {
        return std::chrono::seconds(opts_.minRetryInterval);
    }
    /**
     * Gets the maximum retry interval for automatic reconnect.
     * @return The maximum retry interval for automatic reconnect, in
     *  	   seconds.
     */
    std::chrono::seconds get_max_retry_interval() const {
        return std::chrono::seconds(opts_.maxRetryInterval);
    }
    /**
     * Sets whether the server should remember state for the client across
     * reconnects. (MQTT v3.x only)
     *
     * This will only take effect if the version is _already_ set to v3.x
     * (not v5).
     *
     * @param cleanSession @em true if the server should NOT remember state
     *              for the client across reconnects, @em false otherwise.
     */
    void set_clean_session(bool cleanSession);
    /**
     * Sets whether the server should remember state for the client across
     * reconnects. (MQTT v5 only)
     *
     * If a persistent session is desired (turning this off), then the app
     * should also set the `Session Expiry Interval` property, and add that
     * to the connect options.
     *
     * This will only take effect if the MQTT version is set to v5
     *
     * @param cleanStart @em true if the server should NOT remember state
     *              for the client across reconnects, @em false otherwise.
     */
    void set_clean_start(bool cleanStart);
    /**
     * Sets the "keep alive" interval.
     * This is the maximum time that should pass without communications
     * between client and server. If no messages pass in this time, the
     * client will ping the broker.
     * @param keepAliveInterval The keep alive interval in seconds.
     */
    void set_keep_alive_interval(int keepAliveInterval) {
        opts_.keepAliveInterval = keepAliveInterval;
    }
    /**
     * Sets the "keep alive" interval with a chrono duration.
     * This is the maximum time that should pass without communications
     * between client and server. If no messages pass in this time, the
     * client will ping the broker.
     * @param interval The keep alive interval.
     */
    template <class Rep, class Period>
    void set_keep_alive_interval(const std::chrono::duration<Rep, Period>& interval) {
        // TODO: Check range
        set_keep_alive_interval((int)to_seconds_count(interval));
    }
    /**
     * Sets the connect timeout in seconds.
     * This is the maximum time that the underlying library will wait for a
     * connection before failing.
     * @param timeout The connect timeout in seconds.
     */
    void set_connect_timeout(int timeout) { opts_.connectTimeout = timeout; }
    /**
     * Sets the connect timeout with a chrono duration.
     * This is the maximum time that the underlying library will wait for a
     * connection before failing.
     * @param timeout The connect timeout in seconds.
     */
    template <class Rep, class Period>
    void set_connect_timeout(const std::chrono::duration<Rep, Period>& timeout) {
        // TODO: check range
        set_connect_timeout((int)to_seconds_count(timeout));
    }
    /**
     * Sets the user name to use for the connection.
     * @param userName The user name for connecting to the MQTT broker.
     */
    void set_user_name(string_ref userName);
    /**
     * Sets the password to use for the connection.
     * @param password The password for connecting to the MQTT broker.
     */
    void set_password(binary_ref password);
    /**
     * Sets the maximum number of messages that can be in-flight
     * simultaneously.
     * @param n The maximum number of inflight messages.
     */
    void set_max_inflight(int n) { opts_.maxInflight = n; }
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param will The LWT options.
     */
    void set_will(const will_options& will);
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param will The LWT options.
     */
    void set_will(will_options&& will);
    /**
     * Sets the "Last Will and Testament" (LWT) as a message
     * @param msg The LWT message
     */
    void set_will_message(const message& msg) { set_will(will_options(msg)); }
    /**
     * Sets the "Last Will and Testament" (LWT) as a message
     * @param msg Pointer to a LWT message
     */
    void set_will_message(const_message_ptr msg) {
        if (msg)
            set_will(will_options(*msg));
    }
    /**
     * Sets the callback context to a delivery token.
     * @param tok The delivery token to be used as the callback context.
     */
    void set_token(const token_ptr& tok);
    /**
     * Sets the list of servers to which the client will connect.
     * @param serverURIs A pointer to a collection of server URI's. Each
     *  				 entry should be of the form @em
     *  				 protocol://host:port where @em protocol must be
     *  				 @em tcp or @em ssl. For @em host, you can specify
     *  				 either an IP address or a domain name.
     */
    void set_servers(const_string_collection_ptr serverURIs);
    /**
     * Sets the version of MQTT to be used on the connect.
     *
     * This will also set other connect options to legal values dependent on
     * the selected version.
     *
     * @param mqttVersion The MQTT version to use for the connection:
     *   @li MQTTVERSION_DEFAULT (0) = default: start with 3.1.1, and if
     *       that fails, fall back to 3.1
     *   @li MQTTVERSION_3_1 (3) = only try version 3.1
     *   @li MQTTVERSION_3_1_1 (4) = only try version 3.1.1
     *   @li MQTTVERSION_5 (5) = only try version 5
     *
     * Note that it is preferable to create the options for the desired
     * version rather than using this function to change the version after
     * some parameters have already been set. If you do use this function,
     * call it before setting any other version-specific options. @sa
     * connect_options::v5()
     */
    void set_mqtt_version(int mqttVersion);
    /**
     * Enable or disable automatic reconnects.
     * The retry intervals are not affected.
     * @param on Whether to turn reconnects on or off
     */
    void set_automatic_reconnect(bool on) { opts_.automaticReconnect = to_int(on); }
    /**
     * Enable or disable automatic reconnects.
     * @param minRetryInterval Minimum retry interval in seconds.  Doubled
     *  					   on each failed retry.
     * @param maxRetryInterval Maximum retry interval in seconds.  The
     *  					   doubling stops here on failed retries.
     */
    void set_automatic_reconnect(int minRetryInterval, int maxRetryInterval);
    /**
     * Enable or disable automatic reconnects.
     * @param minRetryInterval Minimum retry interval. Doubled on each
     *  					   failed retry.
     * @param maxRetryInterval Maximum retry interval. The doubling stops
     *  					   here on failed retries.
     */
    template <class Rep1, class Period1, class Rep2, class Period2>
    void set_automatic_reconnect(
        const std::chrono::duration<Rep1, Period1>& minRetryInterval,
        const std::chrono::duration<Rep2, Period2>& maxRetryInterval
    ) {
        set_automatic_reconnect(
            (int)to_seconds_count(minRetryInterval), (int)to_seconds_count(maxRetryInterval)
        );
    }
    /**
     * Gets the connect properties.
     * @return A const reference to the properties for the connect.
     */
    const properties& get_properties() const { return props_; }
    /**
     * Gets a mutable reference to the connect properties.
     * @return A reference to the properties for the connect.
     */
    properties& get_properties() { return props_; }
    /**
     * Sets the properties for the connect.
     * @param props The properties to place into the message.
     */
    void set_properties(const properties& props);
    /**
     * Moves the properties for the connect.
     * @param props The properties to move into the connect object.
     */
    void set_properties(properties&& props);
    /**
     * Gets the HTTP headers
     * @return A const reference to the HTTP headers name/value collection.
     */
    const name_value_collection& get_http_headers() const { return httpHeaders_; }
    /**
     * Sets the HTTP headers for the connection.
     * @param httpHeaders The header nam/value collection.
     */
    void set_http_headers(const name_value_collection& httpHeaders) {
        httpHeaders_ = httpHeaders;
        opts_.httpHeaders = httpHeaders_.empty() ? nullptr : httpHeaders_.c_arr();
    }
    /**
     * Sets the HTTP headers for the connection.
     * @param httpHeaders The header nam/value collection.
     */
    void set_http_headers(name_value_collection&& httpHeaders) {
        httpHeaders_ = std::move(httpHeaders);
        opts_.httpHeaders = httpHeaders_.empty() ? nullptr : httpHeaders_.c_arr();
    }
    /**
     * Gets the HTTP proxy setting.
     * @return The HTTP proxy setting. An empty string means no proxy.
     */
    string get_http_proxy() const { return httpProxy_; }
    /**
     * Sets the HTTP proxy setting.
     * @param httpProxy The HTTP proxy setting. An empty string means no
     *  			  proxy.
     */
    void set_http_proxy(const string& httpProxy);
    /**
     * Gets the secure HTTPS proxy setting.
     * @return The HTTPS proxy setting. An empty string means no proxy.
     */
    string get_https_proxy() const { return httpsProxy_; }
    /**
     * Sets the secure HTTPS proxy setting.
     * @param httpsProxy The HTTPS proxy setting. An empty string means no
     *  			 proxy.
     */
    void set_https_proxy(const string& httpsProxy);
};

/** Smart/shared pointer to a connection options object. */
using connect_options_ptr = connect_options::ptr_t;

/////////////////////////////////////////////////////////////////////////////

/**
 * The connect options that can be updated before an automatic reconnect.
 */
class connect_data
{
    /** The default C struct */
    PAHO_MQTTPP_EXPORT static constexpr MQTTAsync_connectData DFLT_C_STRUCT
        MQTTAsync_connectData_initializer;

    /** The underlying C connect data  */
    MQTTAsync_connectData data_{DFLT_C_STRUCT};

    /** The user name to use for the connection. */
    string_ref userName_;

    /** The password to use for the connection. (Optional) */
    binary_ref password_;

    /** The client has special access */
    friend class async_client;

    /**
     * Updates the underlying C structure to match our strings.
     */
    void update_c_struct();

    /**
     * Create data from a C struct
     * This is a deep copy of the data from the C struct.
     * @param cdata The C connect data.
     */
    connect_data(const MQTTAsync_connectData& cdata);

public:
    /**
     * Creates an empty set of connection data.
     */
    connect_data();
    /**
     * Creates connection data with a user name, but no password.
     * @param userName The user name for reconnecting to the MQTT broker.
     */
    explicit connect_data(string_ref userName);
    /**
     * Creates connection data with a user name and password.
     * @param userName The user name for reconnecting to the MQTT broker.
     * @param password The password for connecting to the MQTT broker.
     */
    connect_data(string_ref userName, binary_ref password);
    /**
     * Copy constructor
     * @param other Another data struct to copy into this one.
     */
    connect_data(const connect_data& other);
    /**
     * Copy the connection data.
     * @param rhs Another data struct to copy into this one.
     * @return A reference to this data
     */
    connect_data& operator=(const connect_data& rhs);
    /**
     * Gets the user name to use for the connection.
     * @return The user name to use for the connection.
     */
    string get_user_name() const { return userName_ ? userName_.to_string() : string(); }
    /**
     * Gets the password to use for the connection.
     * @return The password to use for the connection.
     */
    binary_ref get_password() const { return password_; }
    /**
     * Sets the user name to use for the connection.
     * @param userName The user name for connecting to the MQTT broker.
     */
    void set_user_name(string_ref userName);
    /**
     * Sets the password to use for the connection.
     * @param password The password for connecting to the MQTT broker.
     */
    void set_password(binary_ref password);
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Class to build connect options.
 */
class connect_options_builder
{
    connect_options opts_;

public:
    /** This class */
    using self = connect_options_builder;
    /**
     * Default constructor.
     *
     * @param ver The MQTT version for the connection. Defaults to the most
     *  		  recent v3 supported by the server.
     */
    explicit connect_options_builder(int ver = MQTTVERSION_DEFAULT) : opts_(ver) {}
    /**
     * Copy constructor from an existing set of options.
     */
    explicit connect_options_builder(const connect_options& opts) : opts_(opts) {}
    /**
     * Move constructor from an existing set of options.
     */
    explicit connect_options_builder(connect_options&& opts) : opts_(std::move(opts)) {}
    /**
     * Creates the default options builder for an MQTT v3.x connection.
     * @return An options builder for an MQTT v3.x connection.
     */
    static connect_options_builder v3() {
        return connect_options_builder{connect_options::v3()};
    }
    /**
     * Creates the default options builder for an MQTT v5 connection.
     * @return An options builder for an MQTT v5 connection.
     */
    static connect_options_builder v5() {
        return connect_options_builder{connect_options::v5()};
    }
    /**
     * Creates the default options builder for an MQTT v3.x connection using
     * WebSockets.
     *
     * The keepalive interval is set to 45 seconds to avoid webserver 60
     * second inactivity timeouts.
     *
     * @return An options builder for an MQTT v3.x connection using
     *  	   websockets.
     */
    static connect_options_builder ws() {
        return connect_options_builder{connect_options::ws()};
    }
    /**
     * Creates the default options for an MQTT v5 connection using
     * WebSockets.
     * The keepalive interval is set to 45 seconds to avoid webserver 60
     * second inactivity timeouts.
     *
     * @return An options builder for an MQTT v5 connection using
     *  	   websockets.
     */
    static connect_options_builder v5_ws() {
        return connect_options_builder{connect_options::v5_ws()};
    }
    /**
     * Sets whether the server should remember state for the client across
     * reconnects. (MQTT v3.x only)
     * @param on @em true if the server should NOT remember state for the
     *  		 client across reconnects, @em false otherwise.
     */
    auto clean_session(bool on = true) -> self& {
        opts_.set_clean_session(on);
        return *this;
    }
    /**
     * Sets the "keep alive" interval with a chrono duration.
     * This is the maximum time that should pass without communications
     * between client and server. If no messages pass in this time, the
     * client will ping the broker.
     * @param interval The keep alive interval.
     */
    template <class Rep, class Period>
    auto keep_alive_interval(const std::chrono::duration<Rep, Period>& interval) -> self& {
        opts_.set_keep_alive_interval(interval);
        return *this;
    }
    /**
     * Sets the connect timeout with a chrono duration.
     * This is the maximum time that the underlying library will wait for a
     * connection before failing.
     * @param timeout The connect timeout in seconds.
     */
    template <class Rep, class Period>
    auto connect_timeout(const std::chrono::duration<Rep, Period>& timeout) -> self& {
        opts_.set_connect_timeout(timeout);
        return *this;
    }
    /**
     * Sets the user name for the connection.
     * @param userName The user name for the connection.
     */
    auto user_name(string_ref userName) -> self& {
        opts_.set_user_name(userName);
        return *this;
    }
    /**
     * Sets the password for the connection.
     * @param password The password for the connection.
     */
    auto password(binary_ref password) -> self& {
        opts_.set_password(password);
        return *this;
    }
    /**
     * Sets the maximum number of messages that can be in-flight
     * simultaneously.
     * @param n The maximum number of inflight messages.
     */
    auto max_inflight(int n) -> self& {
        opts_.set_max_inflight(n);
        return *this;
    }
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param will The LWT options.
     */
    auto will(const will_options& will) -> self& {
        opts_.set_will(will);
        return *this;
    }
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param will The LWT options.
     */
    auto will(will_options&& will) -> self& {
        opts_.set_will(std::move(will));
        return *this;
    }
    /**
     * Sets the "Last Will and Testament" (LWT) as a message
     * @param msg The LWT message
     */
    auto will(const message& msg) -> self& {
        opts_.set_will_message(msg);
        return *this;
    }
    /**
     * Sets the SSL options for the connection.
     * These will only have an effect if compiled against the SSL version of
     * the Paho C library, and connecting with a secure URI.
     * @param ssl The SSL options.
     */
    auto ssl(const ssl_options& ssl) -> self& {
        opts_.set_ssl(ssl);
        return *this;
    }
    /**
     * Sets the SSL options for the connection.
     * These will only have an effect if compiled against the SSL version of
     * the Paho C library, and connecting with a secure URI.
     * @param ssl The SSL options.
     */
    auto ssl(ssl_options&& ssl) -> self& {
        opts_.set_ssl(std::move(ssl));
        return *this;
    }
    /**
     * Sets the callback context to a delivery token.
     * @param tok The delivery token to be used as the callback context.
     */
    auto token(const token_ptr& tok) -> self& {
        opts_.set_token(tok);
        return *this;
    }
    /**
     * Sets the list of servers to which the client will connect.
     * @param serverURIs A pointer to a collection of server URI's. Each
     *  				 entry should be of the form @em
     *  				 protocol://host:port where @em protocol must be
     *  				 @em tcp or @em ssl. For @em host, you can specify
     *  				 either an IP address or a domain name.
     */
    auto servers(const_string_collection_ptr serverURIs) -> self& {
        opts_.set_servers(serverURIs);
        return *this;
    }
    /**
     * Sets the version of MQTT to be used on the connect.
     *
     * This will also set other connect options to legal values dependent on
     * the selected version.
     *
     * @param ver The MQTT protocol version to use for the connection:
     *   @li MQTTVERSION_DEFAULT (0) = default: start with 3.1.1, and if
     *       that fails, fall back to 3.1
     *   @li MQTTVERSION_3_1 (3) = only try version 3.1
     *   @li MQTTVERSION_3_1_1 (4) = only try version 3.1.1
     *   @li MQTTVERSION_5 (5) = only try version 5
     *
     * Note that it is preferable to create the options builder for the
     * desired version rather than using this function to change the version
     * after some parameters have already been set. If you do use this
     * function, call it before setting any other version-specific options.
     * @sa connect_options_builder::v5()
     */
    auto mqtt_version(int ver) -> self& {
        opts_.set_mqtt_version(ver);
        return *this;
    }
    /**
     * Enable or disable automatic reconnects.
     * The retry intervals are not affected.
     * @param on Whether to turn reconnects on or off
     */
    auto automatic_reconnect(bool on = true) -> self& {
        opts_.set_automatic_reconnect(on);
        return *this;
    }
    /**
     * Enable or disable automatic reconnects.
     * @param minRetryInterval Minimum retry interval. Doubled on each
     *  					   failed retry.
     * @param maxRetryInterval Maximum retry interval. The doubling stops
     *  					   here on failed retries.
     */
    template <class Rep1, class Period1, class Rep2, class Period2>
    auto automatic_reconnect(
        const std::chrono::duration<Rep1, Period1>& minRetryInterval,
        const std::chrono::duration<Rep2, Period2>& maxRetryInterval
    ) -> self& {
        opts_.set_automatic_reconnect(minRetryInterval, maxRetryInterval);
        return *this;
    }
    /**
     * Sets the 'clean start' flag for the connection. (MQTT v5 only)
     * @param on @em true to set the 'clean start' flag for the connect,
     *  		 @em false otherwise.
     */
    auto clean_start(bool on = true) -> self& {
        opts_.set_clean_start(on);
        return *this;
    }
    /**
     * Sets the properties for the connect message.
     * @param props The properties for the connect message.
     */
    auto properties(const mqtt::properties& props) -> self& {
        opts_.set_properties(props);
        return *this;
    }
    /**
     * Sets the properties for the connect message.
     * @param props The properties for the connect message.
     */
    auto properties(mqtt::properties&& props) -> self& {
        opts_.set_properties(std::move(props));
        return *this;
    }
    /**
     * Sets the HTTP headers for the connection.
     * @param headers The header nam/value collection.
     */
    auto http_headers(const name_value_collection& headers) -> self& {
        opts_.set_http_headers(headers);
        return *this;
    }
    /**
     * Sets the HTTP headers for the connection.
     * @param headers The header nam/value collection.
     */
    auto http_headers(name_value_collection&& headers) -> self& {
        opts_.set_http_headers(std::move(headers));
        return *this;
    }
    /**
     * Sets the HTTP proxy setting.
     * @param httpProxy The HTTP proxy setting. An empty string means no
     *  			  proxy.
     */
    auto http_proxy(const string& httpProxy) -> self& {
        opts_.set_http_proxy(httpProxy);
        return *this;
    }
    /**
     * Sets the secure HTTPS proxy setting.
     * @param httpsProxy The HTTPS proxy setting. An empty string means no
     *  			 proxy.
     */
    auto https_proxy(const string& httpsProxy) -> self& {
        opts_.set_https_proxy(httpsProxy);
        return *this;
    }
    /**
     * Finish building the options and return them.
     * @return The option struct as built.
     */
    connect_options finalize() { return opts_; }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_connect_options_h
