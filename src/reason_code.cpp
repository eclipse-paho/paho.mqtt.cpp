// reason_code.cpp

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

#include "mqtt/properties.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

std::string to_string(ReasonCode reasonCode)
{
    return std::string{MQTTReasonCode_toString(MQTTReasonCodes(reasonCode))};
}

std::ostream& operator<<(std::ostream& os, ReasonCode reasonCode)
{
    os << MQTTReasonCode_toString(MQTTReasonCodes(reasonCode));
    return os;
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
