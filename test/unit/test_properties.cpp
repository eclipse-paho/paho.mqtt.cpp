// test_properties.cpp
//
// Catch2 unit tests for the 'property' and 'properties' classes in the
// Eclipse Paho MQTT C++ library.
//

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

#include <cstring>
#include <iostream>

#include "catch2_version.h"
#include "mqtt/properties.h"

using namespace mqtt;

inline bool stringcmp(char* cstr, const string& s)
{
    return std::memcmp(cstr, s.data(), s.length()) == 0;
}

static const uint8_t FMT_IND = 42;
static const uint16_t TOP_ALIAS = 511;
static const uint32_t MAX_PKT_SZ = 32 * 1024;

static const string TOPIC{"replies/bubba"};
static const string NAME1{"usr1"}, NAME2{"usr2"}, VALUE1{"this is value one"},
    VALUE2{"this is value two"};

static const binary CORR_ID{"\x00\x01\x02\x03\x04", 5};

/////////////////////////////////////////////////////////////////////////////
// property

TEST_CASE("int property constructor", "[property]")
{
    // This is a known byte property
    SECTION("creating a byte property")
    {
        property::code typ = property::PAYLOAD_FORMAT_INDICATOR;

        property prop{typ, 42};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_PAYLOAD_FORMAT_INDICATOR);
        REQUIRE(prop.c_struct().value.byte == 42);

        REQUIRE(prop.type() == typ);
        REQUIRE(get<uint8_t>(prop) == uint8_t(42));
    }

    SECTION("creating a bad byte property")
    {
        // TODO: Test constructor for out of range input
    }

    // This is a known 2-byte integer property
    SECTION("creating an int2 property")
    {
        property::code typ = property::TOPIC_ALIAS;

        property prop{typ, 512};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_TOPIC_ALIAS);
        REQUIRE(prop.c_struct().value.integer2 == 512);

        REQUIRE(prop.type() == typ);
        REQUIRE(get<uint16_t>(prop) == 512);

        // Should be able to support full 16-bit unsigned range
        const uint16_t MAX = std::numeric_limits<uint16_t>::max();

        property propMax{typ, MAX};
        REQUIRE(get<uint16_t>(propMax) == MAX);
    }

    SECTION("creating a bad int2 property")
    {
        // TODO: Test constructor for out of range input
    }

    // This is a known 4-byte integer property
    SECTION("creating an int4 property")
    {
        property::code typ = property::MESSAGE_EXPIRY_INTERVAL;

        property prop{typ, 70000};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_MESSAGE_EXPIRY_INTERVAL);
        REQUIRE(prop.c_struct().value.integer4 == 70000);

        REQUIRE(prop.type() == typ);
        REQUIRE(get<uint32_t>(prop) == 70000);

        // Should be able to support full 32-bit unsigned range
        const uint32_t MAX = std::numeric_limits<uint32_t>::max();

        property propMax{typ, MAX};
        REQUIRE(get<uint32_t>(propMax) == MAX);
    }

    SECTION("creating a bad int2 property")
    {
        // TODO: Test constructor for out of range input
    }
}

TEST_CASE("string property constructor", "[property]")
{
    property::code typ = property::RESPONSE_TOPIC;

    SECTION("property from string")
    {
        string topic{"replies/bubba"};

        property prop{typ, topic};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_RESPONSE_TOPIC);
        REQUIRE(prop.c_struct().value.data.len == int(topic.length()));
        REQUIRE(stringcmp(prop.c_struct().value.data.data, topic));

        REQUIRE(prop.type() == typ);
        REQUIRE(get<string>(prop) == topic);
    }

    SECTION("property from c-string")
    {
        const char* topic = "replies/bubba";
        size_t n = std::strlen(topic);

        property prop{typ, topic};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_RESPONSE_TOPIC);
        REQUIRE(prop.c_struct().value.data.len == int(n));
        REQUIRE(std::memcmp(prop.c_struct().value.data.data, topic, n) == 0);

        REQUIRE(prop.type() == typ);
        REQUIRE(get<string>(prop) == string(topic, n));
    }
}

