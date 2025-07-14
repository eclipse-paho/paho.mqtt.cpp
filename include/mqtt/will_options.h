/////////////////////////////////////////////////////////////////////////////
/// @file will_options.h
/// Declaration of MQTT will_options class
/// @date Jul 7, 2016
/// @author Guilherme M. Ferreira
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
 * Copyright (c) 2016-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi - added copy & move operations
 *******************************************************************************/

#ifndef __mqtt_will_options_h
#define __mqtt_will_options_h

#include "MQTTAsync.h"
#include "mqtt/message.h"
#include "mqtt/platform.h"
#include "mqtt/topic.h"
#include "mqtt/types.h"

namespace mqtt {

class connect_options;

/////////////////////////////////////////////////////////////////////////////

/**
 * Holds the set of options that govern the Last Will and Testament feature.
 *
 * @note
 * This wraps struct v1 of the C library's MQTTAsync_willOptions structure.
 * It sets the LWT binary payload, via the 'payload' struct field, and
   leaves the 'message' field as a nullptr.
 */
class will_options
{
public:
    /** The default QoS for the LWT, if unspecified */
    static constexpr int DFLT_QOS = 0;
    /** The default retained flag for LWT, if unspecified */
    static constexpr bool DFLT_RETAINED = false;

private:
    /** A default C struct to support re-initializing variables */
    static constexpr MQTTAsync_willOptions DFLT_C_STRUCT MQTTAsync_willOptions_initializer;

    /** The underlying C LWT options */
    MQTTAsync_willOptions opts_{DFLT_C_STRUCT};

    /** LWT message topic **/
    string_ref topic_;

    /** LWT message text */
    binary_ref payload_;

    /**
     * The properties for the LWT message.
     * Strangely, in the C lib, the will properties are not in the
     * willOptions struct, but are rather in the connectOptions.
     * So we keep the cached properties here, but need to transfer them to
     * the connect_options when we're added to that struct.
     */
    properties props_;

    /** The connect options has special access */
    friend class connect_options;

    /**
     * Gets a pointer to the C-language NUL-terminated strings for the
     * struct.
     * Some structs, such as this one, require valid pointers at all times,
     * while others expect NULL pointers for missing parameters.
     * So we always return a pointer to a valid C char array, even when
     * empty.
     * @param sr The C++ string object.
     * @return Pointer to a NUL terminated string. This is only valid until
     *  	   the next time the string is updated. This is never nullptr.
     */
    const char* c_str(const string_ref& sr) { return sr ? sr.to_string().c_str() : nullptr; }

public:
    /** Smart/shared pointer to an object of this class. */
    using ptr_t = std::shared_ptr<will_options>;
    /** Smart/shared pointer to a const object of this class. */
    using const_ptr_t = std::shared_ptr<const will_options>;
    /** Smart/shared pointer to an object of this class. */
    using unique_ptr_t = std::unique_ptr<will_options>;

