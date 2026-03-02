// test_response_options.cpp
//
// Unit tests for the response_options class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
 * Copyright (c) 2020-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Guilherme M. Ferreira
 *        - initial implementation and documentation
 *    Frank Pagliughi
 *        - converted to use Catch2
 *        - Merged in delivery response options
 *******************************************************************************/

#define UNIT_TESTS

#include <cstring>

#include "catch2_version.h"
#include "mock_async_client.h"
#include "mqtt/response_options.h"

using namespace mqtt;

/////////////////////////////////////////////////////////////////////////////

static constexpr token::Type TOKEN_TYPE = token::Type::CONNECT;

// The struct_id for the Paho C MQTTSubscribe_options struct.
static constexpr const char* STRUCT_ID = "MQTR";

const properties PROPS{
    {property::PAYLOAD_FORMAT_INDICATOR, 42}, {property::MESSAGE_EXPIRY_INTERVAL, 70000}
};

const std::vector<subscribe_options> SUB_OPTS{
    3, subscribe_options{subscribe_options::NO_LOCAL}
};

static mock_async_client cli;

// ----------------------------------------------------------------------
// Test default constructor
// ----------------------------------------------------------------------

TEST_CASE("response_options dflt ctor", "[options]")
{
    response_options opts;
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(copts.context == nullptr);

    // Make sure the v3 callback functions are set during object construction
    REQUIRE(copts.onSuccess != nullptr);
    REQUIRE(copts.onFailure != nullptr);
    REQUIRE(copts.onSuccess5 == nullptr);
    REQUIRE(copts.onFailure5 == nullptr);
}

// ----------------------------------------------------------------------
// Test user constructor
// ----------------------------------------------------------------------

TEST_CASE("response_options user ctor", "[options]")
{
    token_ptr token{token::create(TOKEN_TYPE, cli)};
    response_options opts{token};
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(copts.context == token.get());

    // Make sure the v3 callback functions are set during object construction
    REQUIRE(copts.onSuccess != nullptr);
    REQUIRE(copts.onFailure != nullptr);
    REQUIRE(copts.onSuccess5 == nullptr);
    REQUIRE(copts.onFailure5 == nullptr);
}

// ----------------------------------------------------------------------
// Test user constructor for v5
// ----------------------------------------------------------------------

TEST_CASE("response_options user v5 ctor", "[options]")
{
    token_ptr token{token::create(TOKEN_TYPE, cli)};
    response_options opts{token, 5};
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(copts.context == token.get());

    // Make sure the v5 callback functions are set during object construction
    REQUIRE(copts.onSuccess == nullptr);
    REQUIRE(copts.onFailure == nullptr);
    REQUIRE(copts.onSuccess5 != nullptr);
    REQUIRE(copts.onFailure5 != nullptr);
}

// ----------------------------------------------------------------------
// Test copy constructor
// ----------------------------------------------------------------------

TEST_CASE("response_options copy ctor", "[options]")
{
    token_ptr token{token::create(TOKEN_TYPE, cli)};

    response_options optsOrg{token, 5};
    optsOrg.set_properties(PROPS);
    optsOrg.set_subscribe_many_options(SUB_OPTS);

    response_options opts{optsOrg};
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(copts.context == token.get());

    // Make sure the v5 callback functions are set during object construction
    REQUIRE(copts.onSuccess == nullptr);
    REQUIRE(copts.onFailure == nullptr);
    REQUIRE(copts.onSuccess5 != nullptr);
    REQUIRE(copts.onFailure5 != nullptr);

    REQUIRE(opts.get_properties().size() == PROPS.size());

    auto subOpts = opts.get_subscribe_many_options();
    REQUIRE(subOpts.size() == SUB_OPTS.size());
    REQUIRE(subOpts[0].get_no_local());
    REQUIRE(subOpts[1].get_no_local());
}

// ----------------------------------------------------------------------
// Test move constructor
// ----------------------------------------------------------------------

