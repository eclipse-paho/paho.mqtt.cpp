/////////////////////////////////////////////////////////////////////////////
/// @file async_client.h
/// Declaration of MQTT async_client class
/// @date May 1, 2013
/// @author Frank Pagliughi
///
/// @mainpage The Eclipse Paho MQTT Library for C++
///
/// @section Introduction
///
/// This is the Eclipse Paho MQTT Library for C++. It contains an MQTT
/// client for memory-managed operating systems like Windows, macOS, Linux,
/// and other *nix-style systems.
///
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2013-2025 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi - MQTT v5 support
 *******************************************************************************/

#ifndef __mqtt_async_client_h
#define __mqtt_async_client_h

#include <functional>
#include <list>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "MQTTAsync.h"
#include "mqtt/callback.h"
#include "mqtt/create_options.h"
#include "mqtt/delivery_token.h"
#include "mqtt/event.h"
#include "mqtt/exception.h"
#include "mqtt/iaction_listener.h"
#include "mqtt/iasync_client.h"
#include "mqtt/iclient_persistence.h"
#include "mqtt/message.h"
#include "mqtt/properties.h"
#include "mqtt/string_collection.h"
#include "mqtt/thread_queue.h"
#include "mqtt/token.h"
#include "mqtt/types.h"

namespace mqtt {

// OBSOLETE: The legacy constants that lacked the "PAHO_MQTTPP_" prefix
// clashed with #define's from other libraries and will be removed at the
// next major version upgrade.

#if defined(PAHO_MQTTPP_VERSIONS)
/** The version number for the client library. */
const uint32_t PAHO_MQTTPP_VERSION = 0x01050003;
/** The version string for the client library  */
const string PAHO_MQTTPP_VERSION_STR("Paho MQTT C++ (mqttpp) v. 1.5.3");
/** Copyright notice for the client library */
const string PAHO_MQTTPP_COPYRIGHT("Copyright (c) 2013-2025 Frank Pagliughi");
#else
/** The version number for the client library. */
const uint32_t VERSION = 0x01050003;
/** The version string for the client library  */
const string VERSION_STR("Paho MQTT C++ (mqttpp) v. 1.5.3");
/** Copyright notice for the client library */
const string COPYRIGHT("Copyright (c) 2013-2025 Frank Pagliughi");
#endif

/////////////////////////////////////////////////////////////////////////////

/**
 * Client for talking to an MQTT server using non-blocking
 * methods that allow an operation to run in the background.
 *
 * The location of the server is specified as a URI string with the
 * following schemas supported to specify the type and security used for the
 * connection:
 * @li @em "mqtt://" - A standard (insecure) connection over TCP. (Also,
 * "tcp://")
 * @li @em "mqtts://" - A secure connection using SSL/TLS sockets. (Also
 * "ssl://")
 * @li @em "ws://" - A standard (insecure) WebSocket connection.
 * @li @em "wss:// - A secure websocket connection using SSL/TLS.
 * @li @em "unix://" - A UNIX-domain connection on the local machine. (*nix
 * systems, only)
 *
 * The secure connection types assume that the library was built with
 * SSL/TLS support, otherwise requesting a secure connection will result in
 * an error.
 *
 * The communication methods of this class - `connect()`, `publish()`,
 * `subscribe()`, etc. - are all asynchronous. They create the request for
 * the server, but return immediately, before a response is received back
 * from the server.
 *
 * These methods return a `token` to the caller which is akin to a C++
 * std::future. The caller can keep the Token, then use it later to block
 * until the asynchronous operation is complete and retrieve the result of
 * the operation, including any response from the server.
 *
 * Alternately, the application can choose to set callbacks to be fired when
 * each operation completes. This can be used to create an event-driven
 * architecture, but is more complex in that it forces the user to avoid any
 * blocking operations and manually handle thread synchronization (since
 * the callbacks run in a separate thread managed by the library).
 *
 * Note that the numerous constructors pre-date the current, expansive,
 * `create_options` structure. For a full set of create options, a
 * builder can be used to specify the full set of options, then construct
 * the client with those options, like this:
 *
 * @code
 *     auto createOpts = mqtt::create_options_builder()
 *                        .server_uri(serverURI)
 *                        .send_while_disconnected()
 *                        .max_buffered_messages(25)
 *                        .delete_oldest_messages()
 *                        .finalize();
 *
 *     mqtt::async_client cli(createOpts);
 * @endcode
 */
class async_client : public virtual iasync_client
{
public:
    /** Smart/shared pointer for an object of this class */
    using ptr_t = std::shared_ptr<async_client>;
    /** Type for a thread-safe queue to consume events synchronously */
    using consumer_queue_type = std::unique_ptr<thread_queue<event>>;