TEST_CASE("binary property constructor", "[property]")
{
    SECTION("property from binary")
    {
        property::code typ = property::CORRELATION_DATA;
        const size_t LEN = 5;
        binary corr_id{"\x00\x01\x02\x03\x04", LEN};

        property prop{property::CORRELATION_DATA, corr_id};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_CORRELATION_DATA);
        REQUIRE(prop.c_struct().value.data.len == int(corr_id.length()));
        REQUIRE(std::memcmp(prop.c_struct().value.data.data, corr_id.data(), LEN) == 0);

        REQUIRE(prop.type() == typ);
        REQUIRE(get<binary>(prop) == corr_id);
    }
}

TEST_CASE("string pair property constructor", "[property]")
{
    property::code typ = property::USER_PROPERTY;

    SECTION("property from strings")
    {
        string name{"bubba"}, value{"some val"};

        property prop{typ, name, value};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_USER_PROPERTY);

        REQUIRE(prop.c_struct().value.data.len == int(name.length()));
        REQUIRE(stringcmp(prop.c_struct().value.data.data, name));

        REQUIRE(prop.c_struct().value.value.len == int(value.length()));
        REQUIRE(stringcmp(prop.c_struct().value.value.data, value));

        REQUIRE(prop.type() == typ);

        auto usr = get<string_pair>(prop);
        REQUIRE(std::get<0>(usr) == name);
        REQUIRE(std::get<1>(usr) == value);
    }

    SECTION("property from c-strings")
    {
        const char* name = "bubba";
        size_t name_len = strlen(name);

        const char* value = "some val";
        size_t value_len = strlen(value);

        property prop{typ, name, value};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_USER_PROPERTY);

        REQUIRE(prop.c_struct().value.data.len == name_len);
        REQUIRE(std::memcmp(prop.c_struct().value.data.data, name, name_len) == 0);

        REQUIRE(prop.c_struct().value.value.len == value_len);
        REQUIRE(std::memcmp(prop.c_struct().value.value.data, value, value_len) == 0);

        REQUIRE(prop.type() == typ);

        auto usr = get<string_pair>(prop);
        REQUIRE(std::get<0>(usr) == string(name, name_len));
        REQUIRE(std::get<1>(usr) == string(value, value_len));
    }
}

TEST_CASE("int property copy constructor", "[property]")
{
    SECTION("copy an int4 property")
    {
        property::code typ = property::MESSAGE_EXPIRY_INTERVAL;

        property org_prop{typ, 70000};
        property prop{org_prop};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_MESSAGE_EXPIRY_INTERVAL);
        REQUIRE(prop.c_struct().value.integer4 == 70000);

        REQUIRE(prop.type() == typ);
        REQUIRE(get<uint32_t>(prop) == 70000);
    }
}

TEST_CASE("int property move constructor", "[property]")
{
    SECTION("move an int4 property")
    {
        property::code typ = property::MESSAGE_EXPIRY_INTERVAL;

        property org_prop{typ, 70000};
        property prop{std::move(org_prop)};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_MESSAGE_EXPIRY_INTERVAL);
        REQUIRE(prop.c_struct().value.integer4 == 70000);

        REQUIRE(prop.type() == typ);
        REQUIRE(get<uint32_t>(prop) == 70000);

        // Make sure the old value was moved
        REQUIRE(org_prop.c_struct().identifier == 0);
        REQUIRE(org_prop.c_struct().value.integer4 == 0);
    }
}

