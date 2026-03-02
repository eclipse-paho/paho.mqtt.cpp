/////////////////////////////////////////////////////////////////////////////
/// @file server_response.h
/// Declaration of MQTT server response classes.
/// @date July 26, 2019
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2019-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_server_response_h
#define __mqtt_server_response_h

#include <iostream>

#include "MQTTAsync.h"
#include "mqtt/properties.h"
#include "mqtt/types.h"

namespace mqtt {

/**
 * Base class for responses from the server.
 */
class server_response
{
    /** The properties from the acknowledge */
    properties props_;

public:
    /**
     * Creates a response with empty property list.
     */
    server_response() {}
    /**
     * Creates a server response with the specified properties.
     * @param props The properties in the response.
     */
    server_response(const properties& props) : props_{props} {}
    /**
     * Creates a server response with the specified properties.
     * @param props The properties in the response.
     */
    server_response(properties&& props) : props_{std::move(props)} {}
    /**
     * Virtual destructor.
     */
    virtual ~server_response() {}
    /**
     * Gets the properties from the response.
     * @return The properties from the response.
     */
    const properties& get_properties() const noexcept { return props_; }
};

/**
 * Response for a connect request.
 */
class connect_response : public server_response
{
    /** The connection string of the server */
    string serverURI_;
    /** The version of MQTT being used */
    int mqttVersion_;
    /** The session present flag returned from the server */
    bool sessionPresent_;

    friend class token;

    /**
     * Create v5 connect response.
     * @param rsp The v5 response struct from the C lib
     */
    connect_response(const MQTTAsync_successData5* rsp);
    /**
     * Create v3 connect response.
     * @param rsp The v3 response struct from the C lib
     */
    connect_response(const MQTTAsync_successData* rsp);

public:
    /**
     * Gets the URI of the broker to which we connected.
     * @return The URI of the broker.
     */
    string get_server_uri() const { return serverURI_; }
    /**
     * Gets the MQTT version for the connection.
     * @return The MQTT version for the connection.
     */
    int get_mqtt_version() const { return mqttVersion_; }
    /**
     * Determines whether a session already existed for this client on the
     * server.
     * This tells whether the server has a persistent session stored for the
     * client, given the ClientID specified in the connect message.
     * @return Whether a session already existed for this client on the server.
     */
    bool is_session_present() const { return sessionPresent_; }
};

/**
 * Response for a subscribe request.
 *
 * This contains the information returned from the broker in the SUBACK
 * packet. It gives information about the granted Qos for each topc in the
 * subscribe request.
 *
 * @li MQTT v3: These are return "codes" with the value 0-2 for each of the
 *     topic filters sent in the subscribe message.
 * @li MQTT v5 These are reason codes, with one for each of the topics sent
 *     in the subscribe message. On success, the values are the same as for
 *     MQTT v3: the granted QoS 0-2. For errors, each could be an error code
 *     with a value >= 0x80, as described in the MQTT v5 spec: (not
 *     authorized, quota exceeded, etc).
 */
struct subscribe_response : public server_response
{
    /** The reason/result code for each topic request. */
    std::vector<ReasonCode> reasonCodes_;

    friend class token;

    /**
     * Create v5 subscribe response.
     * @param rsp The v5 response struct from the C lib
     */
    subscribe_response(MQTTAsync_successData5* rsp);
    /**
     * Create v3 subscribe response.
     * @param n The number of subscription topics
     * @param rsp The v3 response struct from the C lib
     */
    subscribe_response(size_t n, MQTTAsync_successData* rsp);

public:
    /**
     * Gets the reason codes from the server response.
     *
     * On a subscribe ack there is a return/reason code for each topic that
     * was sent in the subscribe packet. Each tells the granted QoS
     * for the corresponding topic.
     *
     * For MQTT v5 values over 0x80 are error codes as described in the MQTT
     * v5 spec.
     *
     * @return A collection of return/reason codes corresponding to
     *  	   subscribing each topic. On success, this is the
     *  	   granted QoS for each topic. On failure it is the
     *  	   reason for the failure.
     */
    const std::vector<ReasonCode>& get_reason_codes() const { return reasonCodes_; }
};

/**
 * Response for unsubscribe messages.
 */
class unsubscribe_response : public server_response
{
    /** The reason/result code for each topic request. */
    std::vector<ReasonCode> reasonCodes_;

    friend class token;

    /**
     * Create v5 unsubscribe response.
     * @param rsp The v5 response struct from the C lib
     */
    unsubscribe_response(MQTTAsync_successData5* rsp);
    /**
     * Create v3 subscribe response.
     * The broker doesn't return any useful information for an unsubscribe
     * in MQTT v3.
     */
    unsubscribe_response(MQTTAsync_successData*) {}

public:
    /**
     * Gets the reason codes from the server response.
     * On an unsubscribe ack there is a reason code for each topic
     * that was sent in the unsubscribe packet. Each tells the
     * result of unsubscribing to the corresponding topic.
     * @return A collection of return codes corresponding to
     *  	   unsubscribing each topic.
     */
    const std::vector<ReasonCode>& get_reason_codes() const { return reasonCodes_; }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_server_response_h
