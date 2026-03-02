/////////////////////////////////////////////////////////////////////////////
/// @file delivery_token.h
/// Declaration of MQTT delivery_token class
/// @date May 1, 2013
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2013-2016 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_delivery_token_h
#define __mqtt_delivery_token_h

#include <memory>

#include "MQTTAsync.h"
#include "mqtt/message.h"
#include "mqtt/token.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * Provides a mechanism to track the delivery progress of a message.
 * Used to track the the delivery progress of a message when a publish is
 * executed in a non-blocking manner (run in the background) action.
 */
class delivery_token : public token
{
    /** The message being tracked. */
    const_message_ptr msg_;

    /** Client has special access. */
    friend class async_client;

    /**
     * Sets the message to which this token corresponds.
     * @param msg The message to which this token corresponds.
     */
    void set_message(const_message_ptr msg) { msg_ = msg; }

public:
    /** Smart/shared pointer to an object of this class */
    using ptr_t = std::shared_ptr<delivery_token>;
    /** Smart/shared pointer to a const object of this class */
    using const_ptr_t = std::shared_ptr<delivery_token>;
    /** Weak pointer to an object of this class */
    using weak_ptr_t = std::weak_ptr<delivery_token>;

    /**
     * Creates an empty delivery token connected to a particular client.
     * @param cli The asynchronous client object.
     */
    delivery_token(iasync_client& cli) : token(token::Type::PUBLISH, cli) {}
    /**
     * Creates a delivery token connected to a particular client.
     * @param cli The asynchronous client object.
     * @param msg The message being tracked.
     */
    delivery_token(iasync_client& cli, const_message_ptr msg)
        : token(token::Type::PUBLISH, cli, msg->get_topic()), msg_(std::move(msg)) {}
    /**
     * Creates a delivery token connected to a particular client.
     * @param cli The asynchronous client object.
     * @param msg The message data.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback optional listener that will be notified when message
     *  			   delivery has completed to the requested quality of
     *  			   service
     */
    delivery_token(
        iasync_client& cli, const_message_ptr msg, void* userContext, iaction_listener& cb
    )
        : token(token::Type::PUBLISH, cli, msg->get_topic(), userContext, cb),
          msg_(std::move(msg)) {}
    /**
     * Creates an empty delivery token connected to a particular client.
     * @param cli The asynchronous client object.
     */
    static ptr_t create(iasync_client& cli) { return std::make_shared<delivery_token>(cli); }
    /**
     * Creates a delivery token connected to a particular client.
     * @param cli The asynchronous client object.
     * @param msg The message data.
     */
    static ptr_t create(iasync_client& cli, const_message_ptr msg) {
        return std::make_shared<delivery_token>(cli, msg);
    }
    /**
     * Creates a delivery token connected to a particular client.
     * @param cli The asynchronous client object.
     * @param msg The message data.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback optional listener that will be notified when message
     *  			   delivery has completed to the requested quality of
     *  			   service
     */
    static ptr_t create(
        iasync_client& cli, const_message_ptr msg, void* userContext, iaction_listener& cb
    ) {
        return std::make_shared<delivery_token>(cli, msg, userContext, cb);
    }
    /**
     * Gets the message associated with this token.
     * @return The message associated with this token.
     */
    virtual const_message_ptr get_message() const { return msg_; }
};

/** Smart/shared pointer to a delivery_token */
using delivery_token_ptr = delivery_token::ptr_t;

/** Smart/shared pointer to a const delivery_token */
using const_delivery_token_ptr = delivery_token::const_ptr_t;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_delivery_token_h
