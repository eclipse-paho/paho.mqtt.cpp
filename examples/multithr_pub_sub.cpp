// async_subscribe.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT publisher/subscriber using the C++
// asynchronous client interface, demonstrating how you can share a client
// between multiple threads.
//
// The app will count the number of "data" messages arriving at the broker
// and then emit "events" with updated counts. A data message is any on a
// "data/#" topic, and counts are emitted on the "events/count" topic. It
// emits an event count around once every ten data messages.
//
// Note that this is a fairly contrived example, and it could be done much
// more easily in a single thread. It is meant to demonstrate how you can
// share a client amongst threads if and when that's a proper thing to do.
//
// At this time, there is a single callback or consumer queue for all
// incoming messages, so you would typically only have one thead receiving
// messages, although it _could_ send messages to multiple threads for
// processing, perhaps based on the topics. It could be common, however, to
// want to have multiple threads for publishing.
//
// This example demonstrates:
//  - Creating a client and sharing it across threads using a shared_ptr<>
//  - Using one thread to receive incoming messages from the broker and
//    another thread to publish messages to it.
//  - Connecting to an MQTT server/broker.
//  - Automatic reconnect
//  - Publishing messages
//  - Subscribing to multiple topics
//  - Using the asynchronous message consumer
//  - Signaling consumer from another thread
//

/*******************************************************************************
 * Copyright (c) 2020-2025 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "mqtt/async_client.h"

using namespace std;
using namespace std::chrono;

const std::string DFLT_SERVER_ADDRESS{"mqtt://localhost:1883"};
const std::string CLIENT_ID{"multithr_pub_sub_cpp"};

/////////////////////////////////////////////////////////////////////////////

/**
 * A thread-safe counter that can be used to occasionally signal a waiter on
 * every 10th increment.
 */
class multithr_counter
{
    using guard = std::unique_lock<std::mutex>;

    size_t count_;
    bool closed_;
    mutable bool ready_;
    mutable std::condition_variable cond_;
    mutable std::mutex lock_;

public:
    // Declare a pointer type for sharing a counter between threads
    using ptr_t = std::shared_ptr<multithr_counter>;

    // Create a new thread-safe counter with an initial count of zero.
    multithr_counter() : count_(0), closed_(false), ready_(false) {}

    // Determines if the counter has been closed.
    bool closed() const
    {
        guard g(lock_);
        return closed_;
    }

    // Close the counter and signal all waiters.
    void close()
    {
        guard g(lock_);
        closed_ = ready_ = true;
        cond_.notify_all();
    }

    // Increments the count, and then signals once every 10 messages.
    void incr()
    {
        guard g(lock_);
        if (closed_)
            throw string("Counter is closed");
        if (++count_ % 10 == 0) {
            ready_ = true;
            g.unlock();
            cond_.notify_all();
        }
    }

    // This will block the caller until at least 10 new messages received.
    size_t get_count() const
    {
        guard g(lock_);
        cond_.wait(g, [this] { return ready_; });
        ready_ = false;
        return count_;
    }
};

/////////////////////////////////////////////////////////////////////////////

// The MQTT publisher function will run in its own thread.
// It runs until the receiver thread closes the counter object.
void publisher_func(mqtt::async_client_ptr cli, multithr_counter::ptr_t counter)
{
    while (true) {
        size_t n = counter->get_count();
        if (counter->closed())
            break;

        string payload = std::to_string(n);
        cli->publish("events/count", payload)->wait();
    }
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    string address = (argc > 1) ? string(argv[1]) : DFLT_SERVER_ADDRESS;

    // Create an MQTT client using a smart pointer to be shared among threads.
    auto cli = std::make_shared<mqtt::async_client>(address, CLIENT_ID);

    // Make a counter object also with a shared pointer.
    auto counter = std::make_shared<multithr_counter>();

    // Connect options for a persistent session and automatic reconnects.
    auto connOpts = mqtt::connect_options_builder()
                        .clean_session(false)
                        .automatic_reconnect(seconds(2), seconds(30))
                        .finalize();

    auto TOPICS = mqtt::string_collection::create({"data/#", "command"});
    const vector<int> QOS{0, 1};

    try {
        // Start consuming _before_ connecting, because we could get a flood
        // of stored messages as soon as the connection completes since
        // we're using a persistent (non-clean) session with the broker.
        cli->start_consuming();

        cout << "Connecting to the MQTT server at " << address << "..." << flush;
        auto rsp = cli->connect(connOpts)->get_connect_response();
        cout << "OK\n" << endl;

        cout << "Now start an application such as 'async_publish_time'\n"
             << "that publishes to a 'data/' topic...\n"
             << endl;

        // Subscribe if this is a new session with the server
        if (!rsp.is_session_present())
            cli->subscribe(TOPICS, QOS);

        // Start the publisher thread

        std::thread publisher(publisher_func, cli, counter);

        // Start another thread to shut us down after a minute

        std::thread{[cli] {
            this_thread::sleep_for(30s);
            cout << "Signaling the consumer to stop." << endl;
            cli->stop_consuming();
        }}.detach();

        // Consume messages in this thread

        // Remember that with the message consumer, we can't detect a
        // reconnect We would need to register a connect callback or use the
        // event consumer.

        while (true) {
            auto msg = cli->consume_message();

            if (!msg) {
                // Exit if the consumer was shut down
                if (cli->consumer_closed())
                    break;

                // Otherwise let auto-reconnect deal with it.
                cout << "Disconnect detected. Attempting an auto-reconnect." << endl;
                continue;
            }

            if (msg->get_topic() == "command" && msg->to_string() == "exit") {
                cout << "Exit command received" << endl;
                break;
            }

            cout << msg->get_topic() << ": " << msg->to_string() << endl;
            counter->incr();
        }

        // Close the counter and wait for the publisher thread to complete
        cout << "\nShutting down..." << flush;
        counter->close();
        publisher.join();

        // Disconnect

        cout << "OK\nDisconnecting..." << flush;
        cli->disconnect();
        cout << "OK" << endl;
    }
    catch (const mqtt::exception& exc) {
        cerr << exc.what() << endl;
        return 1;
    }

    return 0;
}
