// message.cpp

/*******************************************************************************
 * Copyright (c) 2013-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include "mqtt/message.h"

#include <cstring>
#include <utility>

#include "mqtt/export.h"

namespace mqtt {

// A const string to use for references
PAHO_MQTTPP_EXPORT const string message::EMPTY_STR;

// A const binary to use for references
PAHO_MQTTPP_EXPORT const binary message::EMPTY_BIN;

/////////////////////////////////////////////////////////////////////////////

message::message(
    string_ref topic, const void* payload, size_t len, int qos, bool retained,
    const properties& props /*=properties()*/
)
    : topic_(std::move(topic))
{
    set_payload(payload, len);
    set_qos(qos);
    set_retained(retained);
    set_properties(props);
}

message::message(
    string_ref topic, binary_ref payload, int qos, bool retained,
    const properties& props /*=properties()*/
)
    : topic_(std::move(topic))
{
    set_payload(std::move(payload));
    set_qos(qos);
    set_retained(retained);
    set_properties(props);
}

message::message(string_ref topic, const MQTTAsync_message& cmsg)
    : msg_(cmsg), topic_(std::move(topic)), props_(cmsg.properties)
{
    set_payload(cmsg.payload, cmsg.payloadlen);
    msg_.properties = props_.c_struct();
}

message::message(const message& other)
    : msg_(other.msg_), topic_(other.topic_), props_(other.props_)
{
    set_payload(other.payload_);
    msg_.properties = props_.c_struct();
}

message::message(message&& other)
    : msg_(other.msg_), topic_(std::move(other.topic_)), props_(std::move(other.props_))
{
    set_payload(std::move(other.payload_));
    other.msg_.payloadlen = 0;
    other.msg_.payload = nullptr;
    msg_.properties = props_.c_struct();
}

message& message::operator=(const message& rhs)
{
    if (&rhs != this) {
        msg_ = rhs.msg_;
        topic_ = rhs.topic_;
        set_payload(rhs.payload_);
        set_properties(rhs.props_);
    }
    return *this;
}

message& message::operator=(message&& rhs)
{
    if (&rhs != this) {
        msg_ = rhs.msg_;
        topic_ = std::move(rhs.topic_);
        set_payload(std::move(rhs.payload_));
        set_properties(std::move(rhs.props_));

        rhs.msg_ = DFLT_C_STRUCT;
    }
    return *this;
}

void message::clear_payload()
{
    payload_.reset();
    msg_.payload = nullptr;
    msg_.payloadlen = 0;
}

void message::set_payload(binary_ref payload)
{
    payload_ = std::move(payload);

    if (payload_.empty()) {
        msg_.payload = nullptr;
        msg_.payloadlen = 0;
    }
    else {
        msg_.payload = const_cast<binary_ref::value_type*>(payload_.data());
        msg_.payloadlen = int(payload_.length());
    }
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