TEST_CASE("string property copy constructor", "[property]")
{
    SECTION("copy a string property")
    {
        property::code typ = property::RESPONSE_TOPIC;
        string topic{"replies/bubba"};

        property org_prop{typ, topic};
        property prop{org_prop};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_RESPONSE_TOPIC);

        REQUIRE(prop.c_struct().value.data.len == int(topic.length()));
        REQUIRE(
            std::memcmp(prop.c_struct().value.data.data, topic.data(), topic.length()) == 0
        );

        REQUIRE(prop.type() == typ);
        REQUIRE(get<string>(prop) == topic);
    }

    // Make sure the copy is still valid after the original disappears
    SECTION("copy a temp string property")
    {
        property::code typ = property::RESPONSE_TOPIC;
        string topic{"replies/bubba"};

        std::unique_ptr<property> org_prop{new property{typ, topic}};
        property prop{*org_prop};
        org_prop.reset(nullptr);

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_RESPONSE_TOPIC);

        REQUIRE(prop.c_struct().value.data.len == int(topic.length()));
        REQUIRE(
            std::memcmp(prop.c_struct().value.data.data, topic.data(), topic.length()) == 0
        );

        REQUIRE(prop.type() == typ);
        REQUIRE(get<string>(prop) == topic);
    }
}

TEST_CASE("string property move constructor", "[property]")
{
    SECTION("move a string property")
    {
        property::code typ = property::RESPONSE_TOPIC;
        string topic{"replies/bubba"};

        property org_prop{typ, topic};
        property prop{std::move(org_prop)};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_RESPONSE_TOPIC);

        REQUIRE(prop.c_struct().value.data.len == int(topic.length()));
        REQUIRE(stringcmp(prop.c_struct().value.data.data, topic));

        REQUIRE(prop.type() == typ);
        REQUIRE(get<string>(prop) == topic);

        // Make sure the old value was moved
        REQUIRE(org_prop.c_struct().identifier == 0);
        REQUIRE(org_prop.c_struct().value.data.len == 0);
        REQUIRE(org_prop.c_struct().value.data.data == nullptr);
    }
}

TEST_CASE("string pair property copy constructor", "[property]")
{
    property::code typ = property::USER_PROPERTY;

    SECTION("property from strings")
    {
        string name{"bubba"}, value{"some val"};

        property org_prop{typ, name, value};
        property prop{org_prop};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_USER_PROPERTY);

        REQUIRE(prop.c_struct().value.data.len == int(name.length()));
        REQUIRE(stringcmp(prop.c_struct().value.data.data, name));

        REQUIRE(prop.c_struct().value.value.len == int(value.length()));
        REQUIRE(stringcmp(prop.c_struct().value.value.data, value));

        REQUIRE(prop.type() == typ);

        auto usr = get<string_pair>(prop);
        REQUIRE(std::get<0>(usr) == name);
        REQUIRE(std::get<1>(usr) == value);
    }

    // Make sure the property is still valid after the original disappears
    SECTION("property from temp strings property")
    {
        string name{"bubba"}, value{"some val"};

        std::unique_ptr<property> org_prop{new property{typ, name, value}};
        property prop{*org_prop};
        org_prop.reset(nullptr);

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_USER_PROPERTY);

        REQUIRE(prop.c_struct().value.data.len == int(name.length()));
        REQUIRE(stringcmp(prop.c_struct().value.data.data, name));

        REQUIRE(prop.c_struct().value.value.len == int(value.length()));
        REQUIRE(stringcmp(prop.c_struct().value.value.data, value));

        REQUIRE(prop.type() == typ);

        auto usr = get<string_pair>(prop);
        REQUIRE(std::get<0>(usr) == name);
        REQUIRE(std::get<1>(usr) == value);
    }
}

TEST_CASE("string pair property move constructor", "[property]")
{
    property::code typ = property::USER_PROPERTY;

    SECTION("property from strings")
    {
        string name{"bubba"}, value{"some val"};

        property org_prop{typ, name, value};
        property prop{std::move(org_prop)};

        REQUIRE(prop.c_struct().identifier == MQTTPROPERTY_CODE_USER_PROPERTY);

        REQUIRE(prop.c_struct().value.data.len == int(name.length()));
        REQUIRE(stringcmp(prop.c_struct().value.data.data, name));

        REQUIRE(prop.c_struct().value.value.len == int(value.length()));
        REQUIRE(stringcmp(prop.c_struct().value.value.data, value));

        REQUIRE(prop.type() == typ);

        auto usr = get<string_pair>(prop);
        REQUIRE(std::get<0>(usr) == name);
        REQUIRE(std::get<1>(usr) == value);

        // Make sure the old value was moved
        REQUIRE(org_prop.c_struct().identifier == 0);
        REQUIRE(org_prop.c_struct().value.data.len == 0);
        REQUIRE(org_prop.c_struct().value.data.data == nullptr);
        REQUIRE(org_prop.c_struct().value.value.len == 0);
        REQUIRE(org_prop.c_struct().value.value.data == nullptr);
    }
}

