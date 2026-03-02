// properties.cpp

/*******************************************************************************
 * Copyright (c) 2019-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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

PAHO_MQTTPP_EXPORT const std::map<property::code, std::string_view> property::TYPE_NAME{
    {PAYLOAD_FORMAT_INDICATOR, "PayloadFormatIndicator"},
    {MESSAGE_EXPIRY_INTERVAL, "MessageExpiryInterval"},
    {CONTENT_TYPE, "ContentType"},
    {RESPONSE_TOPIC, "ResponseTopic"},
    {CORRELATION_DATA, "CorrelationData"},
    {SUBSCRIPTION_IDENTIFIER, "SubscriptionIdentifier"},
    {SESSION_EXPIRY_INTERVAL, "SessionExpiryInterval"},
    {ASSIGNED_CLIENT_IDENTIFIER, "AssignedClientIdentifer"},
    {SERVER_KEEP_ALIVE, "ServerKeepAlive"},
    {AUTHENTICATION_METHOD, "AuthenticationMethod"},
    {AUTHENTICATION_DATA, "AuthenticationData"},
    {REQUEST_PROBLEM_INFORMATION, "RequestProblemInformation"},
    {WILL_DELAY_INTERVAL, "WillDelayInterval"},
    {REQUEST_RESPONSE_INFORMATION, "RequestResponseInformation"},
    {RESPONSE_INFORMATION, "ResponseInformation"},
    {SERVER_REFERENCE, "ServerReference"},
    {REASON_STRING, "ReasonString"},
    {RECEIVE_MAXIMUM, "ReceiveMaximum"},
    {TOPIC_ALIAS_MAXIMUM, "TopicAliasMaximum"},
    {TOPIC_ALIAS, "TopicAlias"},
    {MAXIMUM_QOS, "MaximumQos"},
    {RETAIN_AVAILABLE, "RetainAvailable"},
    {USER_PROPERTY, "UserProperty"},
    {MAXIMUM_PACKET_SIZE, "MaximumPacketSize"},
    {WILDCARD_SUBSCRIPTION_AVAILABLE, "WildcardSubscriptionAvailable"},
    {SUBSCRIPTION_IDENTIFIERS_AVAILABLE, "SubscriptionIdentifiersAvailable"},
    {SHARED_SUBSCRIPTION_AVAILABLE, "SharedSubscriptionAvailable"}
};

/////////////////////////////////////////////////////////////////////////////

property::property(code c, int32_t val)
{
    prop_.identifier = ::MQTTPropertyCodes(c);

    switch (::MQTTProperty_getType(prop_.identifier)) {
        case MQTTPROPERTY_TYPE_BYTE:
            prop_.value.integer4 = 0;
            prop_.value.byte = uint8_t(val);
            break;
        case MQTTPROPERTY_TYPE_TWO_BYTE_INTEGER:
            prop_.value.integer4 = 0;
            prop_.value.integer2 = uint16_t(val);
            break;
        case MQTTPROPERTY_TYPE_FOUR_BYTE_INTEGER:
        case MQTTPROPERTY_TYPE_VARIABLE_BYTE_INTEGER:
            prop_.value.integer4 = uint32_t(val);
            break;
        default:
            // TODO: Throw an exception
            break;
    }
}

property::property(code c, string_ref val)
{
    prop_.identifier = ::MQTTPropertyCodes(c);

    size_t n = val.size();
    prop_.value.data.len = int(n);
    prop_.value.data.data = (char*)malloc(n);
    std::memcpy(prop_.value.data.data, val.data(), n);
}

property::property(code c, string_ref name, string_ref val)
{
    prop_.identifier = MQTTPropertyCodes(c);

    size_t n = name.size();
    prop_.value.data.len = int(n);
    prop_.value.data.data = (char*)malloc(n);
    std::memcpy(prop_.value.data.data, name.data(), n);

    n = val.size();
    prop_.value.value.len = int(n);
    prop_.value.value.data = (char*)malloc(n);
    std::memcpy(prop_.value.value.data, val.data(), n);
}

property::property(property&& other)
{
    std::memcpy(&prop_, &other.prop_, sizeof(MQTTProperty));
    memset(&other.prop_, 0, sizeof(MQTTProperty));
}

property::~property()
{
    switch (::MQTTProperty_getType(prop_.identifier)) {
        case MQTTPROPERTY_TYPE_UTF_8_STRING_PAIR:
            free(prop_.value.value.data);
            // Fall-through

        case MQTTPROPERTY_TYPE_BINARY_DATA:
        case MQTTPROPERTY_TYPE_UTF_8_ENCODED_STRING:
            free(prop_.value.data.data);
            break;

        default:
            // Nothing necessary
            break;
    }
}

void property::copy(const MQTTProperty& cprop)
{
    size_t n;

    std::memcpy(&prop_, &cprop, sizeof(MQTTProperty));

    switch (::MQTTProperty_getType(prop_.identifier)) {
        case MQTTPROPERTY_TYPE_UTF_8_STRING_PAIR:
            n = prop_.value.value.len;
            prop_.value.value.data = (char*)malloc(n);
            memcpy(prop_.value.value.data, cprop.value.value.data, n);
            // Fall-through

        case MQTTPROPERTY_TYPE_BINARY_DATA:
        case MQTTPROPERTY_TYPE_UTF_8_ENCODED_STRING:
            n = prop_.value.data.len;
            prop_.value.data.data = (char*)malloc(n);
            memcpy(prop_.value.data.data, cprop.value.data.data, n);
            break;

        default:
            // Nothing necessary
            break;
    }
}

property& property::operator=(const property& rhs)
{
    if (&rhs != this)
        copy(rhs.prop_);

    return *this;
}

property& property::operator=(property&& rhs)
{
    if (&rhs != this) {
        std::memcpy(&prop_, &rhs.prop_, sizeof(MQTTProperty));
        memset(&rhs.prop_, 0, sizeof(MQTTProperty));
    }
    return *this;
}

std::string_view property::type_name() const
{
    if (auto p = TYPE_NAME.find(code(prop_.identifier)); p != TYPE_NAME.end()) {
        return p->second;
    }
    return std::string_view("Unknown");
}

const std::type_info& property::value_type_id()
{
    switch (::MQTTProperty_getType(prop_.identifier)) {
        case MQTTPROPERTY_TYPE_BYTE:
            return typeid(uint8_t);
        case MQTTPROPERTY_TYPE_TWO_BYTE_INTEGER:
            return typeid(uint16_t);
        case MQTTPROPERTY_TYPE_FOUR_BYTE_INTEGER:
        case MQTTPROPERTY_TYPE_VARIABLE_BYTE_INTEGER:
            return typeid(uint32_t);
        case MQTTPROPERTY_TYPE_BINARY_DATA:
            return typeid(binary);
        case MQTTPROPERTY_TYPE_UTF_8_ENCODED_STRING:
            return typeid(string);
        case MQTTPROPERTY_TYPE_UTF_8_STRING_PAIR:
            return typeid(string_pair);
    }
    return typeid(int);
}

std::ostream& operator<<(std::ostream& os, const property& prop)
{
    os << prop.type_name() << ": ";

    switch (::MQTTProperty_getType(MQTTPropertyCodes(prop.type()))) {
        case MQTTPROPERTY_TYPE_BYTE:
            os << unsigned(get<uint8_t>(prop));
            break;

        case MQTTPROPERTY_TYPE_TWO_BYTE_INTEGER:
            os << get<uint16_t>(prop);
            break;

        case MQTTPROPERTY_TYPE_FOUR_BYTE_INTEGER:
        case MQTTPROPERTY_TYPE_VARIABLE_BYTE_INTEGER:
            os << get<uint32_t>(prop);
            break;

        case MQTTPROPERTY_TYPE_BINARY_DATA: {
            auto bin = get<binary>(prop);
            auto n = bin.size();
            os << '[';
            if (n > 0) {
                os << std::hex;
                for (size_t i = 0; i < n - 1; ++i) os << "0x" << unsigned(bin[i]) << ", ";
                os << "0x" << unsigned(bin[n - 1]);
                os << std::dec;
            }
            os << ']';
        } break;

        case MQTTPROPERTY_TYPE_UTF_8_ENCODED_STRING:
            os << get<string>(prop);
            break;

        case MQTTPROPERTY_TYPE_UTF_8_STRING_PAIR:
            auto sp = get<string_pair>(prop);
            os << '(' << std::get<0>(sp) << ',' << std::get<1>(sp) << ')';
            break;
    }

    return os;
}

/////////////////////////////////////////////////////////////////////////////

properties::properties(std::initializer_list<property> props)
{
    for (const auto& prop : props) {
        ::MQTTProperties_add(&props_, &prop.c_struct());
    }
}

properties& properties::operator=(const properties& rhs)
{
    if (&rhs != this) {
        ::MQTTProperties_free(&props_);
        props_ = ::MQTTProperties_copy(&rhs.props_);
    }
    return *this;
}

properties& properties::operator=(properties&& rhs)
{
    if (&rhs != this) {
        ::MQTTProperties_free(&props_);
        props_ = rhs.props_;
        rhs.props_ = DFLT_C_STRUCT;
    }
    return *this;
}

property properties::get(property::code propid, size_t idx /*=0*/) const
{
    MQTTProperty* prop = MQTTProperties_getPropertyAt(
        const_cast<MQTTProperties*>(&props_), MQTTPropertyCodes(propid), int(idx)
    );
    if (!prop)
        throw bad_cast();

    return property(*prop);
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