    /** Handler type for registering an individual message callback */
    using message_handler = std::function<void(const_message_ptr)>;
    /** Handler type for when a connection is made or lost */
    using connection_handler = std::function<void(const string& cause)>;
    /** Handler type for when a disconnect packet is received */
    using disconnected_handler = std::function<void(const properties&, ReasonCode)>;
    /** Handler for updating connection data before an auto-reconnect. */
    using update_connection_handler = std::function<bool(connect_data&)>;

private:
    /** Lock guard type for this class */
    using guard = std::unique_lock<std::mutex>;
    /** Unique lock type for this class */
    using unique_lock = std::unique_lock<std::mutex>;

    /** Object monitor mutex */
    mutable std::mutex lock_;
    /** The underlying C-lib client. */
    MQTTAsync cli_;
    /** The options used to create the client */
    const create_options createOpts_;
    /** The MQTT protocol version of the connection */
    int mqttVersion_;
    /** A user persistence wrapper (if any) */
    std::unique_ptr<MQTTClient_persistence> persist_{};
    /** Callback supplied by the user (if any) */
    callback* userCallback_{};
    /** Connection handler */
    connection_handler connHandler_;
    /** Connection lost handler */
    connection_handler connLostHandler_;
    /** Disconnected handler */
    disconnected_handler disconnectedHandler_;
    /** Update connect data/options */
    update_connection_handler updateConnectionHandler_;
    /** Message handler */
    message_handler msgHandler_;
    /** Cached options from the last connect */
    connect_options connOpts_;
    /** Copy of connect token (for re-connects) */
    token_ptr connTok_;
    /** A list of tokens that are in play */
    std::list<token_ptr> pendingTokens_;
    /** A list of delivery tokens that are in play */
    std::list<delivery_token_ptr> pendingDeliveryTokens_;
    /** A queue of messages for consumer API */
    consumer_queue_type que_;

    /** Callbacks from the C library */
    static void on_connected(void* context, char* cause);
    static void on_connection_lost(void* context, char* cause);
    static void on_disconnected(
        void* context, MQTTProperties* cprops, MQTTReasonCodes reasonCode
    );
    static int on_message_arrived(
        void* context, char* topicName, int topicLen, MQTTAsync_message* msg
    );
    static void on_delivery_complete(void* context, MQTTAsync_token tok);
    static int on_update_connection(void* context, MQTTAsync_connectData* cdata);

    /** Manage internal list of active tokens */
    friend class token;
    virtual void add_token(token_ptr tok);
    virtual void add_token(delivery_token_ptr tok);
    virtual void remove_token(token* tok) override;
    virtual void remove_token(token_ptr tok) { remove_token(tok.get()); }
    void remove_token(delivery_token_ptr tok) { remove_token(tok.get()); }

    /** Non-copyable */
    async_client() = delete;
    async_client(const async_client&) = delete;
    async_client& operator=(const async_client&) = delete;