/////////////////////////////////////////////////////////////////////////////
// properties

TEST_CASE("properties constructors", "[properties]")
{
    SECTION("properties default constructor")
    {
        properties props;
        REQUIRE(props.empty());
        REQUIRE(props.size() == 0);
    }

    SECTION("properties init list constructor")
    {
        properties props{
            {property::PAYLOAD_FORMAT_INDICATOR, 42},
            {property::MESSAGE_EXPIRY_INTERVAL, 70000}
        };
        REQUIRE(props.size() == 2);

        REQUIRE(42 == get<uint8_t>(props, property::PAYLOAD_FORMAT_INDICATOR));
        REQUIRE(70000 == get<uint32_t>(props, property::MESSAGE_EXPIRY_INTERVAL));
    }
}

TEST_CASE("properties add", "[properties]")
{
    SECTION("properties adding items")
    {
        properties props;
        REQUIRE(props.empty());
        REQUIRE(props.size() == 0);

        props.add({property::PAYLOAD_FORMAT_INDICATOR, 42});
        REQUIRE(!props.empty());
        REQUIRE(props.size() == 1);

        props.add({property::MESSAGE_EXPIRY_INTERVAL, 70000});
        REQUIRE(!props.empty());
        REQUIRE(props.size() == 2);
    }
}

TEST_CASE("properties clear", "[properties]")
{
    SECTION("properties clear")
    {
        properties props{
            {property::PAYLOAD_FORMAT_INDICATOR, 42},
            {property::MESSAGE_EXPIRY_INTERVAL, 70000}
        };
        REQUIRE(props.size() == 2);

        props.clear();
        REQUIRE(props.empty());
        REQUIRE(props.size() == 0);
    }
}

TEST_CASE("properties count and contains", "[properties]")
{
    SECTION("single count properties")
    {
        properties props;

        REQUIRE(props.count(property::PAYLOAD_FORMAT_INDICATOR) == 0);
        REQUIRE(!props.contains(property::PAYLOAD_FORMAT_INDICATOR));

        props.add({property::PAYLOAD_FORMAT_INDICATOR, 42});
        REQUIRE(props.count(property::PAYLOAD_FORMAT_INDICATOR) == 1);
        REQUIRE(props.contains(property::PAYLOAD_FORMAT_INDICATOR));

        props.add({property::MESSAGE_EXPIRY_INTERVAL, 70000});
        REQUIRE(props.count(property::MESSAGE_EXPIRY_INTERVAL) == 1);

        // Make sure adding expirary didn't affect format ind
        REQUIRE(props.count(property::PAYLOAD_FORMAT_INDICATOR) == 1);
        REQUIRE(props.contains(property::PAYLOAD_FORMAT_INDICATOR));
    }

    /*
    SECTION("single count properties with multi add") {
        properties props;

        props.add({property::PAYLOAD_FORMAT_INDICATOR, 42});
        REQUIRE(props.count(property::PAYLOAD_FORMAT_INDICATOR) == 1);

        // Can't add again
        props.add({property::PAYLOAD_FORMAT_INDICATOR, 16});
        REQUIRE(props.count(property::PAYLOAD_FORMAT_INDICATOR) == 1);
    }
    */

    SECTION("multi count properties")
    {
        properties props;

        REQUIRE(props.count(property::USER_PROPERTY) == 0);

        props.add({property::USER_PROPERTY, "usr1", "bubba"});
        REQUIRE(props.count(property::USER_PROPERTY) == 1);

        props.add({property::USER_PROPERTY, "usr2", "wally"});
        REQUIRE(props.count(property::USER_PROPERTY) == 2);

        props.add({property::USER_PROPERTY, "usr3", "some longer property value"});
        REQUIRE(props.count(property::USER_PROPERTY) == 3);
    }
}