TEST_CASE("response_options move ctor", "[options]")
{
    token_ptr token{token::create(TOKEN_TYPE, cli)};

    response_options optsOrg{token, 5};
    optsOrg.set_properties(PROPS);
    optsOrg.set_subscribe_many_options(SUB_OPTS);

    response_options opts{std::move(optsOrg)};
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(copts.context == token.get());

    // Make sure the v3 callback functions are set during object construction
    REQUIRE(copts.onSuccess == nullptr);
    REQUIRE(copts.onFailure == nullptr);
    REQUIRE(copts.onSuccess5 != nullptr);
    REQUIRE(copts.onFailure5 != nullptr);

    REQUIRE(opts.get_properties().size() == PROPS.size());

    auto subOpts = opts.get_subscribe_many_options();
    REQUIRE(subOpts.size() == SUB_OPTS.size());
    REQUIRE(subOpts[0].get_no_local());
    REQUIRE(subOpts[1].get_no_local());

    auto subOptsOrg = optsOrg.get_subscribe_many_options();
    REQUIRE(subOptsOrg.size() == 0);
}

// ----------------------------------------------------------------------
// Test builder
// ----------------------------------------------------------------------

TEST_CASE("response_options builder", "[options]")
{
    token_ptr token{token::create(TOKEN_TYPE, cli)};

    auto opts = response_options_builder()
                    .mqtt_version(5)
                    .token(token)
                    .properties(PROPS)
                    .subscribe_opts(SUB_OPTS)
                    .finalize();

    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(copts.context == token.get());

    // Make sure the v5 callback functions are set during object construction
    REQUIRE(copts.onSuccess == nullptr);
    REQUIRE(copts.onFailure == nullptr);
    REQUIRE(copts.onSuccess5 != nullptr);
    REQUIRE(copts.onFailure5 != nullptr);

    REQUIRE(opts.get_properties().size() == PROPS.size());

    auto subOpts = opts.get_subscribe_many_options();
    REQUIRE(subOpts.size() == SUB_OPTS.size());
    REQUIRE(subOpts[0].get_no_local());
    REQUIRE(subOpts[1].get_no_local());
}

// ----------------------------------------------------------------------
// Test set context
// ----------------------------------------------------------------------

TEST_CASE("response_options set token", "[options]")
{
    response_options opts;
    const auto& copts = opts.c_struct();

    REQUIRE(copts.context == nullptr);
    token_ptr token{token::create(TOKEN_TYPE, cli)};
    opts.set_token(token);
    REQUIRE(copts.context == token.get());
}

/////////////////////////////////////////////////////////////////////////////
// Delivery Response Options
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Test default constructor
// ----------------------------------------------------------------------

TEST_CASE("delivery_response_options dflt ctor", "[options]")
{
    delivery_response_options opts;
    const auto& copts = opts.c_struct();

    REQUIRE(copts.context == nullptr);

    // Make sure the v3 callback functions are set during object construction
    REQUIRE(copts.onSuccess != nullptr);
    REQUIRE(copts.onFailure != nullptr);
    REQUIRE(copts.onSuccess5 == nullptr);
    REQUIRE(copts.onFailure5 == nullptr);
}

// ----------------------------------------------------------------------
// Test user constructor
// ----------------------------------------------------------------------

TEST_CASE("delivery_response_options user ctor", "[options]")
{
    //    mock_async_client cli;

    delivery_token_ptr token{new delivery_token{cli}};
    delivery_response_options opts{token};
    const auto& copts = opts.c_struct();

    REQUIRE(copts.context == token.get());

    // Make sure the callback functions are set during object construction
    REQUIRE(copts.onSuccess != nullptr);
    REQUIRE(copts.onFailure != nullptr);
}

// ----------------------------------------------------------------------
// Test set context
// ----------------------------------------------------------------------

TEST_CASE("delivery_response_options set token", "[options]")
{
    delivery_response_options opts;
    const auto& copts = opts.c_struct();

    REQUIRE(copts.context == nullptr);

    mock_async_client cli;
    delivery_token_ptr token{new delivery_token{cli}};
    opts.set_token(token);
    REQUIRE(copts.context == token.get());
}
