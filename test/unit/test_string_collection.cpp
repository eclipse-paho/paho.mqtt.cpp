// test_string_collection.cpp
//
// Unit tests for the string_collection and name_value_collection classes in
// the Paho MQTT C++ library.
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
#include <vector>

#include "catch2_version.h"
#include "mqtt/string_collection.h"

using namespace mqtt;

static const string STR{"Some random string"};
static const std::vector<string> VEC{"test0", "test1", "test2"};

static const std::map<string, string> NV_PAIRS{
    {"name0", "value0"}, {"name1", "value1"}, {"name2", "value2"}
};

// ----------------------------------------------------------------------
// Test the default constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection default ctor", "[collections]")
{
    string_collection sc;

    REQUIRE(sc.empty());
    REQUIRE(sc.size() == size_t(0));
}

// ----------------------------------------------------------------------
// Test the string copy constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection str copy ctor", "[collections]")
{
    string_collection sc(STR);

    REQUIRE(size_t(1) == sc.size());
    REQUIRE(STR == sc[0]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(STR.c_str(), c_arr[0]));
}

// ----------------------------------------------------------------------
// Test the string move constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection str move ctor", "[collections]")
{
    string str(STR);
    string_collection sc(std::move(str));

    REQUIRE(size_t(1) == sc.size());
    REQUIRE(STR == sc[0]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(STR.c_str(), c_arr[0]));
}

