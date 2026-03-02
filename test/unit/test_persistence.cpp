// test_persistence.cpp
//
// Unit tests for the iclient_persistence class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
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
 *    Guilherme M. Ferreira - initial implementation and documentation
 *    Frank Pagliughi - Converted to use Catch2
 *******************************************************************************/

#define UNIT_TESTS

#include <cstring>

#include "catch2_version.h"
#include "mock_persistence.h"
#include "mqtt/iclient_persistence.h"

using namespace mqtt;

static const char* CLIENT_ID = "clientid";
static const char* SERVER_URI = "serveruri";
static const char* KEY = "key";
static const char* INVALID_KEY = "invalid_key";

static const char* PAYLOAD = "some random data";
static const size_t PAYLOAD_LEN = strlen(PAYLOAD);

static const char* PAYLOAD2 = "some other random data";
static const size_t PAYLOAD2_LEN = strlen(PAYLOAD2);

static const char* PAYLOAD3 = "some completely random data";
static const size_t PAYLOAD3_LEN = strlen(PAYLOAD3);

using dcp = mock_persistence;

// ----------------------------------------------------------------------
// Test static method persistence_open()
// ----------------------------------------------------------------------

TEST_CASE("persistence", "[persistence]")
{
    dcp per_;
    void* handle_;
    void* context = dynamic_cast<iclient_persistence*>(&per_);

    dcp::persistence_open(&handle_, CLIENT_ID, SERVER_URI, context);

    // Put no buffer
    int bufcount = 3;
    const char* bufs[] = {PAYLOAD, PAYLOAD2, PAYLOAD3};
    int buflens[] = {int(PAYLOAD_LEN), int(PAYLOAD2_LEN), int(PAYLOAD3_LEN)};

    dcp::persistence_put(
        handle_, const_cast<char*>(KEY), bufcount, const_cast<char**>(bufs), buflens
    );

    SECTION("test persistence open")
    {
        dcp per;
        void* handle = nullptr;
        void* context = dynamic_cast<iclient_persistence*>(&per);

        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_open(&handle, CLIENT_ID, SERVER_URI, nullptr)
        );

        REQUIRE(
            MQTTASYNC_SUCCESS ==
            dcp::persistence_open(&handle, CLIENT_ID, SERVER_URI, context)
        );
        REQUIRE(handle == context);
    }

    // ----------------------------------------------------------------------
    // Test static method persistence_close()
    // ----------------------------------------------------------------------

    SECTION("test persistence close")
    {
        dcp per;

        REQUIRE(MQTTCLIENT_PERSISTENCE_ERROR == dcp::persistence_close(nullptr));

        void* context = dynamic_cast<iclient_persistence*>(&per);
        void* handle = nullptr;
        dcp::persistence_open(&handle, CLIENT_ID, SERVER_URI, context);

        REQUIRE(MQTTASYNC_SUCCESS == dcp::persistence_close(handle));
    }

    // ----------------------------------------------------------------------
    // Test static method persistence_put()
    // ----------------------------------------------------------------------

    SECTION("test persistence put 0 buffer")
    {
        dcp per;
        void* handle;
        dcp::persistence_open(
            &handle, CLIENT_ID, SERVER_URI, dynamic_cast<iclient_persistence*>(&per)
        );

        // Put no buffer
        int bufcount = 0;
        const char* bufs[] = {PAYLOAD};
        int buflens[] = {int(PAYLOAD_LEN)};

        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_put(
                handle, const_cast<char*>(KEY), bufcount, const_cast<char**>(bufs), buflens
            )
        );
    }

    SECTION("persistence put 1 buffer")
    {
        dcp per;
        void* handle;
        dcp::persistence_open(
            &handle, CLIENT_ID, SERVER_URI, dynamic_cast<iclient_persistence*>(&per)
        );

        // Put no buffer
        int bufcount = 1;
        const char* bufs[] = {PAYLOAD};
        int buflens[] = {int(PAYLOAD_LEN)};

        REQUIRE(
            MQTTASYNC_SUCCESS ==
            dcp::persistence_put(
                handle, const_cast<char*>(KEY), bufcount, const_cast<char**>(bufs), buflens
            )
        );
        REQUIRE(
            MQTTASYNC_SUCCESS == dcp::persistence_containskey(handle, const_cast<char*>(KEY))
        );
        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_containskey(handle, const_cast<char*>(INVALID_KEY))
        );
    }

    SECTION("test persistence put 2 buffers")
    {
        dcp per;
        void* handle;
        dcp::persistence_open(
            &handle, CLIENT_ID, SERVER_URI, dynamic_cast<iclient_persistence*>(&per)
        );

        // Put no buffer
        int bufcount = 2;
        const char* bufs[] = {PAYLOAD, PAYLOAD2};
        int buflens[] = {int(PAYLOAD_LEN), int(PAYLOAD2_LEN)};

        REQUIRE(
            MQTTASYNC_SUCCESS ==
            dcp::persistence_put(
                handle, const_cast<char*>(KEY), bufcount, const_cast<char**>(bufs), buflens
            )
        );
        REQUIRE(
            MQTTASYNC_SUCCESS == dcp::persistence_containskey(handle, const_cast<char*>(KEY))
        );
        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_containskey(handle, const_cast<char*>(INVALID_KEY))
        );
    }

    SECTION("test persistence put 3 buffers")
    {
        REQUIRE(
            MQTTASYNC_SUCCESS == dcp::persistence_containskey(handle_, const_cast<char*>(KEY))
        );
        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_containskey(handle_, const_cast<char*>(INVALID_KEY))
        );
    }

    SECTION("test persistence put empty buffers")
    {
        dcp per;
        void* handle;
        dcp::persistence_open(
            &handle, CLIENT_ID, SERVER_URI, dynamic_cast<iclient_persistence*>(&per)
        );

        // Put three empty buffers
        int bufcount = 3;
        const char* buffers[] = {"", "", ""};
        int buflens[] = {0, 0, 0};
        REQUIRE(
            MQTTASYNC_SUCCESS ==
            dcp::persistence_put(
                handle, const_cast<char*>(KEY), bufcount, const_cast<char**>(buffers), buflens
            )
        );
    }

    // ----------------------------------------------------------------------
    // Test static method persistence_get()
    // ----------------------------------------------------------------------

    SECTION("test persistence get")
    {
        char* buf = nullptr;
        int buflen = 0;
        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_get(handle_, const_cast<char*>(INVALID_KEY), &buf, &buflen)
        );
        REQUIRE(0 == buflen);

        REQUIRE(
            MQTTASYNC_SUCCESS ==
            dcp::persistence_get(handle_, const_cast<char*>(KEY), &buf, &buflen)
        );

        size_t n = PAYLOAD_LEN + PAYLOAD2_LEN + PAYLOAD3_LEN;
        string str{PAYLOAD};
        str += PAYLOAD2;
        str += PAYLOAD3;

        REQUIRE(int(n) == buflen);
        REQUIRE(buf != nullptr);
        REQUIRE(memcmp(str.data(), buf, n) == 0);
    }

    // ----------------------------------------------------------------------
    // Test static method persistence_remove()
    // ----------------------------------------------------------------------

    SECTION("test persistence remove")
    {
        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_remove(handle_, const_cast<char*>(INVALID_KEY))
        );
        REQUIRE(
            MQTTASYNC_SUCCESS == dcp::persistence_containskey(handle_, const_cast<char*>(KEY))
        );
        REQUIRE(
            MQTTASYNC_SUCCESS == dcp::persistence_remove(handle_, const_cast<char*>(KEY))
        );
        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_containskey(handle_, const_cast<char*>(KEY))
        );
    }

    // ----------------------------------------------------------------------
    // Test static method persistence_keys()
    // ----------------------------------------------------------------------

    SECTION("test persistence keys")
    {
        char** keys = nullptr;
        int nkeys = 0;

        REQUIRE(MQTTASYNC_SUCCESS == dcp::persistence_keys(handle_, &keys, &nkeys));
        REQUIRE(1 == nkeys);
        REQUIRE(std::string(KEY) == std::string(keys[0]));
        // CPPUNIT_ASSERT(!strcmp(KEY, keys[0]));
    }

    // ----------------------------------------------------------------------
    // Test static method persistence_clear()
    // ----------------------------------------------------------------------

    SECTION("test persistence clear")
    {
        REQUIRE(
            MQTTASYNC_SUCCESS == dcp::persistence_containskey(handle_, const_cast<char*>(KEY))
        );
        REQUIRE(MQTTASYNC_SUCCESS == dcp::persistence_clear(handle_));
        REQUIRE(
            MQTTCLIENT_PERSISTENCE_ERROR ==
            dcp::persistence_containskey(handle_, const_cast<char*>(KEY))
        );

        char** keys = nullptr;
        int nkeys = -1;

        REQUIRE(MQTTASYNC_SUCCESS == dcp::persistence_keys(handle_, &keys, &nkeys));
        REQUIRE(0 == nkeys);
    }

    dcp::persistence_clear(handle_);
    dcp::persistence_close(handle_);
}
