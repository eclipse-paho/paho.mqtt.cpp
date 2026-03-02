// sync_publish.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// It's an example of how to send messages as an MQTT publisher using the
// C++ synchronous client interface.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Publishing messages
//  - User-defined persistence
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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "mqtt/client.h"

const std::string DFLT_SERVER_URI{"mqtt://localhost:1883"};
const std::string TOPIC{"hello"};

const std::string PAYLOAD1{"Hello World!"};

const char* PAYLOAD2 = "Hi there!";
const char* PAYLOAD3 = "Is anyone listening?";

const int QOS = 1;

/////////////////////////////////////////////////////////////////////////////

// Example of a simple, in-memory persistence class.
//
// This is an extremely silly example, because if you want to use
// persistence, you actually need it to be out of process so that if the
// client crashes and restarts, the persistence data still exists.
//
// This is just here to show how the persistence API callbacks work. It maps
// well to key/value stores, like Redis, but only if it's on the local host,
// as it wouldn't make sense to persist data over the network, since that's
// what the MQTT client it trying to do.
//
class sample_mem_persistence : virtual public mqtt::iclient_persistence
{
    // Whether the store is open
    bool open_;

    // Use an STL map to store shared persistence pointers
    // against string keys.
    std::map<std::string, std::string> store_;

public:
    sample_mem_persistence() : open_(false) {}

    // "Open" the store
    void open(const std::string& clientId, const std::string& serverURI) override
    {
        std::cout << "  [Opening persistence store for '" << clientId << "' at '" << serverURI
                  << "']" << std::endl;
        open_ = true;
    }

    // Close the persistent store that was previously opened.
    void close() override
    {
        std::cout << "  [Closing persistence store.]" << std::endl;
        open_ = false;
    }

    // Clears persistence, so that it no longer contains any persisted data.
    void clear() override
    {
        std::cout << "  [Clearing persistence store.]" << std::endl;
        store_.clear();
    }

    // Returns whether or not data is persisted using the specified key.
    bool contains_key(const std::string& key) override
    {
        return store_.find(key) != store_.end();
    }

    // Returns the keys in this persistent data store.
    mqtt::string_collection keys() const override
    {
        mqtt::string_collection ks;
        for (const auto& k : store_) ks.push_back(k.first);
        return ks;
    }

    // Puts the specified data into the persistent store.
    void put(const std::string& key, const std::vector<mqtt::string_view>& bufs) override
    {
        std::cout << "  [Persisting data with key '" << key << "']" << std::endl;
        std::string str;
        for (const auto& b : bufs) str.append(b.data(), b.size());  // += b.str();
        store_[key] = std::move(str);
    }

    // Gets the specified data out of the persistent store.
    std::string get(const std::string& key) const override
    {
        std::cout << "  [Searching persistence for key '" << key << "']" << std::endl;
        auto p = store_.find(key);
        if (p == store_.end())
            throw mqtt::persistence_exception();
        std::cout << "  [Found persistence data for key '" << key << "']" << std::endl;

        return p->second;
    }

    // Remove the data for the specified key.
    void remove(const std::string& key) override
    {
        std::cout << "  [Persistence removing key '" << key << "']" << std::endl;
        auto p = store_.find(key);
        if (p == store_.end())
            throw mqtt::persistence_exception();
        store_.erase(p);
        std::cout << "  [Persistence key removed '" << key << "']" << std::endl;
    }
};

/////////////////////////////////////////////////////////////////////////////
// Class to receive callbacks

class user_callback : public virtual mqtt::callback
{
    void connection_lost(const std::string& cause) override
    {
        std::cout << "\nConnection lost" << std::endl;
        if (!cause.empty())
            std::cout << "\tcause: " << cause << std::endl;
    }

    void delivery_complete(mqtt::delivery_token_ptr tok) override
    {
        std::cout << "\n  [Delivery complete for token: "
                  << (tok ? tok->get_message_id() : -1) << "]" << std::endl;
    }

public:
};

// --------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    auto serverURI = (argc > 1) ? std::string{argv[1]} : DFLT_SERVER_URI;

    std::cout << "Initializing..." << std::endl;
    sample_mem_persistence persist;
    mqtt::client client(serverURI, "", &persist);

    user_callback cb;
    client.set_callback(cb);

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);
    std::cout << "...OK" << std::endl;

    try {
        std::cout << "\nConnecting..." << std::endl;
        client.connect(connOpts);
        std::cout << "...OK" << std::endl;

        // First use a message pointer.

        std::cout << "\nSending message..." << std::endl;
        auto pubmsg = mqtt::make_message(TOPIC, PAYLOAD1);
        pubmsg->set_qos(QOS);
        client.publish(pubmsg);
        std::cout << "...OK" << std::endl;

        // Now try with itemized publish.

        std::cout << "\nSending next message..." << std::endl;
        client.publish(TOPIC, PAYLOAD2, strlen(PAYLOAD2) + 1);
        std::cout << "...OK" << std::endl;

        // Now try with a listener, no token, and non-heap message

        std::cout << "\nSending final message..." << std::endl;
        client.publish(mqtt::message(TOPIC, PAYLOAD3, QOS, false));
        std::cout << "OK" << std::endl;

        // Disconnect
        std::cout << "\nDisconnecting..." << std::endl;
        client.disconnect();
        std::cout << "...OK" << std::endl;
    }
    catch (const mqtt::persistence_exception& exc) {
        std::cerr << "Persistence Error: " << exc.what() << " [" << exc.get_reason_code()
                  << "]" << std::endl;
        return 1;
    }
    catch (const mqtt::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    std::cout << "\nExiting" << std::endl;
    return 0;
}
