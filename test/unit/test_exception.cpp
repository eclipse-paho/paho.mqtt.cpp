// test_exception_test.cpp
//
// Unit tests for the exception class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2016-2017 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
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
 *    Guilherme M. Ferreira - original CppUnit implementation and documentation
 *    Frank Pagliughi - Conversion to Catch2 and expansion
 *******************************************************************************/

#define UNIT_TESTS

#include <cstring>

#include "catch2_version.h"
#include "mqtt/exception.h"

using namespace mqtt;

static const std::string ERR_MSG{"Some MQTT error"};

// ----------------------------------------------------------------------
// Test user constructor
// ----------------------------------------------------------------------

TEST_CASE("user constructor", "[exception]")
{
    mqtt::exception ex1(MQTTASYNC_FAILURE);
    REQUIRE(MQTTASYNC_FAILURE == ex1.get_return_code());

    mqtt::exception ex2(MQTTASYNC_PERSISTENCE_ERROR);
    REQUIRE(MQTTASYNC_PERSISTENCE_ERROR == ex2.get_return_code());

    mqtt::exception ex3(MQTTASYNC_OPERATION_INCOMPLETE);
    REQUIRE(MQTTASYNC_OPERATION_INCOMPLETE == ex3.get_return_code());
}

// ----------------------------------------------------------------------
// Test get_message()
// ----------------------------------------------------------------------

TEST_CASE("get message", "[exception]")
{
    mqtt::exception ex(MQTTASYNC_FAILURE, ERR_MSG);
    REQUIRE(ERR_MSG == ex.get_message());
}

// ----------------------------------------------------------------------
// Test get_return_code()
// ----------------------------------------------------------------------

TEST_CASE("get return code", "[exception]")
{
    mqtt::exception ex1(MQTTASYNC_FAILURE);
    REQUIRE(MQTTASYNC_FAILURE == ex1.get_return_code());
}

// ----------------------------------------------------------------------
// Test to string
// ----------------------------------------------------------------------

TEST_CASE("to_str", "[exception]")
{
    mqtt::exception ex1(MQTTASYNC_FAILURE);
    std::string msg1{"MQTT error [-1]"};
    REQUIRE(msg1 == ex1.to_string().substr(0, 15));
}

// ----------------------------------------------------------------------
// Test what
// ----------------------------------------------------------------------

TEST_CASE("what", "[exception]")
{
    mqtt::exception ex1(MQTTASYNC_FAILURE);
    const char* msg1 = "MQTT error [-1]";
    REQUIRE(memcmp(msg1, ex1.what(), 15) == 0);
}
