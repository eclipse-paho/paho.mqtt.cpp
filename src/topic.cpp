// topic.cpp

/*******************************************************************************
 * Copyright (c) 2013-2022 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include "mqtt/topic.h"

#include <algorithm>

#include "mqtt/async_client.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////
//  							topic
/////////////////////////////////////////////////////////////////////////////

// This is just a string split around '/'
std::vector<string> topic::split(const string& s)
{
    std::vector<std::string> v;

    if (s.empty())
        return v;

    const auto delim = '/';
    string::size_type startPos = 0, pos;

    do {
        pos = s.find(delim, startPos);
        auto n = (pos == string::npos) ? pos : (pos - startPos);
        v.push_back(s.substr(startPos, n));
        startPos = pos + 1;
    } while (pos != string::npos);

    return v;
}

delivery_token_ptr topic::publish(const void* payload, size_t n)
{
    return cli_.publish(name_, payload, n, qos_, retained_);
}

delivery_token_ptr topic::publish(const void* payload, size_t n, int qos, bool retained)
{
    return cli_.publish(name_, payload, n, qos, retained);
}

delivery_token_ptr topic::publish(binary_ref payload)
{
    return cli_.publish(name_, std::move(payload), qos_, retained_);
}

delivery_token_ptr topic::publish(binary_ref payload, int qos, bool retained)
{
    return cli_.publish(name_, std::move(payload), qos, retained);
}

token_ptr topic::subscribe(const subscribe_options& opts)
{
    return cli_.subscribe(name_, qos_, opts);
}

/////////////////////////////////////////////////////////////////////////////
//  						topic_filter
/////////////////////////////////////////////////////////////////////////////

topic_filter::topic_filter(const string& filter) : fields_(topic::split(filter)) {}

bool topic_filter::has_wildcards(const string& filter)
{
    auto n = filter.size();

    if (n == 0)
        return false;

    // A '#' should only be the last char, if present
    if (filter[n - 1] == '#')
        return true;

    return filter.find('+') != string::npos;
}

bool topic_filter::has_wildcards() const
{
    return std::any_of(fields_.cbegin(), fields_.cend(), [](const auto& f) {
        return is_wildcard(f);
    });
}

// See if the topic matches this filter.
// OPTIMIZE: If the filter string doesn't contain any wildcards, then a
// match is a simple string comparison. We wouldn't need to split the filter
// or topic into fields.
bool topic_filter::matches(const string& topic) const
{
    auto n = fields_.size();

    // Spec: All Topic Names and Topic Filters MUST be at least one
    //   character long [MQTT-4.7.3-1]
    // So, an empty filter can't match anything, and is technically an
    // error.
    if (n == 0) {
        return false;
    }

    auto topic_fields = topic::split(topic);
    auto nt = topic_fields.size();

    // Filter can't match a topic that is shorter
    if (n > nt && !(n == nt + 1 && fields_[n - 1] == "#")) {
        return false;
    }

    // Might match a longer topic, but only with '#' wildcard
    if (n < nt && fields_[n - 1] != "#") {
        return false;
    }

    // Topics starting with '$' don't match wildcards in the first field
    // MQTT v5 Spec, Section 4.7.2:
    // https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901246

    if (is_wildcard(fields_[0]) && nt > 0 && topic_fields[0].size() > 0 &&
        topic_fields[0][0] == '$') {
        return false;
    }

    for (size_t i = 0; i < n; ++i) {
        if (fields_[i] == "#") {
            break;
        }
        if (i == nt && i < n - 1) {
            return fields_[i + 1] == "#";
        }
        if (fields_[i] != "+" && fields_[i] != topic_fields[i]) {
            return false;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
