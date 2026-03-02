/*******************************************************************************
 * Copyright (c) 2017-2024 Frank Pagliughi <fpagliughi@mindspring.com>
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
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

#include "mqtt/will_options.h"

#include <cstring>
#include <utility>

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

will_options::will_options() { set_topic(string()); }

will_options::will_options(
    string_ref top, const void* payload, size_t payloadlen, int qos, bool retained,
    const properties& props /*=properties()*/
)
    : props_(props)
{
    opts_.qos = qos;
    opts_.retained = retained;
    set_topic(std::move(top));
    set_payload(binary_ref(static_cast<const binary_ref::value_type*>(payload), payloadlen));
}

will_options::will_options(
    const topic& top, const void* payload, size_t payloadlen, int qos, bool retained,
    const properties& props /*=properties()*/
)
    : will_options(top.get_name(), payload, payloadlen, qos, retained, props)
{
}

will_options::will_options(
    string_ref top, binary_ref payload, int qos, bool retained,
    const properties& props /*=properties()*/
)
    : props_(props)
{
    opts_.qos = qos;
    opts_.retained = retained;
    set_topic(std::move(top));
    set_payload(std::move(payload));
}

will_options::will_options(
    string_ref top, const string& payload, int qos, bool retained,
    const properties& props /*=properties()*/
)
    : props_(props)
{
    opts_.qos = qos;
    opts_.retained = retained;
    set_topic(std::move(top));
    set_payload(payload);
}

will_options::will_options(const message& msg)
    : will_options(
          msg.get_topic(), msg.get_payload(), msg.get_qos(), msg.is_retained(),
          msg.get_properties()
      )
{
}

will_options::will_options(const will_options& other)
    : opts_(other.opts_), props_(other.props_)
{
    set_topic(other.topic_);
    set_payload(other.payload_);
}

will_options::will_options(will_options&& other)
    : opts_(other.opts_), props_(std::move(other.props_))
{
    set_topic(std::move(other.topic_));
    set_payload(std::move(other.payload_));
}

will_options& will_options::operator=(const will_options& rhs)
{
    if (&rhs != this) {
        opts_ = rhs.opts_;
        set_topic(rhs.topic_);
        set_payload(rhs.payload_);
        props_ = rhs.props_;
    }
    return *this;
}

will_options& will_options::operator=(will_options&& rhs)
{
    if (&rhs != this) {
        opts_ = rhs.opts_;
        set_topic(std::move(rhs.topic_));
        set_payload(std::move(rhs.payload_));
        props_ = std::move(rhs.props_);
    }
    return *this;
}

void will_options::set_topic(string_ref top)
{
    topic_ = top ? std::move(top) : string_ref(string());
    opts_.topicName = topic_.c_str();
}

void will_options::set_payload(binary_ref msg)
{
    // The C struct payload must not be nullptr for will options
    payload_ = msg ? std::move(msg) : binary_ref(binary());

    opts_.payload.len = (int)payload_.size();
    opts_.payload.data = payload_.data();
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
