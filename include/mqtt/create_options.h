/////////////////////////////////////////////////////////////////////////////
/// @file create_options.h
/// Declaration of MQTT create_options class
/// @date Oct 17, 2020
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2020-2025 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_create_options_h
#define __mqtt_create_options_h

#include <variant>

#include "MQTTAsync.h"
#include "mqtt/iclient_persistence.h"
#include "mqtt/types.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/** An empty type that can be used as a `persistent_type` variant option. */
struct no_persistence
{
};

/** A constant used to indicate that no persistence is desired */
constexpr no_persistence NO_PERSISTENCE{};

/**
 * A variant for the different type of persistence:
 * @li @em no_persistence: Any object of this type indicates no persistence
 *     is desired.
 * @li @em string: Indicates file persistence. The string specifies the
 *     directory for the persistence store.
 * @li @em iclient_persistence*: User-defined persistence
 */
using persistence_type = std::variant<no_persistence, string, iclient_persistence*>;

/////////////////////////////////////////////////////////////////////////////

/**
 * The set of options for constructing a client object.
 *
 * Note that the numerous, incomplete set of constructors pre-date the
 * current, expanded, options structure. For a full set of create options, a
 * builder can be used to specify the options, then construct the client
 * with those options, like this:
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
class create_options
{
    /** The underlying C options */
    MQTTAsync_createOptions opts_ MQTTAsync_createOptions_initializer5;

    /** The address of the server to connect to, specified as a URI */
    string serverURI_{};

    /** A client identifier that is unique on the server */
    string clientId_{};

    /** The persistence for the client */
    persistence_type persistence_{};

    /** The client and tests have special access */
    friend class async_client;
    friend class create_options_builder;

