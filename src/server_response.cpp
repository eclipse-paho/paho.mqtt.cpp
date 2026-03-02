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

#include "mqtt/server_response.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

connect_response::connect_response(const MQTTAsync_successData5* rsp)
    : server_response(properties(rsp->properties)),
      serverURI_(string(rsp->alt.connect.serverURI)),
      mqttVersion_(rsp->alt.connect.MQTTVersion),
      sessionPresent_(to_bool(rsp->alt.connect.sessionPresent))
{
}

connect_response::connect_response(const MQTTAsync_successData* rsp)
    : serverURI_(string(rsp->alt.connect.serverURI)),
      mqttVersion_(rsp->alt.connect.MQTTVersion),
      sessionPresent_(to_bool(rsp->alt.connect.sessionPresent))
{
}

/////////////////////////////////////////////////////////////////////////////

// Implementation note: When there is just a single topic, the C library
// places the return/reason code in the main part of the struct, and not in
// the alt `reasonCodes` or `qosList` array, _even_ if the request was sent
// with the subscribeMany call in the C library.

subscribe_response::subscribe_response(MQTTAsync_successData5* rsp)
    : server_response(properties(rsp->properties))
{
    if (rsp->alt.sub.reasonCodeCount <= 1) {
        reasonCodes_.push_back(ReasonCode(rsp->reasonCode));
    }
    else if (rsp->alt.sub.reasonCodes) {
        for (int i = 0; i < rsp->alt.sub.reasonCodeCount; ++i) {
            reasonCodes_.push_back(ReasonCode(rsp->alt.sub.reasonCodes[i]));
        }
    }
}

subscribe_response::subscribe_response(size_t n, MQTTAsync_successData* rsp)
{
    if (n <= 1) {
        reasonCodes_.push_back(ReasonCode(rsp->alt.qos));
    }
    else if (rsp->alt.qosList) {
        for (size_t i = 0; i < n; ++i) {
            reasonCodes_.push_back(ReasonCode(rsp->alt.qosList[i]));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

// Implementation note: Like with the `subscribe_response`, when there is
// just a single topic, the C library places the return/reason code in the
// main part of the struct, and not in the unsub `reasonCodes` array.

unsubscribe_response::unsubscribe_response(MQTTAsync_successData5* rsp)
    : server_response(properties(rsp->properties))
{
    if (rsp->alt.unsub.reasonCodeCount <= 1) {
        reasonCodes_.push_back(ReasonCode(rsp->reasonCode));
    }
    else if (rsp->alt.unsub.reasonCodes) {
        for (int i = 0; i < rsp->alt.unsub.reasonCodeCount; ++i) {
            reasonCodes_.push_back(ReasonCode(rsp->alt.unsub.reasonCodes[i]));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
}  // end namespace mqtt