TEST_CASE("getting properties", "[properties]")
{
    SECTION("integer properties")
    {
        properties props{
            {property::PAYLOAD_FORMAT_INDICATOR, FMT_IND},
            {property::MAXIMUM_PACKET_SIZE, MAX_PKT_SZ},
            {property::TOPIC_ALIAS, TOP_ALIAS}
        };

        auto fmtInd = props.get(property::PAYLOAD_FORMAT_INDICATOR);
        REQUIRE(get<uint8_t>(fmtInd) == FMT_IND);

        auto topAlias = props.get(property::TOPIC_ALIAS);
        REQUIRE(get<uint16_t>(topAlias) == TOP_ALIAS);

        auto maxPktSz = props.get(property::MAXIMUM_PACKET_SIZE);
        REQUIRE(get<uint32_t>(maxPktSz) == MAX_PKT_SZ);
    }

    SECTION("integer properties with typed get")
    {
        properties props{
            {property::PAYLOAD_FORMAT_INDICATOR, FMT_IND},
            {property::MAXIMUM_PACKET_SIZE, MAX_PKT_SZ},
            {property::TOPIC_ALIAS, TOP_ALIAS}
        };

        REQUIRE(get<uint8_t>(props, property::PAYLOAD_FORMAT_INDICATOR) == FMT_IND);
        REQUIRE(get<uint16_t>(props, property::TOPIC_ALIAS) == TOP_ALIAS);
        REQUIRE(get<uint32_t>(props, property::MAXIMUM_PACKET_SIZE) == MAX_PKT_SZ);
    }

    SECTION("string properties")
    {
        properties props{
            {property::RESPONSE_TOPIC, TOPIC}, {property::CORRELATION_DATA, CORR_ID}
        };

        REQUIRE(get<string>(props, property::RESPONSE_TOPIC) == TOPIC);
        REQUIRE(get<binary>(props, property::CORRELATION_DATA) == CORR_ID);
    }

    SECTION("string pair properties")
    {
        properties props{
            {property::USER_PROPERTY, NAME1, VALUE1}, {property::USER_PROPERTY, NAME2, VALUE2}
        };

        string name1, value1, name2, value2;

        std::tie(name1, value1) = get<string_pair>(props, property::USER_PROPERTY, 0);
        std::tie(name2, value2) = get<string_pair>(props, property::USER_PROPERTY, 1);

        REQUIRE(name1 == NAME1);
        REQUIRE(value1 == VALUE1);

        REQUIRE(name2 == NAME2);
        REQUIRE(value2 == VALUE2);
    }
}

