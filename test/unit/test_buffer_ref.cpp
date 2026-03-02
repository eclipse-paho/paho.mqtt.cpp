// test_buffer_ref.cpp
//
// Unit tests for the buffer_ref class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2017-2020 Frank Pagliughi <fpagliughi@mindspring.com>
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
#include "mqtt/buffer_ref.h"

using namespace mqtt;

static const string EMPTY_STR;

static const string STR{"Some random string"};
static const binary BIN{"\x0\x1\x2\x3\x4\x5\x6\x7"};

static const char* CSTR = "Another random string";
static const size_t CSTR_LEN = strlen(CSTR);

// ----------------------------------------------------------------------
// Test the default constructor
// ----------------------------------------------------------------------

TEST_CASE("dflt_ctor", "[collections]")
{
    string_ref sr;

    REQUIRE_FALSE(sr);
    REQUIRE(sr.empty());
}

// ----------------------------------------------------------------------
// Test the string copy constructor
// ----------------------------------------------------------------------

TEST_CASE("str_copy_ctor", "[collections]")
{
    string_ref sr(STR);
    REQUIRE(STR == sr.str());
}

// ----------------------------------------------------------------------
// Test the string move constructor
// ----------------------------------------------------------------------

TEST_CASE("str_move_ctor", "[collections]")
{
    string str(STR);
    string_ref sr(std::move(str));

    REQUIRE(STR == sr.str());
    REQUIRE(EMPTY_STR == str);
}

// ----------------------------------------------------------------------
// Test the c-string constructor
// ----------------------------------------------------------------------

TEST_CASE("cstr_ctor", "[collections]")
{
    string_ref sr(CSTR);

    REQUIRE(CSTR_LEN == strlen(sr.c_str()));
    REQUIRE(0 == strcmp(CSTR, sr.c_str()));
}

// ----------------------------------------------------------------------
// Test the pointer copy constructor
// ----------------------------------------------------------------------

TEST_CASE("ptr_copy_ctor", "[collections]")
{
    string_ptr sp(new string(STR));
    string_ref sr(sp);

    REQUIRE(STR == sr.str());
}

// ----------------------------------------------------------------------
// Test the pointer move constructor
// ----------------------------------------------------------------------

TEST_CASE("ptr_move_ctor", "[collections]")
{
    string_ptr sp(new string(STR));
    string_ref sr(std::move(sp));

    REQUIRE(STR == sr.str());
    REQUIRE_FALSE(sp);
}

// ----------------------------------------------------------------------
// Test the copy constructor
// ----------------------------------------------------------------------

TEST_CASE("copy_ctor", "[collections]")
{
    string_ref orgSR(STR);
    string_ref sr(orgSR);

    REQUIRE(STR == sr.str());
    REQUIRE(orgSR.ptr().get() == sr.ptr().get());
    REQUIRE(2 == sr.ptr().use_count());
}

// ----------------------------------------------------------------------
// Test the move constructor
// ----------------------------------------------------------------------

TEST_CASE("move_ctor", "[collections]")
{
    string_ref orgSR(STR);
    string_ref sr(std::move(orgSR));

    REQUIRE(STR == sr.str());

    REQUIRE_FALSE(orgSR);
    REQUIRE(1 == sr.ptr().use_count());
}

// ----------------------------------------------------------------------
// Test the copy assignment
// ----------------------------------------------------------------------

TEST_CASE("copy_assignment", "[collections]")
{
    string_ref sr, orgSR(STR);

    sr = orgSR;

    REQUIRE(STR == sr.str());
    REQUIRE(orgSR.ptr().get() == sr.ptr().get());
    REQUIRE(2 == sr.ptr().use_count());

    // Test for true copy
    orgSR = EMPTY_STR;
    REQUIRE(STR == sr.str());
}

// ----------------------------------------------------------------------
// Test the move assignment
// ----------------------------------------------------------------------

TEST_CASE("move_assignment", "[collections]")
{
    string_ref sr, orgSR(STR);

    sr = std::move(orgSR);

    REQUIRE(STR == sr.str());

    REQUIRE_FALSE(orgSR);
    REQUIRE(1 == sr.ptr().use_count());
}

// ----------------------------------------------------------------------
// Test the string copy assignment
// ----------------------------------------------------------------------

TEST_CASE("str_copy_assignment", "[collections]")
{
    string str(STR);
    string_ref sr;

    sr = str;
    REQUIRE(STR == sr.str());

    str = EMPTY_STR;
    REQUIRE(STR == sr.str());
}

// ----------------------------------------------------------------------
// Test the string move assignment
// ----------------------------------------------------------------------

TEST_CASE("str_move_assignment", "[collections]")
{
    string str(STR);
    string_ref sr;

    sr = std::move(str);
    REQUIRE(STR == sr.str());

    REQUIRE(EMPTY_STR == str);
    REQUIRE(1 == sr.ptr().use_count());
}

// ----------------------------------------------------------------------
// Test the c-string assignment
// ----------------------------------------------------------------------

TEST_CASE("cstr_assignment", "[collections]")
{
    string_ref sr;
    sr = CSTR;

    REQUIRE(CSTR_LEN == strlen(sr.c_str()));
    REQUIRE(0 == strcmp(CSTR, sr.c_str()));
}

// ----------------------------------------------------------------------
// Test the pointer copy assignment
// ----------------------------------------------------------------------

TEST_CASE("ptr_copy_assignment", "[collections]")
{
    string_ptr sp(new string(STR));
    string_ref sr;

    sr = sp;

    REQUIRE(STR == sr.str());
}

// ----------------------------------------------------------------------
// Test the pointer move assignment
// ----------------------------------------------------------------------

TEST_CASE("ptr_move_assignment", "[collections]")
{
    string_ptr sp(new string(STR));
    string_ref sr;

    sr = std::move(sp);

    REQUIRE(STR == sr.str());
    REQUIRE_FALSE(sp);
}

// ----------------------------------------------------------------------
// Test the reset
// ----------------------------------------------------------------------

TEST_CASE("reset", "[collections]")
{
    string_ref sr(STR);

    sr.reset();
    REQUIRE_FALSE(sr);
    REQUIRE(sr.empty());
}
