// mock_callback.h
//
// Dummy implementation of mqtt::callback for Unit Test.

/*******************************************************************************
 * Copyright (c) 2017 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
 * Copyright (c) 2020 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Guilherme M. Ferreira - initial implementation
 *    Frank Pagliughi - Renamed 'mock', and refactored for coding
 *    conventions
 *******************************************************************************/

#ifndef __mqtt_mock_callback_h
#define __mqtt_mock_callback_h

#include "mqtt/callback.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

class mock_callback : public mqtt::callback
{
    bool connectionLost_{false};
    bool messageArrived_{false};
    bool deliveryComplete_{false};

    void connection_lost(const std::string& cause) override { connectionLost_ = true; }

    void message_arrived(mqtt::const_message_ptr msg) override { messageArrived_ = true; }

    void delivery_complete(mqtt::delivery_token_ptr tok) override
    {
        deliveryComplete_ = true;
    }

public:
    bool connection_lost() const { return connectionLost_; }
    bool message_arrived() const { return messageArrived_; }
    bool delivery_complete() const { return deliveryComplete_; }
};

/////////////////////////////////////////////////////////////////////////////
// end namespace mqtt
}  // namespace mqtt

#endif  //  __mqtt_mock_callback_h
