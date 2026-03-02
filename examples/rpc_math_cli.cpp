// rpc_math_cli.cpp
//
// This is a Paho MQTT v5 C++ sample application.
//
// It's an example of how to create a client for performing remote procedure
// calls using MQTT with the 'response topic' and 'correlation data'
// properties.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Using MQTT v5 properties
//  - Publishing RPC request messages
//  - Using asynchronous tokens
//	- Subscribing to reply topic
//

/*******************************************************************************
 * Copyright (c) 2019-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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
#include <sstream>
#include <string>
#include <thread>

#include "mqtt/async_client.h"
#include "mqtt/properties.h"

using namespace std;
using namespace std::chrono;

const string SERVER_ADDRESS{"mqtt://localhost:1883"};
const auto TIMEOUT = std::chrono::seconds(10);

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    if (argc < 4) {
        cout << "USAGE: rpc_math_cli <add|mult> <num1> <num2> [... numN]" << endl;
        return 1;
    }

    constexpr int QOS = 1;
    const string REQ_TOPIC_HDR{"requests/math/"};

    // Create a client
    mqtt::async_client cli(SERVER_ADDRESS, "");

    cli.start_consuming();

    try {
        cout << "Connecting..." << flush;
        auto connOpts = mqtt::connect_options::v5();

        mqtt::token_ptr tok = cli.connect(connOpts);
        auto connRsp = tok->get_connect_response();
        cout << "OK (" << connRsp.get_server_uri() << ")" << endl;

        // Since we gave an empty client ID, the server should create a
        // unique one for us and send it back as ASSIGNED_CLIENT_IDENTIFIER
        // in the connect properties.

        string clientId =
            get<string>(connRsp.get_properties(), mqtt::property::ASSIGNED_CLIENT_IDENTIFIER);

        // So now we can create a unique RPC response topic using
        // the assigned (unique) client ID.

        string repTopic = "replies/" + clientId + "/math";
        cout << "    Reply topic: " << repTopic << endl;

        // Subscribe to the reply topic and verify the QoS

        tok = cli.subscribe(repTopic, QOS);
        tok->wait();

        if (int(tok->get_reason_code()) != QOS) {
            cerr << "Error: Server doesn't support reply QoS: [" << tok->get_reason_code()
                 << "]" << endl;
            return 2;
        }

        // Create and send the request message

        string req{argv[1]}, reqTopic{REQ_TOPIC_HDR + req};

        mqtt::properties props{
            {mqtt::property::RESPONSE_TOPIC, repTopic},
            {mqtt::property::CORRELATION_DATA, "1"}
        };

        ostringstream os;
        os << "[ ";
        for (int i = 2; i < argc - 1; ++i) os << argv[i] << ", ";
        os << argv[argc - 1] << " ]";

        string reqArgs{os.str()};

        cout << "\nSending '" << req << "' request " << os.str() << "..." << flush;
        auto pubmsg = mqtt::message_ptr_builder()
                          .topic(reqTopic)
                          .payload(reqArgs)
                          .qos(QOS)
                          .properties(props)
                          .finalize();

        cli.publish(pubmsg)->wait_for(TIMEOUT);
        cout << "OK" << endl;

        // Wait for reply.

        auto msg = cli.try_consume_message_for(seconds(5));
        if (!msg) {
            cerr << "Didn't receive a reply from the service." << endl;
            return 1;
        }

        cout << "  Result: " << msg->to_string() << endl;

        // Unsubscribe

        cli.unsubscribe(repTopic)->wait();

        // Disconnect
        cout << "\nDisconnecting..." << flush;
        cli.disconnect()->wait();
        cout << "OK" << endl;
    }
    catch (const mqtt::exception& exc) {
        cerr << exc.what() << endl;
        return 1;
    }

    return 0;
}
