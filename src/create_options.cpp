/*******************************************************************************
 * Copyright (c) 2020-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include "mqtt/create_options.h"

#include <cstring>

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

create_options::create_options(int mqttVersion, int maxBufferedMessages)
{
    opts_.MQTTVersion = mqttVersion;

    if (maxBufferedMessages != 0) {
        opts_.sendWhileDisconnected = to_int(true);
        opts_.maxBufferedMessages = maxBufferedMessages;
    }
}

// --------------------------------------------------------------------------

create_options& create_options::operator=(const create_options& rhs)
{
    if (&rhs != this) {
        opts_ = rhs.opts_;
        serverURI_ = rhs.serverURI_;
        clientId_ = rhs.clientId_;
        persistence_ = rhs.persistence_;
    }
    return *this;
}

create_options& create_options::operator=(create_options&& rhs)
{
    if (&rhs != this) {
        opts_ = std::move(rhs.opts_);
        serverURI_ = std::move(rhs.serverURI_);
        clientId_ = std::move(rhs.clientId_);
        persistence_ = std::move(rhs.persistence_);
    }
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
}  // end namespace mqtt