    /** Checks a function return code and throws on error. */
    static void check_ret(int rc) {
        if (rc != MQTTASYNC_SUCCESS)
            throw exception(rc);
    }
    /**
     * Create an async_client that can be used to communicate with an MQTT
     * server, which allows for off-line message buffering.
     * This allows the caller to specify a user-defined persistence object,
     * or use no persistence.
     * @throw exception if an argument is invalid
     */
    void create();

public:
    /**
     * Create an async_client that can be used to communicate with an MQTT
     * server.
     * This uses file-based persistence in the specified directory.
     * @param serverURI the address of the server to connect to, specified
     *  				as a URI.
     * @param clientId a client identifier that is unique on the server
     *  			   being connected to
     * @param persistence The persistence that the client should use.
     * @throw exception if an argument is invalid
     */
    explicit async_client(
        const string& serverURI, const string& clientId = string{},
        const persistence_type& persistence = NO_PERSISTENCE
    )
        : createOpts_{serverURI, clientId, persistence} {
        create();
    }
    /**
     * Create an async_client that can be used to communicate with an MQTT
     * server, which allows for off-line message buffering.
     * This uses file-based persistence in the specified directory.
     * @param serverURI the address of the server to connect to, specified
     *  				as a URI.
     * @param clientId a client identifier that is unique on the server
     *  			   being connected to
     * @param maxBufferedMessages the maximum number of messages allowed to
     *  						  be buffered while not connected
     * @param persistence The persistence that the client should use.
     * @throw exception if an argument is invalid
     */
    async_client(
        const string& serverURI, const string& clientId, int maxBufferedMessages,
        const persistence_type& persistence = NO_PERSISTENCE
    )
        : createOpts_{serverURI, clientId, maxBufferedMessages, persistence} {
        create();
    }
    /**
     * Create an async_client that can be used to communicate with an MQTT
     * server, which allows for off-line message buffering.
     * This uses file-based persistence in the specified directory.
     * @param serverURI the address of the server to connect to, specified
     *  				as a URI.
     * @param clientId a client identifier that is unique on the server
     *  			   being connected to
     * @param opts The create options
     * @param persistence The persistence that the client should use.
     * @throw exception if an argument is invalid
     */
    async_client(
        const string& serverURI, const string& clientId, const create_options& opts,
        const persistence_type& persistence = NO_PERSISTENCE
    )
        : createOpts_{serverURI, clientId, opts, persistence} {
        create();
    }
    /**
     * Create an async_client that can be used to communicate with an MQTT
     * server, which allows for off-line message buffering.
     * This allows the caller to specify a user-defined persistence object,
     * or use no persistence.
     * @param opts The create options
     * @throw exception if an argument is invalid
     */
    async_client(const create_options& opts) : createOpts_{opts} { create(); }
    /**
     * Destructor
     */
    ~async_client() override;
    /**
     * Sets a callback listener to use for events that happen
     * asynchronously.
     * @param cb callback receiver which will be invoked for certain
     *  		 asynchronous events
     */
    void set_callback(callback& cb) override;
    /**
     * Stops callbacks.
     * This is not normally called by the application. It should be used
     * cautiously as it may cause the application to lose messages.
     */
    void disable_callbacks() override;
    /**
     * Callback for when a connection is made.
     * @param cb Callback functor for when the connection is made.
     */
    void set_connected_handler(connection_handler cb) /*override*/;
    /**
     * Callback for when a connection is lost.
     * @param cb Callback functor for when the connection is lost.
     */
    void set_connection_lost_handler(connection_handler cb) /*override*/;
    /**
     * Callback for when a disconnect packet is received from the server.
     * @param cb Callback for when the disconnect packet is received.
     */
    void set_disconnected_handler(disconnected_handler cb) /*override*/;
    /**
     * Sets the callback for when a message arrives from the broker.
     * Note that the application can only have one message handler which can
     * be installed individually using this method, or installled as a
     * listener object.
     * @param cb The callback functor to register with the library.
     */
    void set_message_callback(message_handler cb) /*override*/;
    /**
     * Sets a callback to allow the application to update the connection
     * data on automatic reconnects.
     * @param cb The callback functor to register with the library.
     */
    void set_update_connection_handler(update_connection_handler cb);
    /**
     * Connects to an MQTT server using the default options.
     * @return token used to track and wait for the connect to complete. The
     *  	   token will be passed to any callback that has been set.
     * @throw exception for non security related problems
     * @throw security_exception for security related problems
     */
    token_ptr connect() override;
    /**
     * Connects to an MQTT server using the provided connect options.
     * @param options a set of connection parameters that override the
     *  			  defaults.
     * @return token used to track and wait for the connect to complete. The
     *  	   token will be passed to any callback that has been set.
     * @throw exception for non security related problems
     * @throw security_exception for security related problems
     */
    token_ptr connect(connect_options options) override;
    /**
     * Connects to an MQTT server using the specified options.
     * @param options a set of connection parameters that override the
     *  			  defaults.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when the connect
     *  			   completes.
     * @return token used to track and wait for the connect to complete. The
     *  	   token will be passed to any callback that has been set.
     * @throw exception for non security related problems
     * @throw security_exception for security related problems
     */
    token_ptr connect(
        connect_options options, void* userContext, iaction_listener& cb
    ) override;
    /**
     *
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when the connect
     *  			   completes.
     * @return token used to track and wait for the connect to complete. The
     *  	   token will be passed to any callback that has been set.
     * @throw exception for non security related problems
     * @throw security_exception for security related problems
     */
    token_ptr connect(void* userContext, iaction_listener& cb) override {
        return connect(connect_options{}, userContext, cb);
    }
    /**
     * Reconnects the client using options from the previous connect.
     * The client must have previously called connect() for this to work.
     * @return token used to track the progress of the reconnect.
     */
    token_ptr reconnect() override;
    /**
     * Disconnects from the server.
     * @return token used to track and wait for the disconnect to complete.
     *  	   The token will be passed to any callback that has been set.
     * @throw exception for problems encountered while disconnecting
     */
    token_ptr disconnect() override { return disconnect(disconnect_options()); }
    /**
     * Disconnects from the server.
     * @param opts Options for disconnecting.
     * @return token used to track and wait for the disconnect to complete.
     *  	   The token will be passed to any callback that has been set.
     * @throw exception for problems encountered while disconnecting
     */
    token_ptr disconnect(disconnect_options opts) override;
    /**
     * Disconnects from the server.
     * @param timeout the amount of time in milliseconds to allow for
     *  			  existing work to finish before disconnecting. A value
     *  			  of zero or less means the client will not quiesce.
     * @return Token used to track and wait for disconnect to complete. The
     *  	   token will be passed to the callback methods if a callback is
     *  	   set.
     * @throw exception for problems encountered while disconnecting
     */
    token_ptr disconnect(int timeout) override {
        return disconnect(disconnect_options(timeout));
    }
    /**
     * Disconnects from the server.
     * @param timeout the amount of time in milliseconds to allow for
     *  			  existing work to finish before disconnecting. A value
     *  			  of zero or less means the client will not quiesce.
     * @return Token used to track and wait for disconnect to complete. The
     *  	   token will be passed to the callback methods if a callback is
     *  	   set.
     * @throw exception for problems encountered while disconnecting
     */
    template <class Rep, class Period>
    token_ptr disconnect(const std::chrono::duration<Rep, Period>& timeout) {
        // TODO: check range
        return disconnect((int)to_milliseconds_count(timeout));
    }
    /**
     * Disconnects from the server.
     * @param timeout the amount of time in milliseconds to allow for
     *  			  existing work to finish before disconnecting. A value
     *  			  of zero or less means the client will not quiesce.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when the disconnect
     *  			   completes.
     * @return token_ptr Token used to track and wait for disconnect to
     *  	   complete. The token will be passed to the callback methods if
     *  	   a callback is set.
     * @throw exception for problems encountered while disconnecting
     */
    token_ptr disconnect(int timeout, void* userContext, iaction_listener& cb) override;
    /**
     * Disconnects from the server.
     * @param timeout the amount of time in milliseconds to allow for
     *  			  existing work to finish before disconnecting. A value
     *  			  of zero or less means the client will not quiesce.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when the disconnect
     *  			   completes.
     * @return token_ptr Token used to track and wait for disconnect to
     *  	   complete. The token will be passed to the callback methods if
     *  	   a callback is set.
     * @throw exception for problems encountered while disconnecting
     */
    template <class Rep, class Period>
    token_ptr disconnect(
        const std::chrono::duration<Rep, Period>& timeout, void* userContext,
        iaction_listener& cb
    ) {
        // TODO: check range
        return disconnect((int)to_milliseconds_count(timeout), userContext, cb);
    }
    /**
     * Disconnects from the server.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when the disconnect
     *  			   completes.
     * @return token_ptr Token used to track and wait for disconnect to
     *  	   complete. The token will be passed to the callback methods if
     *  	   a callback is set.
     * @throw exception for problems encountered while disconnecting
     */
    token_ptr disconnect(void* userContext, iaction_listener& cb) override {
        return disconnect(0L, userContext, cb);
    }
    /**
     * Returns the delivery token for the specified message ID.
     * @return delivery_token
     */
    delivery_token_ptr get_pending_delivery_token(int msgID) const override;
    /**
     * Returns the delivery tokens for any outstanding publish operations.
     * @return delivery_token[]
     */
    std::vector<delivery_token_ptr> get_pending_delivery_tokens() const override;
    /**
     * Returns the client ID used by this client.
     * @return The client ID used by this client.
     */
    string get_client_id() const override { return createOpts_.get_client_id(); }
    /**
     * Returns the address of the server used by this client.
     * @return The server's address, as a URI String.
     */
    string get_server_uri() const override { return createOpts_.get_server_uri(); }
    /**
     * Gets the MQTT version used by the client.
     * @return The MQTT version used by the client
     *   @li MQTTVERSION_DEFAULT (0) = default: start with 3.1.1, and if
     *       that fails, fall back to 3.1
     *   @li MQTTVERSION_3_1 (3) = only try version 3.1
     *   @li MQTTVERSION_3_1_1 (4) = only try version 3.1.1
     *   @li MQTTVERSION_5 (5) = only try version 5
     */
    int mqtt_version() const noexcept { return mqttVersion_; }
    /**
     * Gets a copy of the connect options that were last used in a request
     * to connect to the broker.
     * @returns The last connect options that were used.
     */
    connect_options get_connect_options() const {
        guard g(lock_);
        return connOpts_;
    }
    /**
     * Determines if this client is currently connected to the server.
     * @return true if connected, false otherwise.
     */
    bool is_connected() const override { return to_bool(MQTTAsync_isConnected(cli_)); }
    /**
     * Publishes a message to a topic on the server
     * @param topic The topic to deliver the message to
     * @param payload the bytes to use as the message payload
     * @param n the number of bytes in the payload
     * @param qos the Quality of Service to deliver the message at. Valid
     *  		  values are 0, 1 or 2.
     * @param retained whether or not this message should be retained by the
     *  			   server.
     * @return token used to track and wait for the publish to complete. The
     *  	   token will be passed to callback methods if set.
     */
    delivery_token_ptr publish(
        string_ref topic, const void* payload, size_t n, int qos, bool retained,
        const properties& props = properties()
    ) override;
    /**
     * Publishes a message to a topic on the server
     * @param topic The topic to deliver the message to
     * @param payload the bytes to use as the message payload
     * @param n the number of bytes in the payload
     * @return token used to track and wait for the publish to complete. The
     *  	   token will be passed to callback methods if set.
     */
    delivery_token_ptr publish(string_ref topic, const void* payload, size_t n) override {
        return publish(
            std::move(topic), payload, n, message::DFLT_QOS, message::DFLT_RETAINED
        );
    }
    /**
     * Publishes a message to a topic on the server
     * @param topic The topic to deliver the message to
     * @param payload the bytes to use as the message payload
     * @param qos the Quality of Service to deliver the message at. Valid
     *  		  values are 0, 1 or 2.
     * @param retained whether or not this message should be retained by the
     *  			   server.
     * @return token used to track and wait for the publish to complete. The
     *  	   token will be passed to callback methods if set.
     */
    delivery_token_ptr publish(
        string_ref topic, binary_ref payload, int qos, bool retained,
        const properties& props = properties()
    ) override;
    /**
     * Publishes a message to a topic on the server
     * @param topic The topic to deliver the message to
     * @param payload the bytes to use as the message payload
     * @return token used to track and wait for the publish to complete. The
     *  	   token will be passed to callback methods if set.
     */
    delivery_token_ptr publish(string_ref topic, binary_ref payload) override {
        return publish(
            std::move(topic), std::move(payload), message::DFLT_QOS, message::DFLT_RETAINED
        );
    }
    /**
     * Publishes a message to a topic on the server
     * @param topic The topic to deliver the message to
     * @param payload the bytes to use as the message payload
     * @param n the number of bytes in the payload
     * @param qos the Quality of Service to deliver the message at. Valid
     *  		  values are 0, 1 or 2.
     * @param retained whether or not this message should be retained by the
     *  			   server.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb Listener callback object
     * @return token used to track and wait for the publish to complete. The
     *  	   token will be passed to callback methods if set.
     */
    delivery_token_ptr publish(
        string_ref topic, const void* payload, size_t n, int qos, bool retained,
        void* userContext, iaction_listener& cb
    ) override;
    /**
     * Publishes a message to a topic on the server Takes an Message
     * message and delivers it to the server at the requested quality of
     * service.
     * @param msg the message to deliver to the server
     * @return token used to track and wait for the publish to complete. The
     *  	   token will be passed to callback methods if set.
     */
    delivery_token_ptr publish(const_message_ptr msg) override;
    /**
     * Publishes a message to a topic on the server.
     * @param msg the message to deliver to the server
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback optional listener that will be notified when message
     *  			   delivery has completed to the requested quality of
     *  			   service
     * @return token used to track and wait for the publish to complete. The
     *  	   token will be passed to callback methods if set.
     */
    delivery_token_ptr publish(
        const_message_ptr msg, void* userContext, iaction_listener& cb
    ) override;
    /**
     * Subscribe to a topic, which may include wildcards.
     * @param topicFilter the topic to subscribe to, which can include
     *  				  wildcards.
     * @param qos The quality of service for the subscription
     * @param opts The MQTT v5 subscribe options for the topic
     * @param props The MQTT v5 properties.
     * @return token used to track and wait for the subscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr subscribe(
        const string& topicFilter, int qos,
        const subscribe_options& opts = subscribe_options(),
        const properties& props = properties()
    ) override;
    /**
     * Subscribe to a topic, which may include wildcards.
     * @param topicFilter the topic to subscribe to, which can include
     *  				  wildcards.
     * @param qos The maximum quality of service at which to subscribe.
     *  		  Messages published at a lower quality of service will be
     *  		  received at the published QoS. Messages published at a
     *  		  higher quality of service will be received using the QoS
     *  		  specified on the subscribe.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb listener that will be notified when subscribe has completed
     * @param opts The MQTT v5 subscribe options for the topic
     * @param props The MQTT v5 properties.
     * @return token used to track and wait for the subscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr subscribe(
        const string& topicFilter, int qos, void* userContext, iaction_listener& cb,
        const subscribe_options& opts = subscribe_options(),
        const properties& props = properties()
    ) override;
    /**
     * Subscribe to multiple topics, each of which may include wildcards.
     * @param topicFilters The collection of topic filters to subscribe to,
     *                     any of which can include wildcards
     * @param qos The maximum quality of service at which to subscribe.
     *  		  Messages published at a lower quality of service will be
     *  		  received at the published QoS. Messages published at a
     *  		  higher quality of service will be received using the QoS
     *  		  specified on the subscribe.
     * @param opts The MQTT v5 subscribe options (one for each topic)
     * @param props The MQTT v5 properties.
     * @return token used to track and wait for the subscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr subscribe(
        const_string_collection_ptr topicFilters, const qos_collection& qos,
        const std::vector<subscribe_options>& opts = std::vector<subscribe_options>(),
        const properties& props = properties()
    ) override;
    /**
     * Subscribes to multiple topics, each of which may include wildcards.
     * @param topicFilters The collection of topic filters to subscribe to,
     *                     any of which can include wildcards
     * @param qos The maximum quality of service at which to subscribe.
     *  		  Messages published at a lower quality of service will be
     *  		  received at the published QoS. Messages published at a
     *  		  higher quality of service will be received using the QoS
     *  		  specified on the subscribe.
     * @param userContext Optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb listener that will be notified when subscribe has completed
     * @param opts The MQTT v5 subscribe options (one for each topic)
     * @param props The MQTT v5 properties.
     * @return token used to track and wait for the subscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr subscribe(
        const_string_collection_ptr topicFilters, const qos_collection& qos,
        void* userContext, iaction_listener& cb,
        const std::vector<subscribe_options>& opts = std::vector<subscribe_options>(),
        const properties& props = properties()
    ) override;
    /**
     * Requests the server unsubscribe the client from a topic.
     * @param topicFilter The topic to unsubscribe from. It must match a
     *  				  topicFilter specified on an earlier subscribe.
     * @param props The MQTT v5 properties.
     * @return token Used to track and wait for the unsubscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr unsubscribe(
        const string& topicFilter, const properties& props = properties()
    ) override;
    /**
     * Requests the server unsubscribe the client from one or more topics.
     * @param topicFilters One or more topics to unsubscribe from. Each
     *  				   topicFilter must match one specified on an
     *  				   earlier subscribe.
     * @param props The MQTT v5 properties.
     * @return token used to track and wait for the unsubscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr unsubscribe(
        const_string_collection_ptr topicFilters, const properties& props = properties()
    ) override;
    /**
     * Requests the server unsubscribe the client from one or more topics.
     * @param topicFilters One or more topics to unsubscribe from. Each
     *  				   topicFilter must match one specified on an
     *  				   earlier subscribe.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb listener that will be notified when unsubscribe has
     *  		 completed
     * @param props The MQTT v5 properties.
     * @return token used to track and wait for the unsubscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr unsubscribe(
        const_string_collection_ptr topicFilters, void* userContext, iaction_listener& cb,
        const properties& props = properties()
    ) override;
    /**
     * Requests the server unsubscribe the client from a topics.
     * @param topicFilter the topic to unsubscribe from. It must match a
     *  				  topicFilter specified on an earlier subscribe.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb listener that will be notified when unsubscribe has
     *  		 completed
     * @param props The MQTT v5 properties.
     * @return token used to track and wait for the unsubscribe to complete.
     *  	   The token will be passed to callback methods if set.
     */
    token_ptr unsubscribe(
        const string& topicFilter, void* userContext, iaction_listener& cb,
        const properties& props = properties()
    ) override;
    /**
     * Start consuming messages.
     *
     * This initializes the client to receive messages through a queue that
     * can be read synchronously.
     *
     * Normally this should be called _before_ connecting the client to the
     * broker, in order to have the consumer queue in place in the event
     * that the immediately starts sending messages (such as any retained
     * messages) while the client is still in the context of the connect
     * call.
     *
     * This _must_ also be called before calling any 'consume_message' or
     * "'consume_event' methods.
     *
     * Internally, this just creates a thread-safe queue for `mqtt::event`
     * objects, then hooks into the message and state-change callback to
     * push events into the queue in the order received.
     */
    void start_consuming() override;
    /**
     * Stop consuming messages.
     *
     * This shuts down the internal callback and closes the internal
     * consumer queue. Any remaining messages and events can be read until
     * the queue is emptied, but nothing further will be added to it.
     * This will also wake up any thread waiting on the queue.
     */
    void stop_consuming() override;
    /**
     * This clears the consumer queue, discarding any pending event.
     */
    void clear_consumer() override {
        if (que_)
            que_->clear();
    }
    /**
     * Determines if the consumer queue has been closed.
     * Once closed, any events in the queue can still be read, but no new
     * events can be added to it.
     * @return @true if the consumer queue has been closed, @false
     *         otherwise.
     */
    bool consumer_closed() noexcept override { return !que_ || que_->closed(); }
    /**
     * Determines if the consumer queue is "done" (closed and empty).
     * Once the queue is done, no more events can be added or removed from
     * the queue.
     * @return @true if the consumer queue is closed and empty, @false
     *         otherwise.
     */
    bool consumer_done() noexcept override { return !que_ || que_->done(); }
    /**
     * Gets the number of events available for immediate consumption.
     * Note that this retrieves the number of "raw" events, not messages,
     * e.g. may include a connected_event which is not returned by try_consume_message().
     * When polling the queue from multiple threads, prefer using try_consume_event(),
     * as the event count may change between checking the size and actual retrieval.
     * @return the number of events in the queue.
     */
    std::size_t consumer_queue_size() const override { return (que_) ? que_->size() : 0; }
    /**
     * Read the next client event from the queue.
     * This blocks until a new message arrives.
     * If the consumer queue is closed, this returns a shutdown event.
     * @return The client event.
     */
    event consume_event() override;
    /**
     * Try to read the next client event without blocking.
     * @param evt Pointer to the value to receive the event
     * @return @em true if an event was read, @em false if no
     *  	   event was available.
     */
    bool try_consume_event(event* evt) override;
    /**
     * Waits a limited time for a client event to appear.
     * @param evt Pointer to the value to receive the event.
     * @param relTime The maximum amount of time to wait for an event.
     * @return @em true if an event was read, @em false if a timeout
     *  	   occurred.
     */
    template <typename Rep, class Period>
    bool try_consume_event_for(
        event* evt, const std::chrono::duration<Rep, Period>& relTime
    ) {
        if (!que_)
            throw mqtt::exception(-1, "Consumer not started");

        try {
            return que_->try_get_for(evt, relTime);
        }
        catch (queue_closed&) {
            *evt = event{shutdown_event{}};
            return true;
        }
    }
    /**
     * Waits a limited time for a client event to arrive.
     * @param relTime The maximum amount of time to wait for an event.
     * @return The event that was received. It will contain empty message on
     *  	   timeout.
     */
    template <typename Rep, class Period>
    event try_consume_event_for(const std::chrono::duration<Rep, Period>& relTime) {
        event evt;
        try {
            que_->try_get_for(&evt, relTime);
        }
        catch (queue_closed&) {
            evt = event{shutdown_event{}};
        }
        return evt;
    }
    /**
     * Waits until a specific time for a client event to appear.
     * @param evt Pointer to the value to receive the event.
     * @param absTime The time point to wait until, before timing out.
     * @return @em true if an event was recceived, @em false if a timeout
     *  	   occurred.
     */
    template <class Clock, class Duration>
    bool try_consume_event_until(
        event* evt, const std::chrono::time_point<Clock, Duration>& absTime
    ) {
        if (!que_)
            throw mqtt::exception(-1, "Consumer not started");

        try {
            return que_->try_get_until(evt, absTime);
        }
        catch (queue_closed&) {
            *evt = event{shutdown_event{}};
            return true;
        }
    }
    /**
     * Waits until a specific time for a client event to appear.
     * @param absTime The time point to wait until, before timing out.
     * @return The event that was received. It will contain empty message on
     *  	   timeout.
     */
    template <class Clock, class Duration>
    event try_consume_event_until(const std::chrono::time_point<Clock, Duration>& absTime) {
        event evt;
        try {
            que_->try_get_until(&evt, absTime);
        }
        catch (queue_closed&) {
            evt = event{shutdown_event{}};
        }
        return evt;
    }
    /**
     * Read the next message from the queue.
     * This blocks until a new message arrives or until a disconnect or
     * shutdown occurs.
     * @return The message and topic.
     */
    const_message_ptr consume_message() override;
    /**
     * Try to read the next message from the queue without blocking.
     * @param msg Pointer to the value to receive the message
     * @return @em true is a message was read, @em false if no message was
     *  	   available.
     */
    bool try_consume_message(const_message_ptr* msg) override;
    /**
     * Waits a limited time for a message to arrive.
     * @param msg Pointer to the value to receive the message
     * @param relTime The maximum amount of time to wait for a message.
     * @return @em true if a message was read, @em false if a timeout
     *  	   occurred.
     */
    template <typename Rep, class Period>
    bool try_consume_message_for(
        const_message_ptr* msg, const std::chrono::duration<Rep, Period>& relTime
    ) {
        if (!que_)
            throw mqtt::exception(-1, "Consumer not started");

        event evt;

        while (true) {
            if (!try_consume_event_for(&evt, relTime))
                return false;

            if (const auto* pval = evt.get_message_if()) {
                *msg = std::move(*pval);
                break;
            }

            if (evt.is_any_disconnect()) {
                *msg = const_message_ptr{};
                break;
            }
        }
        return true;
    }
    /**
     * Waits a limited time for a message to arrive.
     * @param relTime The maximum amount of time to wait for a message.
     * @return A shared pointer to the message that was received. It will be
     *  	   empty on timeout.
     */
    template <typename Rep, class Period>
    const_message_ptr try_consume_message_for(
        const std::chrono::duration<Rep, Period>& relTime
    ) {
        const_message_ptr msg;
        this->try_consume_message_for(&msg, relTime);
        return msg;
    }
    /**
     * Waits until a specific time for a message to appear.
     * @param msg Pointer to the value to receive the message
     * @param absTime The time point to wait until, before timing out.
     * @return @em true if a message was read, @em false if a timeout
     *  	   occurred.
     */
    template <class Clock, class Duration>
    bool try_consume_message_until(
        const_message_ptr* msg, const std::chrono::time_point<Clock, Duration>& absTime
    ) {
        if (!que_)
            throw mqtt::exception(-1, "Consumer not started");

        event evt;

        while (true) {
            if (!try_consume_event_until(&evt, absTime))
                return false;

            if (const auto* pval = evt.get_message_if()) {
                *msg = std::move(*pval);
                break;
            }

            if (!evt.is_any_disconnect()) {
                *msg = const_message_ptr{};
                break;
            }
        }

        return true;
    }
    /**
     * Waits until a specific time for a message to appear.
     * @param absTime The time point to wait until, before timing out.
     * @return The message, if read, an empty pointer if not.
     */
    template <class Clock, class Duration>
    const_message_ptr try_consume_message_until(
        const std::chrono::time_point<Clock, Duration>& absTime
    ) {
        const_message_ptr msg;
        this->try_consume_message_until(&msg, absTime);
        return msg;
    }
};

/** Smart/shared pointer to an asynchronous MQTT client object */
using async_client_ptr = async_client::ptr_t;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_async_client_h
