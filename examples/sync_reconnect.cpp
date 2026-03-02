// sync_reconnect.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// It's a fairly contrived, but useful example of an MQTT data monitor and
// publisher, using the C++ synchronous client interface. A fairly common
// usage for MQTT applications to stay offline for much of the time and only
// connect to the broker when there is data to send.
//
// Since we don't have a universal sensor to use for this example, we simply
// use time itself as out input data. We periodically "sample" the time
// value, connect, send the value, disconnect, and then sleep. In this case
// we use the system clock, measuring the time with millisecond precision.
//
// The sample demonstrates:
//  - The synchronous client
//  - Connecting to an MQTT server/broker
//  - Periodically reconnecting to the broker
//  - Publishing messages using a `topic` object
//  - Using `connect_options` with builder classes
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
#include <thread>

#include "mqtt/client.h"

using namespace std;
using namespace std::chrono;

const std::string DFLT_SERVER_URI{"mqtt://localhost:1883"};

// The QoS for sending data
const int QOS = 1;

// How often to sample the "data"
const auto SAMPLE_PERIOD = seconds(5);

// --------------------------------------------------------------------------
// Gets the current time as the number of milliseconds since the epoch:
// like a time_t with ms resolution.

uint64_t timestamp()
{
    auto now = system_clock::now();
    auto tse = now.time_since_epoch();
    auto msTm = duration_cast<milliseconds>(tse);
    return uint64_t(msTm.count());
}

// --------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // The server URI (address)
    string serverURI = (argc > 1) ? string{argv[1]} : DFLT_SERVER_URI;

    // The amount of time to run (in sec). Zero means "run forever".
    uint64_t trun = (argc > 2) ? stoll(argv[2]) : 0LL;

    cout << "Initializing for server '" << serverURI << "'..." << endl;

    mqtt::client cli(serverURI, "");

    auto connOpts = mqtt::connect_options_builder().clean_session().finalize();

    cli.set_timeout(seconds(3));

    auto top = cli.get_topic("data/time", QOS);

    uint64_t t = timestamp(), tstart = t;

    try {
        // We need to connect once before we can use reconnect()
        cli.connect(connOpts);

        while (true) {
            cout << "\nCollecting data..." << endl;

            // Collect some data
            t = timestamp();

            if (!cli.is_connected()) {
                cout << "Reconnecting..." << endl;
                cli.reconnect();
            }

            cout << "Publishing data: " << t << "..." << endl;
            top.publish(to_string(t));

            cout << "Disconnecting..." << endl;
            cli.disconnect();

            // Quit if it's past time
            if (trun > 0 && t >= (trun + tstart))
                break;

            cout << "Going to sleep." << endl;
            this_thread::sleep_for(SAMPLE_PERIOD);
        }
    }
    catch (const mqtt::exception& exc) {
        cerr << exc << endl;
        return 1;
    }

    return 0;
}