TEST_CASE("properties copy and move", "[properties]")
{
    properties orgProps{
        {property::PAYLOAD_FORMAT_INDICATOR, FMT_IND},
        {property::MAXIMUM_PACKET_SIZE, MAX_PKT_SZ},
        {property::TOPIC_ALIAS, TOP_ALIAS},
        {property::RESPONSE_TOPIC, TOPIC},
        {property::CORRELATION_DATA, CORR_ID},
        {property::USER_PROPERTY, NAME1, VALUE1},
        {property::USER_PROPERTY, NAME2, VALUE2}
    };

    string name1, value1, name2, value2;

    SECTION("copy constructor")
    {
        properties props{orgProps};

        // Make sure it's a real copy, not a reference to org
        const auto& cprops = props.c_struct();
        const auto& orgCprops = orgProps.c_struct();
        REQUIRE(orgCprops.array != cprops.array);

        orgProps.clear();

        REQUIRE(get<uint8_t>(props, property::PAYLOAD_FORMAT_INDICATOR) == FMT_IND);
        REQUIRE(get<uint16_t>(props, property::TOPIC_ALIAS) == TOP_ALIAS);
        REQUIRE(get<uint32_t>(props, property::MAXIMUM_PACKET_SIZE) == MAX_PKT_SZ);

        REQUIRE(get<string>(props, property::RESPONSE_TOPIC) == TOPIC);
        REQUIRE(get<binary>(props, property::CORRELATION_DATA) == CORR_ID);

        std::tie(name1, value1) = get<string_pair>(props, property::USER_PROPERTY, 0);
        std::tie(name2, value2) = get<string_pair>(props, property::USER_PROPERTY, 1);

        REQUIRE(name1 == NAME1);
        REQUIRE(value1 == VALUE1);

        REQUIRE(name2 == NAME2);
        REQUIRE(value2 == VALUE2);
    }

    SECTION("move constructor")
    {
        properties props{std::move(orgProps)};

        REQUIRE(get<uint8_t>(props, property::PAYLOAD_FORMAT_INDICATOR) == FMT_IND);
        REQUIRE(get<uint16_t>(props, property::TOPIC_ALIAS) == TOP_ALIAS);
        REQUIRE(get<uint32_t>(props, property::MAXIMUM_PACKET_SIZE) == MAX_PKT_SZ);

        REQUIRE(get<string>(props, property::RESPONSE_TOPIC) == TOPIC);
        REQUIRE(get<binary>(props, property::CORRELATION_DATA) == CORR_ID);

        std::tie(name1, value1) = get<string_pair>(props, property::USER_PROPERTY, 0);
        std::tie(name2, value2) = get<string_pair>(props, property::USER_PROPERTY, 1);

        REQUIRE(name1 == NAME1);
        REQUIRE(value1 == VALUE1);

        REQUIRE(name2 == NAME2);
        REQUIRE(value2 == VALUE2);

        REQUIRE(orgProps.empty());
        REQUIRE(0 == orgProps.size());

        const auto& orgCprops = orgProps.c_struct();
        REQUIRE(nullptr == orgCprops.array);
    }

    SECTION("copy assignment")
    {
        properties props;
        props = orgProps;

        // Make sure it's a real copy, not a reference to org
        const auto& cprops = props.c_struct();
        const auto& orgCprops = orgProps.c_struct();
        REQUIRE(orgCprops.array != cprops.array);

        orgProps.clear();

        REQUIRE(get<uint8_t>(props, property::PAYLOAD_FORMAT_INDICATOR) == FMT_IND);
        REQUIRE(get<uint16_t>(props, property::TOPIC_ALIAS) == TOP_ALIAS);
        REQUIRE(get<uint32_t>(props, property::MAXIMUM_PACKET_SIZE) == MAX_PKT_SZ);

        REQUIRE(get<string>(props, property::RESPONSE_TOPIC) == TOPIC);
        REQUIRE(get<binary>(props, property::CORRELATION_DATA) == CORR_ID);

        std::tie(name1, value1) = get<string_pair>(props, property::USER_PROPERTY, 0);
        std::tie(name2, value2) = get<string_pair>(props, property::USER_PROPERTY, 1);

        REQUIRE(name1 == NAME1);
        REQUIRE(value1 == VALUE1);

        REQUIRE(name2 == NAME2);
        REQUIRE(value2 == VALUE2);
    }

    SECTION("move assignment")
    {
        properties props;
        props = std::move(orgProps);

        REQUIRE(get<uint8_t>(props, property::PAYLOAD_FORMAT_INDICATOR) == FMT_IND);
        REQUIRE(get<uint16_t>(props, property::TOPIC_ALIAS) == TOP_ALIAS);
        REQUIRE(get<uint32_t>(props, property::MAXIMUM_PACKET_SIZE) == MAX_PKT_SZ);

        REQUIRE(get<string>(props, property::RESPONSE_TOPIC) == TOPIC);
        REQUIRE(get<binary>(props, property::CORRELATION_DATA) == CORR_ID);

        std::tie(name1, value1) = get<string_pair>(props, property::USER_PROPERTY, 0);
        std::tie(name2, value2) = get<string_pair>(props, property::USER_PROPERTY, 1);

        REQUIRE(name1 == NAME1);
        REQUIRE(value1 == VALUE1);

        REQUIRE(name2 == NAME2);
        REQUIRE(value2 == VALUE2);

        REQUIRE(orgProps.empty());
        REQUIRE(0 == orgProps.size());
    }
}