public:
    /** Smart/shared pointer to an object of this class. */
    using ptr_t = std::shared_ptr<create_options>;
    /** Smart/shared pointer to a const object of this class. */
    using const_ptr_t = std::shared_ptr<const create_options>;

    /**
     * Default set of client create options.
     */
    create_options() {}
    /**
     * Default create options for the specified version of MQTT.
     * @param mqttVersion The MQTT version used to create the client.
     */
    explicit create_options(int mqttVersion) : create_options() {
        opts_.MQTTVersion = mqttVersion;
    }
    /**
     * Default create options, but with off-line buffering enabled.
     * @param mqttVersion The MQTT version used to create the client.
     * @param maxBufferedMessages the maximum number of messages allowed to
     *  						  be buffered while not connected
     */
    create_options(int mqttVersion, int maxBufferedMessages);
    /**
     * Create options for the specified server and client ID, with optional
     * persistence.
     * This allows the caller to specify a user-defined persistence object,
     * or use no persistence.
     * @param serverURI the address of the server to connect to, specified
     *  				as a URI.
     * @param clientId a client identifier that is unique on the server
     *  			   being connected to
     * @param persistence The desired persistence structure.
     * @throw exception if an argument is invalid
     */
    explicit create_options(
        const string& serverURI, const string& clientId = string{},
        const persistence_type& persistence = NO_PERSISTENCE
    )
        : serverURI_{serverURI}, clientId_{clientId}, persistence_{persistence} {}
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
    create_options(
        const string& serverURI, const string& clientId, int maxBufferedMessages,
        const persistence_type& persistence = NO_PERSISTENCE
    )
        : serverURI_{serverURI}, clientId_{clientId}, persistence_{persistence} {
        opts_.maxBufferedMessages = maxBufferedMessages;
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
    create_options(
        const string& serverURI, const string& clientId, const create_options& opts,
        const persistence_type& persistence
    )
        : opts_{opts.opts_},
          serverURI_{serverURI},
          clientId_{clientId},
          persistence_{persistence} {}
    /**
     * Copy constructor.
     * @param opts The other options.
     */
    create_options(const create_options& opts)
        : opts_{opts.opts_},
          serverURI_{opts.serverURI_},
          clientId_{opts.clientId_},
          persistence_{opts.persistence_} {}
    /**
     * Move constructor.
     * @param opts The other options.
     */
    create_options(create_options&& opts)
        : opts_{opts.opts_},
          serverURI_{std::move(opts.serverURI_)},
          clientId_{std::move(opts.clientId_)},
          persistence_{std::move(opts.persistence_)} {}

    create_options& operator=(const create_options& rhs);
    create_options& operator=(create_options&& rhs);
    /**
     * Set the address of the server to connect to, specified as a URI
     * @param serverURI The URI of the server.
     */
    void set_server_uri(const string& serverURI) { serverURI_ = serverURI; };
    /**
     * Get the address of the server to connect to, specified as a URI.
     * @return The URI of the server.
     */
    const string& get_server_uri() const noexcept { return serverURI_; };
    /**
     * Set the client identifier.
     * @param clientId The client identifier.
     */
    void set_client_id(const string& clientId) { clientId_ = clientId; }
    /**
     * Get the client identifier.
     * @return The client identifier.
     */
    const string& get_client_id() const noexcept { return clientId_; }
    /**
     * Set the persistence for the client.
     * @param persistence The persistence for the client
     */
    void set_persistence(const persistence_type& persistence) { persistence_ = persistence; }
    /**
     * Get the persistence for the client.
     * @return The persistence for the client
     */
    const persistence_type& get_persistence() const noexcept { return persistence_; }
    /**
     * Gets whether the client will accept message to publish while
     * disconnected.
     */
    bool get_send_while_disconnected() const { return to_bool(opts_.sendWhileDisconnected); }
    /**
     * Sets whether the client will accept message to publish while
     * disconnected.
     *
     * @param on @em true to allow the application to publish messages while
     *  		 disconnected, @em false returns an error on publish if
     *  		 disconnected.
     * @param anyTime If @em true, allows you to publish messages before the
     *  			  first successful connection.
     */
    void set_send_while_disconnected(bool on, bool anyTime = false) {
        opts_.sendWhileDisconnected = to_int(on);
        opts_.allowDisconnectedSendAtAnyTime = to_int(anyTime);
    }
    /**
     * Gets the maximum number of offline buffered messages.
     * @return The maximum number of offline buffered messages.
     */
    int get_max_buffered_messages() const { return opts_.maxBufferedMessages; }
    /**
     * Sets the maximum number of offline buffered messages.
     * @param n The maximum number of offline buffered messages.
     */
    void set_max_buffered_messages(int n) { opts_.maxBufferedMessages = n; }
    /**
     * Gets the MQTT version used to create the client.
     * @return The MQTT version used to create the client.
     */
    int mqtt_version() const { return opts_.MQTTVersion; }
    /**
     * Sets the MQTT version used to create the client.
     * @param ver The MQTT version used to create the client.
     */
    void set_mqtt_version(int ver) { opts_.MQTTVersion = ver; }
    /**
     * Whether the oldest messages are deleted when the output buffer is
     * full.
     *
     * @return @em true if the oldest messages should be deleted when the
     *  	   output buffer is full, @em false if the new messages should
     *  	   be dropped when the buffer is full.
     */
    bool get_delete_oldest_messages() const { return to_bool(opts_.deleteOldestMessages); }
    /**
     * Determines what to do when the maximum number of buffered messages is
     * reached: delete the oldest messages rather than the newest
     * @param on @em true When the output queue is full, delete the oldest
     *  		 message, @em false drop the newest message being added.
     */
    void set_delete_oldest_messages(bool on) { opts_.deleteOldestMessages = to_int(on); }
    /**
     * Whether the messages will be restored from persistence or the store
     * will be cleared.
     * @return @em true if the messages will be restored from persistence,
     *  	   @em false if the persistence store will be cleared.
     */
    bool get_restore_messages() const { return to_bool(opts_.restoreMessages); }
    /**
     * Determine whether to restore messages from persistence or clear the
     * persistence store.
     * @param on @em true to restore messages from persistence, @em false to
     *  		 clear the persistence store.
     */
    void set_restore_messages(bool on) { opts_.restoreMessages = to_int(on); }
    /**
     * Whether to persist QoS 0 messages.
     *
     * @return @em true if QoS 0 messages are persisted, @em false if not.
     */
    bool get_persist_qos0() const { return to_bool(opts_.persistQoS0); }
    /**
     * Determine whether to persist QoS 0 messages.
     *
     * @param on @em true if QoS 0 messages are persisted, @em false if not.
     */
    void set_persist_qos0(bool on) { opts_.persistQoS0 = to_int(on); }
};

