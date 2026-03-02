// topic_publish.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// It's an example of how to send messages as an MQTT publisher using the
// C++ asynchronous client interface using a 'topic' object to repeatedly
// send data to the same topic.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Publishing messages
//  - Use of the 'topic' class
//

/*******************************************************************************
 * Copyright (c) 2019-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>  // For sleep

#include "mqtt/async_client.h"

using namespace std;

const string DFLT_SERVER_URI{"mqtt://localhost:1883"};

const string TOPIC{"test"};
const int QOS = 1;

const char* PAYLOADS[] = {
    "Hello World!", "Hi there!", "Is anyone listening?", "Someone is always listening.",
    nullptr
};

const auto TIMEOUT = std::chrono::seconds(10);

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    string serverURI = (argc > 1) ? string(argv[1]) : DFLT_SERVER_URI;

    cout << "Initializing for server '" << serverURI << "'..." << endl;
    mqtt::async_client cli(serverURI, "");

    cout << "  ...OK" << endl;

    try {
        cout << "\nConnecting..." << endl;
        cli.connect()->wait();
        cout << "  ...OK" << endl;

        cout << "\nPublishing messages..." << endl;

        mqtt::topic top(cli, "test", QOS);
        mqtt::token_ptr tok;

        size_t i = 0;
        while (PAYLOADS[i]) {
            tok = top.publish(PAYLOADS[i++]);
        }
        tok->wait();  // Just wait for the last one to complete.
        cout << "OK" << endl;

        // Disconnect
        cout << "\nDisconnecting..." << endl;
        cli.disconnect()->wait();
        cout << "  ...OK" << endl;
    }
    catch (const mqtt::exception& exc) {
        cerr << exc << endl;
        return 1;
    }

    return 0;
}
