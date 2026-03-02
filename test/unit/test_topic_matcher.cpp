// test_topic_matcher.cpp
//
// Unit tests for the topic_matcher class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2022 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *******************************************************************************/

#define UNIT_TESTS

#include "catch2_version.h"
#include "mqtt/topic_matcher.h"

using namespace mqtt;

/////////////////////////////////////////////////////////////////////////////

TEST_CASE("insert/get", "[topic_matcher]")
{
    topic_matcher<int> tm;

    tm.insert({"some/random/topic", 42});

    auto it = tm.find("some/random/topic");

    REQUIRE(it != tm.end());
    REQUIRE(it->first == "some/random/topic");
    REQUIRE(it->second == 42);
}

TEST_CASE("matcher initialize", "[topic_matcher]")
{
    topic_matcher<int> tm{
        {"some/random/topic", 42},
        {"some/#", 99},
        {"some/other/topic", 55},
        {"some/+/topic", 33}
    };

    auto it = tm.matches("some/random/topic");

    for (; it != tm.matches_end(); ++it) {
        bool ok =
            ((it->first == "some/random/topic" && it->second == 42) ||
             (it->first == "some/#" && it->second == 99) ||
             (it->first == "some/+/topic" && it->second == 33));
        REQUIRE(ok);
    }
}

// This one is mostly borrowed from the Paho Python tests.
// It has a number of good corner cases that shoud and should not match.
TEST_CASE("matcher matches", "[topic_matcher]")
{
    // Should match

    REQUIRE((topic_matcher<int>{{"foo/bar", 42}}.has_match("foo/bar")));
    REQUIRE((topic_matcher<int>{{"foo/+", 42}}.has_match("foo/bar")));
    REQUIRE((topic_matcher<int>{{"foo/+/baz", 42}}.has_match("foo/bar/baz")));
    REQUIRE((topic_matcher<int>{{"foo/+/#", 42}}.has_match("foo/bar/baz")));
    REQUIRE((topic_matcher<int>{{"foo/bar/#", 42}}.has_match("foo/bar/baz")));
    REQUIRE((topic_matcher<int>{{"foo/bar/#", 42}}.has_match("foo/bar")));
    REQUIRE((topic_matcher<int>{{"A/B/+/#", 42}}.has_match("A/B/B/C")));
    REQUIRE((topic_matcher<int>{{"#", 42}}.has_match("foo/bar/baz")));
    REQUIRE((topic_matcher<int>{{"#", 42}}.has_match("/foo/bar")));
    REQUIRE((topic_matcher<int>{{"/#", 42}}.has_match("/foo/bar")));
    REQUIRE((topic_matcher<int>{{"$SYS/bar", 42}}.has_match("$SYS/bar")));
    REQUIRE((topic_matcher<int>{{"foo/#", 42}}.has_match("foo/$bar")));
    REQUIRE((topic_matcher<int>{{"foo/+/baz", 42}}.has_match("foo/$bar/baz")));

    // Should not match

    REQUIRE(!(topic_matcher<int>{{"test/6/#", 42}}.has_match("test/3")));
    REQUIRE(!(topic_matcher<int>{{"foo/bar", 42}}.has_match("foo")));
    REQUIRE(!(topic_matcher<int>{{"foo/+", 42}}.has_match("foo/bar/baz")));
    REQUIRE(!(topic_matcher<int>{{"foo/+/baz", 42}}.has_match("foo/bar/bar")));
    REQUIRE(!(topic_matcher<int>{{"foo/+/#", 42}}.has_match("fo2/bar/baz")));
    REQUIRE(!(topic_matcher<int>{{"/#", 42}}.has_match("foo/bar")));
    REQUIRE(!(topic_matcher<int>{{"#", 42}}.has_match("$SYS/bar")));
    REQUIRE(!(topic_matcher<int>{{"$BOB/bar", 42}}.has_match("$SYS/bar")));
    REQUIRE(!(topic_matcher<int>{{"+/bar", 42}}.has_match("$SYS/bar")));
}
