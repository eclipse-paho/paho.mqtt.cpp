// async_consume.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT consumer/subscriber using the C++
// asynchronous client interface, employing the  to receive messages
// and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT v3 server/broker.
//  - Subscribing to a topic
//  - Persistent subscriber session
//  - Receiving messages through the synchronous queuing API
//  - Auto reconnecting
//

/*******************************************************************************
 * Copyright (c) 2013-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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
#include <string>
#include <thread>

#include "mqtt/async_client.h"

using namespace std;

const string DFLT_SERVER_URI{"mqtt://localhost:1883"};
const string CLIENT_ID{"paho_cpp_async_consume"};

const string TOPIC{"hello"};
const int QOS = 1;

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    auto serverUri = (argc > 1) ? string{argv[1]} : DFLT_SERVER_URI;

    mqtt::async_client cli(serverUri, CLIENT_ID);

    auto connOpts = mqtt::connect_options_builder::v3()
                        .keep_alive_interval(chrono::seconds(30))
                        .clean_session(false)
                        .automatic_reconnect()
                        .finalize();

    // The client will handle automatic reconnects, but we add this
    // callbacks to let the user know when we're reconnected.
    cli.set_connected_handler([](const std::string&) {
        cout << "\n*** Connected ***" << endl;
    });

    try {
        // Start consumer before connecting to make sure to not miss any messages

        cli.start_consuming();

        // Connect to the server

        cout << "Connecting to the MQTT server..." << flush;
        auto tok = cli.connect(connOpts);

        // Getting the connect response will block waiting for the
        // connection to complete.
        auto rsp = tok->get_connect_response();

        // If there is no session present, then we need to subscribe, but if
        // there is a session, then the server remembers us and our
        // subscriptions.
        if (!rsp.is_session_present()) {
            cout << "  No session present on server. Subscribing..." << flush;
            cli.subscribe(TOPIC, QOS)->wait();
        }

        cout << "OK" << endl;

        // Consume messages

        cout << "\nWaiting for messages on topic: '" << TOPIC << "'" << endl;

        while (true) {
            auto msg = cli.consume_message();

            if (msg)
                cout << msg->get_topic() << ": " << msg->to_string() << endl;
            else
                cout << "*** Connection Lost ***" << endl;
        }
    }
    catch (const mqtt::exception& exc) {
        cerr << "\n  " << exc << endl;
        return 1;
    }

    return 0;
}
