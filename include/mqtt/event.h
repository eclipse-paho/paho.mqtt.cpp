/////////////////////////////////////////////////////////////////////////////
/// @file event.h
/// Declaration of MQTT event-related classes
/// @date July 6, 2024
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2024 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_event_h
#define __mqtt_event_h

#include <variant>

#include "mqtt/message.h"
#include "mqtt/properties.h"
#include "mqtt/reason_code.h"
#include "mqtt/types.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/** Event for when the client is connected/reconnected */
struct connected_event
{
    string cause;
};

/** Event for when the connection is lost */
struct connection_lost_event
{
    string cause;
};

/** Event for when we receive a DISCONNECT packet from the server */
struct disconnected_event
{
    properties props;
    ReasonCode reasonCode;
};

/** Event for when the consumer queue is shutdown from another thread */
struct shutdown_event
{
};

/* Event for when a message arrives is just a message pointer */

/////////////////////////////////////////////////////////////////////////////

/**
 * An MQTT event.
 *
 * This is used by the client consumer to pass events and state changes from
 * the client to the application without the need for any additional
 * callbacks or client queries.
 *
 * Each instance carries the relevant data for specific event that caused
 * it. For example an incoming message event contains a shared pointer to
 * the message that arrived.
 *
 * The supported event types are:
 * @li **message** A message arrived from the server.
 * @li **connected** The client connected. If the client was configured for
 * automatic reconnects, this can be from a reconnection. (No data)
 * @li **connection lost** The client lost the connection. (No data)
 * @li **disconnected** (v5) The client received a DISCONNECT packet from
 * the server. This includes the reason code and properties for the
 * disconnect.
 */
class event
{
public:
    /** The variant type for any possible event. */
    using event_type = std::variant<
        const_message_ptr, connected_event, connection_lost_event, disconnected_event,
        shutdown_event>;

private:
    event_type evt_{};

public:
    /**
     * Constructs an empty event.
     * This shows as a message, but the message pointer is null.
     */
    event() {}
    /**
     * Constructs an event from an event type variant.
     * @param evt The event type variant.
     */
    event(event_type evt) : evt_{std::move(evt)} {}
    /**
     * Constructs a message event.
     * @param msg A shared message pointer.
     */
    event(message_ptr msg) : evt_{std::move(msg)} {}
    /**
     * Constructs a message event.
     * @param msg A shared const message pointer.
     */
    event(const_message_ptr msg) : evt_{std::move(msg)} {}
    /**
     * Constructs a 'connected' event.
     * @param evt A connected event.
     */
    event(connected_event evt) : evt_{std::move(evt)} {}
    /**
     * Constructs a 'connection lost' event.
     * @param evt A connection lost event.
     */
    event(connection_lost_event evt) : evt_{std::move(evt)} {}
    /**
     * Constructs a 'disconnected' event.
     * @param evt A disconnected event.
     */
    event(disconnected_event evt) : evt_{std::move(evt)} {}
    /**
     * Constructs a 'shutdown' event.
     * @param evt A shutdown event.
     */
    event(shutdown_event evt) : evt_{std::move(evt)} {}
    /**
     * Copy constructor.
     * @param evt The event to copy.
     */
    event(const event& evt) : evt_{evt.evt_} {}
    /**
     * Move constructor.
     * @param evt The event to move.
     */
    event(event&& evt) : evt_{std::move(evt.evt_)} {}
    /**
     * Assignment from an event type variant.
     * @param evt The event type variant.
     * @return A reference to this object.
     */
    event& operator=(event_type evt) {
        evt_ = std::move(evt);
        return *this;
    }
    /**
     * Copy assignment.
     * @param rhs The event to copy.
     * @return A reference to this object.
     */
    event& operator=(const event& rhs) {
        if (&rhs != this)
            evt_ = rhs.evt_;
        return *this;
    }
    /**
     * Move assignment.
     * @param rhs The event to move.
     * @return A reference to this object.
     */
    event& operator=(event&& rhs) {
        if (&rhs != this)
            evt_ = std::move(rhs.evt_);
        return *this;
    }
    /**
     * Determines if this event is an incoming message.
     * @return @em true if this event is an incoming message, @em false
     *         otherwise.
     */
    bool is_message() const { return std::holds_alternative<const_message_ptr>(evt_); }
    /**
     * Determines if this event is a client (re)connection.
     * @return @em true if this event is a client connection, @em false
     *         otherwise.
     */
    bool is_connected() const { return std::holds_alternative<connected_event>(evt_); }
    /**
     * Determines if this event is a client connection lost.
     * @return @em true if this event is a client connection lost, @em false
     *         otherwise.
     */
    bool is_connection_lost() const {
        return std::holds_alternative<connection_lost_event>(evt_);
    }
    /**
     * Determines if this event is a client disconnected.
     * @return @em true if this event is a client disconnected, @em false
     *         otherwise.
     */
    bool is_disconnected() const { return std::holds_alternative<disconnected_event>(evt_); }
    /**
     * Determines if this event is an internal shutdown request.
     * @return @em true if this event is a shutdown request, @em false
     *         otherwise.
     */
    bool is_shutdown() const { return std::holds_alternative<disconnected_event>(evt_); }
    /**
     * Determines if this is any type of client disconnect or shutdown.
     * @return @em true if this event is any type of client disconnect such
     *         as a 'connection lost', 'disconnected', or shutdown event.
     */
    bool is_any_disconnect() const {
        return std::holds_alternative<connection_lost_event>(evt_) ||
               std::holds_alternative<disconnected_event>(evt_) ||
               std::holds_alternative<shutdown_event>(evt_);
    }
    /**
     * Gets the message from the event, iff this is a message event.
     * @return A message pointer, if this is a message event.
     * @throw std::bad_variant_access if this is not a 'message' event.
     */
    const_message_ptr get_message() { return std::get<const_message_ptr>(evt_); }
    /**
     * Gets the underlying information for a disconnected event iff this is
     * a 'disconnected' event.
     * This contains the reason code and properties that the server sent in
     * the DISCONNECT packet.
     * @return The disconnected event object containing information about
     *         why the server disconnected.
     * @throw std::bad_variant_access if this is not a 'disconnected' event.
     */
    disconnected_event get_disconnected() { return std::get<disconnected_event>(evt_); }
    /**
     * Gets a pointer to the message in the event, iff this is a message
     * event.
     * @return A pointer to a message pointer, if this is a message event.
     *         Returns nulltr if this is not a message event.
     */
    constexpr std::add_pointer_t<const_message_ptr> get_message_if() noexcept {
        return std::get_if<const_message_ptr>(&evt_);
    }
    /**
     * Gets a pointer the underlying information for a disconnected event,
     * iff this is a 'disconnected' event.
     * This contains the reason code and properties that the server sent in
     * the DISCONNECT packet.
     * @return The disconnected event object containing information about
     *         why the server disconnected.
     * @throw std::bad_variant_access if this is not a 'disconnected' event.
     */
    constexpr std::add_pointer_t<disconnected_event> get_disconnected_if() noexcept {
        return std::get_if<disconnected_event>(&evt_);
    }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_event_h
