// test_thread_queue.cpp
//
// Unit tests for the thread_queue class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2022-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi - Initial implementation
 *******************************************************************************/

#define UNIT_TESTS

#include <chrono>
#include <future>
#include <thread>
#include <vector>

#include "catch2_version.h"
#include "mqtt/thread_queue.h"
#include "mqtt/types.h"

using namespace mqtt;
using namespace std::chrono;

TEST_CASE("thread_queue put/get", "[thread_queue]")
{
    thread_queue<int> que;

    que.put(1);
    que.put(2);
    REQUIRE(que.get() == 1);

    que.put(3);
    REQUIRE(que.get() == 2);
    REQUIRE(que.get() == 3);
}

TEST_CASE("thread_queue tryget", "[thread_queue]")
{
    thread_queue<int> que;
    int n;

    // try_get's should fail on empty queue
    REQUIRE(!que.try_get(&n));
    REQUIRE(!que.try_get_for(&n, 5ms));

    auto timeout = steady_clock::now() + 15ms;
    REQUIRE(!que.try_get_until(&n, timeout));

    que.put(1);
    que.put(2);
    REQUIRE(que.try_get(&n));
    REQUIRE(n == 1);

    que.put(3);
    REQUIRE(que.try_get(&n));
    REQUIRE(n == 2);
    REQUIRE(que.try_get(&n));
    REQUIRE(n == 3);

    // Empty now. Try should fail and leave 'n' unchanged
    REQUIRE(!que.try_get(&n));
    REQUIRE(n == 3);
}

TEST_CASE("thread_queue tryput", "[thread_queue]")
{
    thread_queue<int> que{2};

    REQUIRE(que.try_put(1));
    REQUIRE(que.try_put(2));

    // Queue full. Put should fail
    REQUIRE(!que.try_put(3));
    REQUIRE(!que.try_put_for(3, 5ms));

    auto timeout = steady_clock::now() + 15ms;
    REQUIRE(!que.try_put_until(3, timeout));
}

TEST_CASE("thread_queue mt put/get", "[thread_queue]")
{
    thread_queue<string> que;
    const size_t N = 100000;
    const size_t N_THR = 2;

    auto producer = [&que, &N]() {
        string s;
        for (size_t i = 0; i < 512; ++i) {
            s.push_back('a' + i % 26);
        }

        for (size_t i = 0; i < N; ++i) {
            que.put(s);
        }
    };

    auto consumer = [&que, &N]() {
        string s;
        bool ok = true;
        for (size_t i = 0; i < N && ok; ++i) {
            ok = que.try_get_for(&s, 250ms);
        }
        return ok;
    };

    std::vector<std::thread> producers;
    std::vector<std::future<bool>> consumers;

    for (size_t i = 0; i < N_THR; ++i) {
        producers.push_back(std::thread(producer));
    }

    for (size_t i = 0; i < N_THR; ++i) {
        consumers.push_back(std::async(consumer));
    }

    for (size_t i = 0; i < N_THR; ++i) {
        producers[i].join();
    }

    for (size_t i = 0; i < N_THR; ++i) {
        REQUIRE(consumers[i].get());
    }
}

TEST_CASE("thread_queue close", "[thread_queue]")
{
    thread_queue<int> que;
    REQUIRE(!que.closed());

    que.put(1);
    que.put(2);
    que.close();

    // Queue is closed. Shouldn't accept any new items.
    REQUIRE(que.closed());
    REQUIRE(que.size() == 2);

    REQUIRE_THROWS_AS(que.put(3), queue_closed);
    REQUIRE(!que.try_put(3));
    REQUIRE(!que.try_put_for(3, 10ms));
    REQUIRE(!que.try_put_until(3, steady_clock::now() + 10ms));

    // But can get any items already in there.
    REQUIRE(que.get() == 1);
    REQUIRE(que.get() == 2);

    // When done (closed and empty), should throw on a get(),
    // or fail on a try_get
    REQUIRE(que.empty());
    REQUIRE(que.done());

    int n;
    REQUIRE_THROWS_AS(que.get(), queue_closed);
    REQUIRE(!que.try_get(&n));
    REQUIRE(!que.try_get_for(&n, 10ms));
    REQUIRE(!que.try_get_until(&n, steady_clock::now() + 10ms));
}

TEST_CASE("thread_queue close_signals", "[thread_queue]")
{
    thread_queue<int> que;
    REQUIRE(!que.closed());

    auto thr = std::thread([&que] {
        std::this_thread::sleep_for(10ms);
        que.close();
    });

    // Should initially block, but then throw when the queue
    // is closed by the other thread.
    REQUIRE_THROWS_AS(que.get(), queue_closed);

    thr.join();
}
