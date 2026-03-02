// mqttpp_chat.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// The "chat" application is practically the "Hello World" application for
// messaging systems. This allows a user to type in message to send to a
// "group" while seeing all the messages that the other members of the group
// send.
//
// This application is an MQTT publisher/subscriber using the C++
// asynchronous client interface, employing callbacks to receive messages
// and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Publishing messages.
//  - Subscribing to a topic
//  - Receiving messages (callbacks) through a lambda function
//
// USAGE:
//     mqttpp_chat <user> <group>

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
#include <string>
#include <thread>

#include "mqtt/async_client.h"
#include "mqtt/topic.h"

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    // The broker/server address
    const std::string SERVER_ADDRESS("mqtt://localhost:1883");

    // The QoS to use for publishing and subscribing
    const int QOS = 1;

    // Tell the broker we don't want our own messages sent back to us.
    const bool NO_LOCAL = true;

    if (argc != 3) {
        std::cout << "USAGE: mqttpp_chat <user> <group>" << std::endl;
        return 1;
    }

    std::string chatUser{argv[1]}, chatGroup{argv[2]}, chatTopic{"chat/" + chatGroup};

    mqtt::async_client cli(SERVER_ADDRESS);

    // LWT message is broadcast to other users if out connection is lost

    auto lwt =
        mqtt::message(chatTopic, "<<<" + chatUser + " was disconnected>>>", QOS, false);

    // Set up the connect options

    auto connOpts = mqtt::connect_options_builder::v5()
                        .properties({{mqtt::property::SESSION_EXPIRY_INTERVAL, 604800}})
                        .clean_start(false)
                        .will(std::move(lwt))
                        .keep_alive_interval(std::chrono::seconds(20))
                        .finalize();

    // Set a callback for connection lost.
    // This just exits the app.

    cli.set_connection_lost_handler([](const std::string&) {
        std::cout << "*** Connection Lost  ***" << std::endl;
        exit(2);
    });

    // Set the callback for incoming messages

    cli.set_message_callback([](mqtt::const_message_ptr msg) {
        std::cout << msg->get_payload_str() << std::endl;
    });

    // We publish and subscribe to one topic,
    // so a 'topic' object is helpful.

    mqtt::topic topic{cli, "chat/" + chatGroup, QOS};

    // Start the connection.

    try {
        std::cout << "Connecting to the chat server at '" << SERVER_ADDRESS << "'..."
                  << std::flush;
        auto tok = cli.connect(connOpts);
        tok->wait();

        // Subscribe to the topic using "no local" so that
        // we don't get own messages sent back to us

        std::cout << "Ok\nJoining the group..." << std::flush;
        auto subOpts = mqtt::subscribe_options(NO_LOCAL);
        topic.subscribe(subOpts)->wait();
        std::cout << "Ok" << std::endl;
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "\nERROR: Unable to connect. " << exc.what() << std::endl;
        return 1;
    }

    // Let everyone know that a new user joined the conversation.

    topic.publish("<<" + chatUser + " joined the group>>");

    // Read messages from the console and publish them.
    // Quit when the use enters an empty line.

    std::string usrMsg;

    while (std::getline(std::cin, usrMsg) && !usrMsg.empty()) {
        usrMsg = chatUser + ": " + usrMsg;
        topic.publish(usrMsg);
    }

    // Let everyone know that the user left the conversation.

    topic.publish("<<" + chatUser + " left the group>>")->wait();

    // Disconnect

    try {
        std::cout << "Disconnecting from the chat server..." << std::flush;
        cli.disconnect()->wait();
        std::cout << "OK" << std::endl;
    }
    catch (const mqtt::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
