// mock_action_listener.h
//
// Dummy implementation of mqtt::iaction_listener for Unit Test.
//

/*******************************************************************************
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
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
 *    Frank Pagliughi - prepared for Catch2, renamed 'mock'
 *******************************************************************************/

#ifndef __mqtt_dummy_action_listener_h
#define __mqtt_dummy_action_listener_h

#include "mqtt/token.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * Test/mock action listener to determine which callback gets triggered, if
 * any.
 */
class mock_action_listener : public iaction_listener
{
    bool onSuccessCalled_{false};
    bool onFailureCalled_{false};

    void on_success(const mqtt::token&) override { onSuccessCalled_ = true; }

    void on_failure(const mqtt::token&) override { onFailureCalled_ = true; }

public:
    bool succeeded() const { return onSuccessCalled_; }
    bool failed() const { return onFailureCalled_; }
};

/////////////////////////////////////////////////////////////////////////////
// end namespace mqtt
}  // namespace mqtt

#endif  //  __mqtt_dummy_action_listener_h