// ----------------------------------------------------------------------
// Test the vector copy constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection vec copy ctor", "[collections]")
{
    string_collection sc(VEC);

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the vector move constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection vec move ctor", "[collections]")
{
    std::vector<string> vec{VEC};

    string_collection sc(std::move(vec));

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the ini vector constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection ini str ctor", "[collections]")
{
    string_collection sc({VEC[0], VEC[1], VEC[2]});

    REQUIRE(size_t(3) == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the ini vector constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection ini cstr ctor", "[collections]")
{
    string_collection sc({"test0", "test1", "test2"});

    REQUIRE(size_t(3) == sc.size());

    REQUIRE(string("test0") == sc[0]);
    REQUIRE(string("test1") == sc[1]);
    REQUIRE(string("test2") == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(sc[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(sc[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(sc[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the copy constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection copy ctor", "[collections]")
{
    string_collection orgSC(VEC);
    string_collection sc(orgSC);
    ;

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the move constructor
// ----------------------------------------------------------------------

TEST_CASE("string_collection move ctor", "[collections]")
{
    string_collection orgSC(VEC);
    string_collection sc(std::move(orgSC));

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the copy assignment
// ----------------------------------------------------------------------

TEST_CASE("string_collection copy assignment", "[collections]")
{
    string_collection orgSC(VEC);
    string_collection sc;

    sc = orgSC;

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the move assignment
// ----------------------------------------------------------------------

TEST_CASE("string_collection move assignment", "[collections]")
{
    string_collection orgSC(VEC);
    string_collection sc;

    sc = std::move(orgSC);

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the push back of strings
// ----------------------------------------------------------------------

TEST_CASE("string_collection push str", "[collections]")
{
    string_collection sc;

    for (const auto& s : VEC) sc.push_back(s);

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the push back of C strings
// ----------------------------------------------------------------------

TEST_CASE("string_collection push cstr", "[collections]")
{
    string_collection sc;

    for (const auto& s : VEC) sc.push_back(s.c_str());

    REQUIRE(VEC.size() == sc.size());

    REQUIRE(VEC[0] == sc[0]);
    REQUIRE(VEC[1] == sc[1]);
    REQUIRE(VEC[2] == sc[2]);

    auto c_arr = sc.c_arr();

    REQUIRE(0 == strcmp(VEC[0].c_str(), c_arr[0]));
    REQUIRE(0 == strcmp(VEC[1].c_str(), c_arr[1]));
    REQUIRE(0 == strcmp(VEC[2].c_str(), c_arr[2]));
}

// ----------------------------------------------------------------------
// Test the clear method
// ----------------------------------------------------------------------

TEST_CASE("string_collection clear", "[collections]")
{
    string_collection sc(VEC);

    REQUIRE_FALSE(sc.empty());

    sc.clear();

    REQUIRE(sc.empty());
    REQUIRE(size_t(0) == sc.size());
}

// ----------------------------------------------------------------------
// Test the const_iterator class
// ----------------------------------------------------------------------

TEST_CASE("string_collection for each", "[collections]")
{
    string_collection sc(VEC);

    size_t i = 0u;
    for (const auto& string : sc) {
        REQUIRE(sc[i] == sc[i]);
        ++i;
    }
    REQUIRE(i == sc.size());
}

TEST_CASE("string_collection iterator for with advance", "[collections]")
{
    string_collection sc(VEC);

    size_t i = 0u;
    for (auto it = sc.begin(); it != sc.end(); std::advance(it, 1)) {
        REQUIRE(*it == sc[i]);
        ++i;
    }
    REQUIRE(i == sc.size());
}

TEST_CASE("string_collection iterator for with pre-increment", "[collections]")
{
    string_collection sc(VEC);

    size_t i = 0u;
    for (auto it = sc.begin(); it != sc.end(); ++it) {
        REQUIRE(*it == sc[i]);
        ++i;
    }
    REQUIRE(i == sc.size());
}

TEST_CASE("string_collection iterator for with post increment", "[collections]")
{
    string_collection sc(VEC);

    size_t i = 0u;
    for (auto it = sc.begin(); it != sc.end(); it++) {
        REQUIRE(*it == sc[i]);
        ++i;
    }
    REQUIRE(i == sc.size());
}

TEST_CASE("string_collection begin", "[collections]")
{
    string_collection sc(VEC);

    REQUIRE(*(sc.begin()) == sc[0u]);
}

TEST_CASE("string_collection std copy", "[collections]")
{
    string_collection sc(VEC);

    std::vector<std::string> output;
    std::copy(sc.begin(), sc.end(), std::back_inserter(output));

    REQUIRE(sc.size() == output.size());

    for (size_t i = 0u; i < sc.size(); ++i) {
        REQUIRE(sc[i] == output[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////
// 							name_value_collection
/////////////////////////////////////////////////////////////////////////////

TEST_CASE("name_value_collection default ctor", "[collections]")
{
    name_value_collection nvc;
    REQUIRE(nvc.empty());
    REQUIRE(0 == nvc.size());
}

TEST_CASE("name_value_collection initializer ctor", "[collections]")
{
    name_value_collection nvc{{"name0", "value0"}, {"name1", "value1"}, {"name2", "value2"}};

    REQUIRE_FALSE(nvc.empty());
    REQUIRE(3 == nvc.size());

    auto cArr = nvc.c_arr();

    REQUIRE(0 == strcmp("name0", cArr[0].name));
    REQUIRE(0 == strcmp("value0", cArr[0].value));
    REQUIRE(0 == strcmp("name1", cArr[1].name));
    REQUIRE(0 == strcmp("value1", cArr[1].value));
    REQUIRE(0 == strcmp("name2", cArr[2].name));
    REQUIRE(0 == strcmp("value2", cArr[2].value));

    REQUIRE(nullptr == cArr[3].name);
    REQUIRE(nullptr == cArr[3].value);
}

TEST_CASE("name_value_collection coll ctor", "[collections]")
{
    name_value_collection nvc{NV_PAIRS};

    std::map<string, string> nvPairs{NV_PAIRS};
    const size_t SZ = nvPairs.size();

    REQUIRE_FALSE(nvc.empty());
    REQUIRE(SZ == nvc.size());

    auto cArr = nvc.c_arr();

    for (size_t i = 0; i < SZ; ++i) {
        auto key = string(cArr[i].name);
        auto val = string(cArr[i].value);

        auto it = nvPairs.find(key);
        if (it == nvPairs.end()) {
            FAIL("Can't find name/value key");
        }

        REQUIRE(it->first == key);
        REQUIRE(it->second == val);
        nvPairs.erase(it);
    }

    REQUIRE(nvPairs.empty());
}

TEST_CASE("name_value_collection insert", "[collections]")
{
    name_value_collection nvc;

    std::map<string, string> nvPairs{NV_PAIRS};
    for (const auto& nv : nvPairs) {
        nvc.insert(nv);
    }

    const size_t SZ = nvPairs.size();

    REQUIRE_FALSE(nvc.empty());
    REQUIRE(SZ == nvc.size());

    auto cArr = nvc.c_arr();

    for (size_t i = 0; i < SZ; ++i) {
        auto key = string(cArr[i].name);
        auto val = string(cArr[i].value);

        auto it = nvPairs.find(key);
        if (it == nvPairs.end()) {
            FAIL("Can't find name/value key");
        }

        REQUIRE(it->first == key);
        REQUIRE(it->second == val);
        nvPairs.erase(it);
    }

    REQUIRE(nvPairs.empty());
}
