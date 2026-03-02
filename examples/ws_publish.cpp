// ws_publish.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// It's an example of how to connect to an MQTT broker using websockets with
// an optional proxy.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker using websockets
//  - Publishing messages
//  - Using asynchronous tokens
//
// This example requires a broker that is configured to accept websocket
// connections, and optionally, an HTTP proxy.
//

/*******************************************************************************
 * Copyright (c) 2020-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "mqtt/async_client.h"

// Assume a local server with websocket support on port 8080
const std::string DFLT_SERVER_URI{"ws://localhost:8080"};

// A local proxy, like squid on port 3128
// Here assuming basic authentication with user "user" and password "pass".
const std::string DFLT_PROXY_ADDRESS{"http://user:pass@localhost:3128"};

// Quality of service for this app.
const int QOS = 1;

// Timeout for publish to complete
const auto TIMEOUT = std::chrono::seconds(10);

using namespace std;

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    string serverURI = (argc > 1) ? string{argv[1]} : DFLT_SERVER_URI,
           proxy = (argc > 2) ? string{argv[2]} : DFLT_PROXY_ADDRESS;

    cout << "Initializing for server '" << serverURI << "'..." << endl;
    if (!proxy.empty())
        cout << "    with proxy '" << proxy << "'" << endl;

    mqtt::async_client client(serverURI, "");

    // Build the connect options.

    auto connBuilder = mqtt::connect_options_builder::ws();

    if (!proxy.empty())
        connBuilder.http_proxy(proxy);

    auto connOpts = connBuilder.keep_alive_interval(std::chrono::seconds(45)).finalize();

    cout << "  ...OK" << endl;

    try {
        // Connect to the server

        cout << "\nConnecting..." << endl;
        client.connect(connOpts)->wait();
        cout << "  ...OK" << endl;

        // Send a message

        cout << "\nSending message..." << endl;
        auto msg = mqtt::make_message("hello", "Hello C++ websocket world!", QOS, false);
        bool ok = client.publish(msg)->wait_for(TIMEOUT);
        cout << "  ..." << (ok ? "OK" : "Error") << endl;

        // Disconnect

        cout << "\nDisconnecting..." << endl;
        client.disconnect()->wait();
        cout << "  ...OK" << endl;
    }
    catch (const mqtt::exception& exc) {
        cerr << exc.get_error_str() << endl;
        return 1;
    }

    return 0;
}