/** Smart/shared pointer to a connection options object. */
using create_options_ptr = create_options::ptr_t;

/////////////////////////////////////////////////////////////////////////////

/**
 * Builder class to generate the create options.
 */
class create_options_builder
{
    /** The underlying options */
    create_options opts_;

public:
    /** This class */
    using self = create_options_builder;
    /**
     * Default constructor.
     */
    create_options_builder() {}
    /**
     * Set the server URI.
     * @param serverURI The address of the server to connect to, specified
     *  			  as a URI
     */
    auto server_uri(const string& serverURI) -> self& {
        opts_.set_server_uri(serverURI);
        return *this;
    }
    /**
     * Sets the client ID.
     * @param clientId A client identifier that is unique on the server
     */
    auto client_id(const string& clientId) -> self& {
        opts_.set_client_id(clientId);
        return *this;
    }
    /**
     * Sets the persistence.
     * @param persistence The persistence the client should use.
     */
    auto persistence(const persistence_type& persistence) -> self& {
        opts_.set_persistence(persistence);
        return *this;
    }
    /**
     * Sets whether the client will accept message to publish while
     * disconnected.
     *
     * @param on @em true to allow the application to publish messages while
     *  		 disconnected, @em false returns an error on publish if
     *  		 disconnected.
     * @param anyTime If @em true, allows you to publish messages before the
     *  			  first successful connection.
     * @return A reference to this object.
     */
    auto send_while_disconnected(bool on = true, bool anyTime = false) -> self& {
        opts_.opts_.sendWhileDisconnected = to_int(on);
        opts_.opts_.allowDisconnectedSendAtAnyTime = to_int(anyTime);
        return *this;
    }
    /**
     * Sets the maximum number of offline buffered messages.
     * @param n The maximum number of offline buffered messages.
     * @return A reference to this object.
     */
    auto max_buffered_messages(int n) -> self& {
        opts_.opts_.maxBufferedMessages = n;
        return *this;
    }
    /**
     * Sets the MQTT version used to create the client.
     * @param ver The MQTT version used to create the client.
     */
    auto mqtt_version(int ver) -> self& {
        opts_.opts_.MQTTVersion = ver;
        return *this;
    }
    /**
     * Determines what to do when the maximum number of buffered messages is
     * reached: delete the oldest messages rather than the newest.
     * @param on @em true When the output queue is full, delete the oldest
     *  		 message, @em false drop the newest message being added.
     * @return A reference to this object.
     */
    auto delete_oldest_messages(bool on = true) -> self& {
        opts_.opts_.deleteOldestMessages = to_int(on);
        return *this;
    }
    /**
     * Determines whether to restore persisted messages or clear the
     * persistence store. (Defaults true)
     *
     * @param on @em true to restore persisted messages, @em false to clear
     *  		 the persistence store.
     * @return A reference to this object.
     */
    auto restore_messages(bool on = true) -> self& {
        opts_.opts_.restoreMessages = to_int(on);
        return *this;
    }
    /**
     * Whether to persist QoS 0 messages. (Defaults true)
     *
     * @param on @em true persist QoS 0 messages, @em false, don't.
     * @return A reference to this object
     */
    auto persist_qos0(bool on = true) -> self& {
        opts_.opts_.persistQoS0 = to_int(on);
        return *this;
    }
    /**
     * Finish building the options and return them.
     * @return The option struct as built.
     */
    create_options finalize() { return opts_; }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_create_options_h
