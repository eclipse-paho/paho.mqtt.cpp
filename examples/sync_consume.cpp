// sync_consume.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT consumer/subscriber using the C++ synchronous
// client interface, which uses the queuing API to receive messages.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Using a persistent (non-clean) session
//  - Subscribing to multiple topics
//  - Receiving messages through the queueing consumer API
//  - Receiving and acting upon commands via MQTT topics
//  - Auto reconnect
//  - Updating auto-reconnect data
//

/*******************************************************************************
 * Copyright (c) 2013-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include "mqtt/client.h"

using namespace std;
using namespace std::chrono;

const string DFLT_SERVER_URI{"mqtt://localhost:1883"};
const string CLIENT_ID{"paho_cpp_sync_consume"};

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    auto serverURI = (argc > 1) ? std::string{argv[1]} : DFLT_SERVER_URI;

    mqtt::client cli(serverURI, CLIENT_ID);

    auto connOpts = mqtt::connect_options_builder::v3()
                        .user_name("user")
                        .password("passwd")
                        .keep_alive_interval(seconds(30))
                        .automatic_reconnect(seconds(2), seconds(30))
                        .clean_session(false)
                        .finalize();

    // You can install a callback to change some connection data
    // on auto reconnect attempts. To make a change, update the
    // `connect_data` and return 'true'.
    cli.set_update_connection_handler([](mqtt::connect_data& connData) {
        string newUserName{"newuser"};
        if (connData.get_user_name() == newUserName)
            return false;

        cout << "Previous user: '" << connData.get_user_name() << "'" << endl;
        connData.set_user_name(newUserName);
        cout << "New user name: '" << connData.get_user_name() << "'" << endl;
        return true;
    });

    const vector<string> TOPICS{"data/#", "command"};
    const vector<int> QOS{0, 1};

    try {
        cout << "Connecting to the MQTT server..." << flush;
        mqtt::connect_response rsp = cli.connect(connOpts);
        cout << "OK\n" << endl;

        if (!rsp.is_session_present()) {
            std::cout << "Subscribing to topics..." << std::flush;
            cli.subscribe(TOPICS, QOS);
            std::cout << "OK" << std::endl;
        }
        else {
            cout << "Session already present. Skipping subscribe." << std::endl;
        }

        // Consume messages

        while (true) {
            auto msg = cli.consume_message();

            if (msg) {
                if (msg->get_topic() == "command" && msg->to_string() == "exit") {
                    cout << "Exit command received" << endl;
                    break;
                }

                cout << msg->get_topic() << ": " << msg->to_string() << endl;
            }
            else if (!cli.is_connected()) {
                cout << "Lost connection" << endl;
                while (!cli.is_connected()) {
                    this_thread::sleep_for(milliseconds(250));
                }
                cout << "Re-established connection" << endl;
            }
        }

        // Disconnect

        cout << "\nDisconnecting from the MQTT server..." << flush;
        cli.disconnect();
        cout << "OK" << endl;
    }
    catch (const mqtt::exception& exc) {
        cerr << exc.what() << endl;
        return 1;
    }

    return 0;
}
