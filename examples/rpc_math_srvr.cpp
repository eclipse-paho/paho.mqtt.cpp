// rpc_math_srvr.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT consumer/subscriber using the C++ synchronous
// client interface, which uses the queuing API to receive messages.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Subscribing to multiple topics
//  - Receiving messages through the queueing consumer API
//  - Receiving and acting upon commands via MQTT topics
//  - Manual reconnects
//  - Using a persistent (non-clean) session
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

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "mqtt/client.h"

using namespace std;
using namespace std::chrono;

const string SERVER_ADDRESS{"mqtt://localhost:1883"};
const string CLIENT_ID{"rpc_math_srvr"};

constexpr auto RESPONSE_TOPIC = mqtt::property::RESPONSE_TOPIC;
constexpr auto CORRELATION_DATA = mqtt::property::CORRELATION_DATA;

// --------------------------------------------------------------------------
// Simple function to manually reconnect a client.

bool try_reconnect(mqtt::client& cli)
{
    constexpr int N_ATTEMPT = 30;

    for (int i = 0; i < N_ATTEMPT && !cli.is_connected(); ++i) {
        try {
            cli.reconnect();
            return true;
        }
        catch (const mqtt::exception&) {
            this_thread::sleep_for(seconds(1));
        }
    }
    return false;
}

// --------------------------------------------------------------------------
// RPC function implementations

double add(const std::vector<double>& nums)
{
    double sum = 0.0;
    for (auto n : nums) sum += n;
    return sum;
}

double mult(const std::vector<double>& nums)
{
    double prod = 1.0;
    for (auto n : nums) prod *= n;
    return prod;
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    mqtt::create_options createOpts(MQTTVERSION_5);
    mqtt::client cli(SERVER_ADDRESS, CLIENT_ID, createOpts);

    auto connOpts = mqtt::connect_options_builder::v5()
                        .keep_alive_interval(seconds(20))
                        .clean_start()
                        .finalize();

    const vector<string> TOPICS{"requests/math", "requests/math/#"};
    const vector<int> QOS{1, 1};

    try {
        cout << "Connecting to the MQTT server..." << flush;
        cli.connect(connOpts);
        cli.subscribe(TOPICS, QOS);
        cout << "OK\n" << endl;

        // Consume messages

        cout << "Waiting for RPC requests..." << endl;
        while (true) {
            auto msg = cli.consume_message();

            if (!msg) {
                if (!cli.is_connected()) {
                    cout << "Lost connection. Attempting reconnect" << endl;
                    if (try_reconnect(cli)) {
                        cli.subscribe(TOPICS, QOS);
                        cout << "Reconnected" << endl;
                        continue;
                    }
                    else {
                        cout << "Reconnect failed." << endl;
                        break;
                    }
                }
                else
                    break;
            }

            cout << "Received a request" << endl;

            const mqtt::properties& props = msg->get_properties();

            if (props.contains(RESPONSE_TOPIC) && props.contains(CORRELATION_DATA)) {
                mqtt::binary corr_id = mqtt::get<string>(props, CORRELATION_DATA);
                string reply_to = mqtt::get<string>(props, RESPONSE_TOPIC);

                cout << "Client wants a reply to [" << corr_id << "] on '" << reply_to << "'"
                     << endl;

                cout << msg->get_topic() << ": " << msg->to_string() << endl;

                char c;
                double x;
                vector<double> nums;

                istringstream is(msg->to_string());
                if (!(is >> c) || c != '[') {
                    cout << "Malformed arguments" << endl;
                    // Maybe send an error message to client.
                    continue;
                }

                c = ',';
                while (c == ',' && (is >> x >> c)) nums.push_back(x);

                if (c != ']') {
                    cout << "Bad closing delimiter" << endl;
                    continue;
                }

                x = 0.0;
                if (msg->get_topic() == "requests/math/add")
                    x = add(nums);
                else if (msg->get_topic() == "requests/math/mult")
                    x = mult(nums);
                else {
                    cout << "Unknown request: " << msg->get_topic() << endl;
                    continue;
                }

                cout << "  Result: " << x << endl;

                auto reply_msg = mqtt::message::create(reply_to, to_string(x), 1, false);
                cli.publish(reply_msg);
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