    /**
     * Constructs a new object using the default values.
     */
    will_options();
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param top The LWT message is published to the this topic.
     * @param payload The message that is published to the Will Topic.
     * @param payload_len The message size in bytes
     * @param qos The message Quality of Service.
     * @param retained Tell the broker to keep the LWT message after send to
     *  			   subscribers.
     * @param props MQTT v5 properties for the will message.
     */
    will_options(
        string_ref top, const void* payload, size_t payload_len, int qos = DFLT_QOS,
        bool retained = DFLT_RETAINED, const properties& props = properties()
    );
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param top The LWT message is published to the this topic.
     * @param payload The message that is published to the Will Topic.
     * @param payload_len The message size in bytes.
     * @param qos The message Quality of Service.
     * @param retained Tell the broker to keep the LWT message after send to
     *  			   subscribers.
     * @param props MQTT v5 properties for the will message.
     */
    will_options(
        const topic& top, const void* payload, size_t payload_len, int qos = DFLT_QOS,
        bool retained = DFLT_RETAINED, const properties& props = properties()
    );
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param top The LWT message is published to the this topic.
     * @param payload The message payload that is published to the Will
     *  			  Topic.
     * @param qos The message Quality of Service.
     * @param retained Tell the broker to keep the LWT message after send to
     *  			   subscribers.
     * @param props MQTT v5 properties for the will message.
     */
    will_options(
        string_ref top, binary_ref payload, int qos = DFLT_QOS, bool retained = DFLT_RETAINED,
        const properties& props = properties()
    );
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param top The LWT message is published to the this topic.
     * @param payload The message payload that is published to the Will
     *  			  Topic, as a string.
     * @param qos The message Quality of Service.
     * @param retained Tell the broker to keep the LWT message after send to
     *  			   subscribers.
     * @param props MQTT v5 properties for the will message.
     */
    will_options(
        string_ref top, const string& payload, int qos = DFLT_QOS, bool retained = DFLT_RETAINED,
        const properties& props = properties()
    );
    /**
     * Sets the "Last Will and Testament" (LWT) for the connection.
     * @param msg The message that is published to the Will Topic.
     */
    will_options(const message& msg);
    /**
     * Copy constructor for the LWT options.
     * @param opt The other options.
     */
    will_options(const will_options& opt);
    /**
     * Move constructor for the LWT options.
     * @param opt The other options.
     */
    will_options(will_options&& opt);
    /**
     * Copy assignment for the LWT options.
     * @param opt The other options.
     */
    will_options& operator=(const will_options& opt);
    /**
     * Move assignment for the LWT options.
     * @param opt The other options.
     */
    will_options& operator=(will_options&& opt);
/**
 * Expose the underlying C struct for the unit tests.
 */
#if defined(UNIT_TESTS)
    const MQTTAsync_willOptions& c_struct() const { return opts_; }
#endif
    /**
     * Gets the LWT message topic name.
     * @return The LWT message topic name.
     */
    string get_topic() const { return topic_ ? topic_.to_string() : string(); }
    /**
     * Gets the LWT message payload.
     * @return The LWT message payload.
     */
    const binary_ref& get_payload() const { return payload_; }
    /**
     * Gets the LWT message payload as a string.
     * @return The LWT message payload as a string.
     */
    string get_payload_str() const { return payload_ ? payload_.to_string() : string(); }
    /**
     * Gets the QoS value for the LWT message.
     * @return The QoS value for the LWT message.
     */
    int get_qos() const { return opts_.qos; }
    /**
     * Gets the 'retained' flag for the LWT message.
     * @return The 'retained' flag for the LWT message.
     */
    bool is_retained() const { return opts_.retained != 0; }
    /**
     * Gets the LWT message as a message object.
     * @return A pointer to a copy of the LWT message.
     */
    const_message_ptr get_message() const {
        return message::create(topic_, payload_, opts_.qos, to_bool(opts_.retained));
    }
    /**
     * Sets the LWT message topic name.
     * @param top The topic where to sent the message
     */
    void set_topic(string_ref top);
    /**
     * Sets the LWT message text.
     * @param msg The LWT message
     */
    void set_payload(binary_ref msg);
    /**
     * Sets the LWT message text.
     * @param msg The LWT message
     */
    void set_payload(string msg) { set_payload(binary_ref(std::move(msg))); }
    /**
     * Sets the QoS value.
     * @param qos The LWT message QoS
     */
    void set_qos(const int qos) { opts_.qos = qos; }
    /**
     * Sets the retained flag.
     * @param retained Tell the broker to keep the LWT message after send to
     *  			   subscribers.
     */
    void set_retained(bool retained) { opts_.retained = to_int(retained); }
    /**
     * Gets the connect properties.
     * @return A const reference to the properties for the connect.
     */
    const properties& get_properties() const { return props_; }
    /**
     * Sets the properties for the connect.
     * @param props The properties to place into the message.
     */
    void set_properties(const properties& props) { props_ = props; }
    /**
     * Moves the properties for the connect.
     * @param props The properties to move into the connect object.
     */
    void set_properties(properties&& props) { props_ = std::move(props); }
};

/** Shared pointer to a will options object. */
using will_options_ptr = will_options::ptr_t;

/** Shared pointer to a const will options object. */
using const_will_options_ptr = will_options::const_ptr_t;

/** Unique pointer to a will options object. */
using will_options_unique_ptr = will_options::unique_ptr_t;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_will_options_h
