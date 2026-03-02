// test_create_options.cpp
//
// Unit tests for the create_options class in the Paho MQTT C++ library.
//

/*******************************************************************************
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
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#define UNIT_TESTS

#include <cstring>

#include "catch2_version.h"
#include "mock_async_client.h"
#include "mqtt/create_options.h"

using namespace mqtt;

// ----------------------------------------------------------------------
// Test the default constructor
// ----------------------------------------------------------------------

TEST_CASE("create_options default ctor", "[options]")
{
    mqtt::create_options opts;

    REQUIRE(!opts.get_send_while_disconnected());
    REQUIRE(!opts.get_delete_oldest_messages());

    REQUIRE(opts.get_restore_messages());
    REQUIRE(opts.get_persist_qos0());
}

/////////////////////////////////////////////////////////////////////////////
//							create_options_builder
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Test the default constructor
// ----------------------------------------------------------------------

TEST_CASE("create_options_builder default ctor", "[options]")
{
    const auto opts = create_options_builder().finalize();

    REQUIRE(!opts.get_send_while_disconnected());
    REQUIRE(!opts.get_delete_oldest_messages());

    REQUIRE(opts.get_restore_messages());
    REQUIRE(opts.get_persist_qos0());
}

TEST_CASE("create_options_builder sets", "[options]")
{
    const auto opts = create_options_builder()
                          .send_while_disconnected()
                          .delete_oldest_messages()
                          .finalize();

    REQUIRE(opts.get_send_while_disconnected());
    REQUIRE(opts.get_delete_oldest_messages());

    REQUIRE(opts.get_restore_messages());
    REQUIRE(opts.get_persist_qos0());
}
