// server_props_v5.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT client using the C++ asynchronous interface
// which shows how to check the server cofiguration for an MQTT v5
// connection.
//
// With an MQTT v5 connection, the server specify can some features that it
// doesn't supports, or limits in some way. It does this by adding v5
// properties to the CONNACK packet it sends back to the client in a connect
// transaction. The C++ application can retrieve these from the connect
// token via the `connect_response` object.
//
// It also shows short-lived persistent sessions. The client asks the server
// to just keep the session for 10sec. If you re-run the application in less
// than 10sec, it should report that the session exists. Any longer, and the
// session will be gone.
//
// Note that 10sec is probably *way too short* a time for real-world
// applications. This is just for demonstrating/testing the session expiry
// interval.
//
// The sample demonstrates:
//  - Connecting to an MQTT v5 server/broker.
//  - Specifying a short-lived (10sec) persistent session.
//  - Retrieving the v5 properties from the connect response (i.e. CONNACK
//    packet)
//  - Iterating through v5 properties.
//  - Displaying server properties to the user.
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
const string CLIENT_ID{"server_props_v5"};

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    auto serverURI = (argc > 1) ? string{argv[1]} : DFLT_SERVER_URI;

    mqtt::async_client cli(serverURI, CLIENT_ID);
    auto connOpts = mqtt::connect_options_builder::v5()
                        .clean_start(false)
                        .properties({{mqtt::property::SESSION_EXPIRY_INTERVAL, 10}})
                        .finalize();

    try {
        // Connect to the server

        cout << "Connecting to the MQTT server at '" << serverURI << "'..." << flush;
        auto tok = cli.connect(connOpts);

        // Getting the connect response will block waiting for the
        // connection to complete.
        auto rsp = tok->get_connect_response();
        cout << "OK" << endl;

        // Make sure we were granted a v5 connection.
        if (rsp.get_mqtt_version() < MQTTVERSION_5) {
            cout << "Did not get an MQTT v5 connection." << endl;
            exit(1);
        }

        // Does the server have a session for us?
        cout << "\nThe session is " << (rsp.is_session_present() ? "" : "not ")
             << "present on the server." << endl;

        // Show the v5 properties from the CONNACK, if any
        cout << "\nConnection Properties:" << endl;
        if (rsp.get_properties().empty()) {
            cout << "  <none>" << endl;
        }
        else {
            for (const auto& prop : rsp.get_properties()) {
                cout << "  " << prop << endl;
            }
        }

        // OK, we're done.
        cli.disconnect()->wait();
    }
    catch (const mqtt::exception& exc) {
        cerr << "\n  " << exc << endl;
        return 1;
    }

    return 0;
}
