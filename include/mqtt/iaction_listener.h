/////////////////////////////////////////////////////////////////////////////
/// @file iaction_listener.h
/// Declaration of MQTT iaction_listener class
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

#ifndef __mqtt_iaction_listener_h
#define __mqtt_iaction_listener_h

#include <vector>

#include "MQTTAsync.h"
#include "mqtt/types.h"

namespace mqtt {

class token;

/////////////////////////////////////////////////////////////////////////////

/**
 * Provides a mechanism for tracking the completion of an asynchronous
 * action.
 *
 * A listener is registered on a token and that token is associated with
 * an action like connect or publish. When used with tokens on the
 * async_client the listener will be called back on the MQTT client's
 * thread. The listener will be informed if the action succeeds or fails. It
 * is important that the listener returns control quickly otherwise the
 * operation of the MQTT client will be stalled.
 */
class iaction_listener
{
public:
    /** Smart/shared pointer to an object of this class. */
    using ptr_t = std::shared_ptr<iaction_listener>;
    /** Smart/shared pointer to a const object of this class. */
    using const_ptr_t = std::shared_ptr<const iaction_listener>;

    /**
     * Virtual base destructor.
     */
    virtual ~iaction_listener() {}
    /**
     * This method is invoked when an action fails.
     * @param asyncActionToken The token
     */
    virtual void on_failure(const token& asyncActionToken) = 0;
    /**
     * This method is invoked when an action has completed successfully.
     * @param asyncActionToken The token
     */
    virtual void on_success(const token& asyncActionToken) = 0;
};

/** Smart/shared pointer to an action listener */
using iaction_listener_ptr = iaction_listener::ptr_t;

/** Smart/shared pointer to a const action listener */
using const_iaction_listener_ptr = iaction_listener::const_ptr_t;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_iaction_listener_h
