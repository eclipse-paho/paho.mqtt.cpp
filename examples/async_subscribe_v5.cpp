// async_subscribe.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT subscriber using the C++ asynchronous client
// interface, employing callbacks to receive messages and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker using MQTT v5.
//  - Subscribing to a topic
//  - Receiving messages through the callback API
//  - Displaying MQTT v5 message properties.
//  - Receiving network disconnect updates and attempting manual reconnects.
//  - Using a "clean session" and manually re-subscribing to topics on
//    reconnect.
//

/*******************************************************************************
 * Copyright (c) 2013-2025 Frank Pagliughi <fpagliughi@mindspring.com>
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

const std::string DFLT_SERVER_URI("mqtt://localhost:1883");
const std::string CLIENT_ID("paho_cpp_async_subscribe");
const std::string TOPIC("#");

const int QOS = 1;
const int N_RETRY_ATTEMPTS = 5;

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token& tok) override
    {
        std::cout << name_ << " failure";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        std::cout << std::endl;
    }

    void on_success(const mqtt::token& tok) override
    {
        std::cout << name_ << " success";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        auto top = tok.get_topics();
        if (top && !top->empty())
            std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
        std::cout << std::endl;
    }

public:
    action_listener(const std::string& name) : name_(name) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener

{
    // Counter for the number of connection retries
    int nretry_;
    // The MQTT client
    mqtt::async_client& cli_;
    // Options to use if we need to reconnect
    mqtt::connect_options& connOpts_;
    // An action listener to display the result of actions.
    action_listener subListener_;

    // This deomonstrates manually reconnecting to the broker by calling
    // connect() again. This is a possibility for an application that keeps
    // a copy of it's original connect_options, or if the app wants to
    // reconnect with different options.
    // Another way this can be done manually, if using the same options, is
    // to just call the async_client::reconnect() method.
    void reconnect()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        try {
            cli_.connect(connOpts_, nullptr, *this);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error: " << exc.what() << std::endl;
            exit(1);
        }
    }

    // Re-connection failure
    void on_failure(const mqtt::token& tok) override
    {
        std::cout << "Connection attempt failed" << std::endl;
        if (++nretry_ > N_RETRY_ATTEMPTS)
            exit(1);
        reconnect();
    }

    // (Re)connection success
    // Either this or connected() can be used for callbacks.
    void on_success(const mqtt::token& tok) override {}

    // (Re)connection success
    void connected(const std::string& cause) override
    {
        std::cout << "\nConnection success" << std::endl;
        std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
                  << "\tfor client " << CLIENT_ID << " using QoS" << QOS << "\n"
                  << "\nPress Q<Enter> to quit\n"
                  << std::endl;

        cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
    }

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void connection_lost(const std::string& cause) override
    {
        std::cout << "\nConnection lost" << std::endl;
        if (!cause.empty())
            std::cout << "\tcause: " << cause << std::endl;

        std::cout << "Reconnecting..." << std::endl;
        nretry_ = 0;
        reconnect();
    }

    // Callback for when a message arrives.
    void message_arrived(mqtt::const_message_ptr msg) override
    {
        std::cout << "\nMessage arrived" << std::endl;
        std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
        std::cout << "\tpayload: '" << msg->to_string() << std::endl;

        const mqtt::properties& props = msg->get_properties();
        if (size_t n = props.size(); n != 0) {
            std::cout << "\tproperties (" << n << "):\n\t  [";
            for (size_t i = 0; i < n - 1; ++i) std::cout << props[i] << ", ";
            std::cout << props[n - 1] << "]" << std::endl;
        }
    }

    void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
    callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
        : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription")
    {
    }
};

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    // A subscriber often wants the server to remember its messages when its
    // disconnected. In that case, it needs a unique ClientID and a
    // non-clean session.

    auto serverURI = (argc > 1) ? std::string{argv[1]} : DFLT_SERVER_URI;

    mqtt::async_client cli(serverURI, CLIENT_ID);

    auto connOpts = mqtt::connect_options_builder::v5().clean_start(true).finalize();

    // Install the callback(s) before connecting.
    callback cb(cli, connOpts);
    cli.set_callback(cb);

    // Start the connection.
    // When completed, the callback will subscribe to topic.

    try {
        std::cout << "Connecting to the MQTT server '" << serverURI << "'..." << std::flush;
        cli.connect(connOpts, nullptr, cb);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "\nERROR: Unable to connect to MQTT server: '" << serverURI << "'" << exc
                  << std::endl;
        return 1;
    }

    // Just block till user tells us to quit.

    while (std::tolower(std::cin.get()) != 'q');

    // Disconnect

    try {
        std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
        cli.disconnect()->wait();
        std::cout << "OK" << std::endl;
    }
    catch (const mqtt::exception& exc) {
        std::cerr << exc << std::endl;
        return 1;
    }

    return 0;
}
