// test_subscribe_options.cpp
//
// Unit tests for the subscribe_options class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2025 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi
 *        - initial implementation and documentation
 *******************************************************************************/

#define UNIT_TESTS

#include <cstring>

#include "catch2_version.h"
#include "mqtt/subscribe_options.h"

using namespace mqtt;

// The struct_id for the Paho C MQTTSubscribe_options struct.
static constexpr const char* STRUCT_ID = "MQSO";

/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Test default constructor
// ----------------------------------------------------------------------

TEST_CASE("subscribe_options dflt ctor", "[options]")
{
    subscribe_options opts;
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(0 == copts.noLocal);
    REQUIRE(0 == copts.retainAsPublished);
    REQUIRE(0 == copts.retainHandling);

    REQUIRE(!opts.get_no_local());
    REQUIRE(!opts.get_retain_as_published());
    REQUIRE(0 == opts.get_retain_handling());
}

// ----------------------------------------------------------------------
// Test constructor
// ----------------------------------------------------------------------

TEST_CASE("subscribe_options ctor", "[options]")
{
    subscribe_options opts{
        subscribe_options::NO_LOCAL,
        subscribe_options::RETAIN_AS_PUBLISHED,
        subscribe_options::DONT_SEND_RETAINED,
    };
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(0 != copts.noLocal);
    REQUIRE(0 != copts.retainAsPublished);
    REQUIRE(2 == copts.retainHandling);

    REQUIRE(opts.get_no_local());
    REQUIRE(opts.get_retain_as_published());
    REQUIRE(subscribe_options::DONT_SEND_RETAINED == opts.get_retain_handling());
}

// ----------------------------------------------------------------------
// Test copy constructor
// ----------------------------------------------------------------------

TEST_CASE("subscribe_options copy ctor", "[options]")
{
    subscribe_options org_opts{
        subscribe_options::NO_LOCAL,
        subscribe_options::RETAIN_AS_PUBLISHED,
        subscribe_options::DONT_SEND_RETAINED,
    };

    subscribe_options opts{org_opts};
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(0 != copts.noLocal);
    REQUIRE(0 != copts.retainAsPublished);
    REQUIRE(2 == copts.retainHandling);

    REQUIRE(opts.get_no_local());
    REQUIRE(opts.get_retain_as_published());
    REQUIRE(subscribe_options::DONT_SEND_RETAINED == opts.get_retain_handling());
}

// ----------------------------------------------------------------------
// Test copy assignment
// ----------------------------------------------------------------------

TEST_CASE("subscribe_options copy assignment", "[options]")
{
    subscribe_options org_opts{
        subscribe_options::NO_LOCAL,
        subscribe_options::RETAIN_AS_PUBLISHED,
        subscribe_options::DONT_SEND_RETAINED,
    };

    subscribe_options opts;

    opts = org_opts;
    const auto& copts = opts.c_struct();

    REQUIRE(0 == memcmp(copts.struct_id, STRUCT_ID, 4));
    REQUIRE(0 != copts.noLocal);
    REQUIRE(0 != copts.retainAsPublished);
    REQUIRE(2 == copts.retainHandling);

    REQUIRE(opts.get_no_local());
    REQUIRE(opts.get_retain_as_published());
    REQUIRE(subscribe_options::DONT_SEND_RETAINED == opts.get_retain_handling());
}
