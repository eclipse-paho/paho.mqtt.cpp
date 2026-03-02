/////////////////////////////////////////////////////////////////////////////
/// @file subscribe_options.h
/// Declaration of MQTT subscribe_options class
/// @date Aug 1, 2019 @
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2019-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_subscribe_options_h
#define __mqtt_subscribe_options_h

#include "MQTTAsync.h"
#include "MQTTSubscribeOpts.h"
#include "mqtt/platform.h"
#include "mqtt/types.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * The MQTT v5 subscription options.
 *
 * The subscribe options are bitfields in the payload of a SUBSCRIBE packet,
 * forming a single options byte for each topic filter in the subscription.
 *
 * These were added in MQTT v5. The default (zero/false) value for each
 * field gives the behavior that was present in MQTT v3.1.1. To get a new
 * behavior the field(s) must be set.
 *
 * These are defined in section 3.8.3.1 of the MQTT v5 spec.
 */
class subscribe_options
{
    /** The underlying C structure */
    MQTTSubscribe_options opts_;

    /** The client and response have special access */
    friend class async_client;
    friend class response_options;

public:
    /** Smart/shared pointer to an object of this class. */
    using ptr_t = std::shared_ptr<subscribe_options>;
    /** Smart/shared pointer to a const object of this class. */
    using const_ptr_t = std::shared_ptr<const subscribe_options>;

    /** Don't receive our own publications */
    static constexpr bool NO_LOCAL = true;
    /** Receive our own publications */
    static constexpr bool LOCAL = false;

    /** @deprecated Don't receive our own publications */
    [[deprecated("Use NO_LOCAL")]] static constexpr bool SUBSCRIBE_NO_LOCAL = true;
    /** @deprecated Receive our own publications (obsolete name) */
    [[deprecated("Use LOCAL")]] static constexpr bool SUBSCRIBE_LOCAL = false;

    /**
     * Retain flag is only set on publications sent by a broker if in
     * response to a subscribe request
     */
    static constexpr bool NO_RETAIN_AS_PUBLISHED = false;
    /** Keep the retain flag as on the original publish message */
    static constexpr bool RETAIN_AS_PUBLISHED = true;

    /** The options for subscription retain handling */
    enum RetainHandling {
        /** Send retained messages at the time of the subscribe */
        SEND_RETAINED_ON_SUBSCRIBE = 0,
        /** Send retained messages on subscribe only if subscription is new */
        SEND_RETAINED_ON_NEW = 1,
        /** Do not send retained messages at all */
        DONT_SEND_RETAINED = 2
    };

    /**
     * Create default subscription options.
     * These are the default options corresponding to the original MQTT (v3)
     * behaviors.
     */
    subscribe_options() : opts_(MQTTSubscribe_options_initializer) {}
    /**
     * Creates a set of subscription options.
     *
     * @param noLocal Whether the server should send back our own
     *  			  publications, if subscribed.
     * @param retainAsPublished Whether to keep the retained flag as in the
     *  						original published message (true).
     * @param retainHandling When to send retained messages:
     *  	@li (SEND_RETAINED_ON_SUBSCRIBE, 0) At the time of the subscribe
     *  	@li (SEND_RETAINED_ON_NEW, 1) Only at the time of a new
     *  		subscribe
     *  	@li (DONT_SEND_RETAINED, 2) Not at the time of subscribe
     */
    explicit subscribe_options(
        bool noLocal, bool retainAsPublished = false,
        RetainHandling retainHandling = SEND_RETAINED_ON_SUBSCRIBE
    )
        : opts_(MQTTSubscribe_options_initializer) {
        opts_.noLocal = noLocal ? 1 : 0;
        opts_.retainAsPublished = retainAsPublished ? 1 : 0;
        opts_.retainHandling = (unsigned char)retainHandling;
    }
    /**
     * Creates the set of subscribe options from an underlying C struct.
     * @param opts The Paho C subscribe options
     */
    explicit subscribe_options(MQTTSubscribe_options opts) : opts_{opts} {}

#if defined(UNIT_TESTS)
    /**
     * Expose the underlying C struct for the unit tests.
     */
    const auto& c_struct() const { return opts_; }
#endif

    /**
     * Gets the value of the "no local" flag.
     * @return Whether the server should send back our own publications, if
     *  	   subscribed.
     */
    bool get_no_local() const { return to_bool(opts_.noLocal); }
    /**
     * Sets the "no local" flag on or off.
     * @param on Whether the server should send back our own publications,
     *  		 if subscribed.
     */
    void set_no_local(bool on = true) { opts_.noLocal = on ? 1 : 0; }
    /**
     * Gets the "retain as published" flag.
     * @return Whether to keep the retained flag as in the original
     *  		 published message.
     */
    bool get_retain_as_published() const { return to_bool(opts_.retainAsPublished); }
    /**
     * Sets the "retain as published" flag on or off.
     * @param on Whether to keep the retained flag as in the original
     *  		 published message.
     */
    void set_retain_as_published(bool on = true) { opts_.retainAsPublished = on ? 1 : 0; }
    /**
     * Gets the "retain handling" option.
     * @return When to send retained messages:
     *  	@li (SEND_RETAINED_ON_SUBSCRIBE, 0) At the time of the subscribe
     *  	@li (SEND_RETAINED_ON_NEW, 1) Only at the time of a new
     *  		subscribe
     *  	@li (DONT_SEND_RETAINED, 2) Not at the time of subscribe
     */
    auto get_retain_handling() const -> RetainHandling {
        return RetainHandling(opts_.retainHandling);
    }
    /**
     * Sets the "retain handling" option.
     * @param retainHandling When to send retained messages:
     *  	@li (SEND_RETAINED_ON_SUBSCRIBE, 0) At the time of the subscribe
     *  	@li (SEND_RETAINED_ON_NEW, 1) Only at the time of a new
     *  		subscribe
     *  	@li (DONT_SEND_RETAINED, 2) Not at the time of subscribe
     */
    void set_retain_handling(RetainHandling retainHandling) {
        opts_.retainHandling = (unsigned char)retainHandling;
    }
};

/** Smart/shared pointer to a subscribe options object. */
using subscribe_options_ptr = subscribe_options::ptr_t;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_subscribe_options_h
