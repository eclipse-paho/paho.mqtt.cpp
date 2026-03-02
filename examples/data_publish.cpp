// data_publish.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// It's an example of how to collect and publish periodic data to MQTT, as
// an MQTT publisher using the C++ asynchronous client interface.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Publishing messages
//  - Using a topic object to repeatedly publish to the same topic.
//  - Automatic reconnects
//  - Off-line buffering
//  - User file-based persistence with simple encoding.
//
// This just uses the steady clock to run a periodic loop. Each time
// through, it generates a random number [0-100] as simulated data and
// creates a text, CSV payload in the form:
//  	<sample #>,<time stamp>,<data>
//
// Note that it uses the steady clock to pace the periodic timing, but then
// reads the system_clock to generate the timestamp for local calendar time.
//
// The sample number is just a counting integer to help test the off-line
// buffering to easily confirm that all the messages got across.
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

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>

#include "mqtt/async_client.h"

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

const std::string DFLT_SERVER_URI{"mqtt://localhost:1883"};
const std::string CLIENT_ID{"paho-cpp-data-publish"};

const string TOPIC{"data/rand"};
const int QOS = 1;

// How often we output a data point
const auto PERIOD = seconds(5);

// The number of out-bound messages we will buffer locally when disconnected.
const int MAX_BUFFERED_MSGS = 120;  // 120 * 5sec => 10min off-line buffering

// Top-level directory to keep persistence data
const fs::path PERSIST_DIR{"persist"};

// A key for encoding the persistence data
const string PERSIST_KEY{"elephant"};

// Class to pace timing and signal and exit without delay.
class quit_signal
{
    condition_variable cv_;
    mutex mtx_;
    bool quit_{false};

public:
    template <class Clock, class Duration>
    bool wait_until(const time_point<Clock, Duration>& abs_time)
    {
        unique_lock lk(mtx_);
        return cv_.wait_until(lk, abs_time, [this] { return quit_; });
    }

    void signal()
    {
        unique_lock<mutex> lk(mtx_);
        quit_ = true;
        lk.unlock();
        cv_.notify_one();
    }
};

// Variable to pace timing and signal exit
quit_signal quit;

/////////////////////////////////////////////////////////////////////////////

// Example of user-based file persistence with a simple XOR encoding scheme.
//
// Similar to the built-in file persistence, this just creates a
// subdirectory for the persistence data, then places each key into a
// separate file using the key as the file name.
//
// With user-defined persistence, you can transform the data in any way you
// like, such as with encryption/decryption, and you can store the data any
// place you want, such as here with disk files, or use a local DB like
// SQLite or a local key/value store like Redis.
class encoded_file_persistence : virtual public mqtt::iclient_persistence
{
    // The directory for the persistence store.
    fs::path dir_;

    // A key for encoding the data, as supplied by the user
    string encodeKey_;

    // Simple, in-place XOR encoding and decoding
    void encode(string& s) const
    {
        size_t n = encodeKey_.size();
        if (n == 0 || s.empty())
            return;

        for (size_t i = 0; i < s.size(); ++i) s[i] ^= encodeKey_[i % n];
    }

    // Gets the persistence file name for the supplied key.
    fs::path path_name(const string& key) const { return dir_ / key; }

public:
    // Create the persistence object with the specified encoding key
    encoded_file_persistence(const string& encodeKey) : encodeKey_(encodeKey) {}

    // "Open" the persistence store.
    // Create a directory for persistence files, using the client ID and
    // serverURI to make a unique directory name. Note that neither can be
    // empty. In particular, the app can't use an empty `clientID` if it
    // wants to use persistence. (This isn't an absolute rule for your own
    // persistence, but you do need a way to keep data from different apps
    // separate).
    void open(const string& clientId, const string& serverURI) override
    {
        if (clientId.empty() || serverURI.empty())
            throw mqtt::persistence_exception();

        // Create a name for the persistence subdirectory for this client
        string name = serverURI + "-" + clientId;
        std::replace(name.begin(), name.end(), ':', '-');

        dir_ = PERSIST_DIR;
        dir_ /= name;

        fs::create_directories(dir_);
    }

    // Close the persistent store that was previously opened.
    // Remove the persistence directory, if it's empty.
    void close() override
    {
        fs::remove(dir_);
        fs::remove(dir_.parent_path());
    }

    // Clears persistence, so that it no longer contains any persisted data.
    // Just remove all the files from the persistence directory.
    void clear() override
    {
        // We could iterate through and remove each file,
        // but this does the same thing in fewer steps.
        if (!fs::is_empty(dir_)) {
            fs::remove_all(dir_);
            fs::create_directories(dir_);
        }
    }

    // Returns whether or not data is persisted using the specified key.
    // We just look for a file in the store directory with the same name as
    // the key.
    bool contains_key(const string& key) override
    {
        if (fs::exists(dir_)) {
            for (const auto& entry : fs::directory_iterator(dir_)) {
                if (entry.path().filename() == key)
                    return true;
            }
        }
        return false;
    }

    // Returns the keys in this persistent data store.
    // We just make a collection of the file names in the store directory.
    mqtt::string_collection keys() const override
    {
        mqtt::string_collection ks;

        if (fs::exists(dir_)) {
            for (const auto& entry : fs::directory_iterator(dir_)) {
                ks.push_back(entry.path().filename().string());
            }
        }
        return ks;
    }

    // Puts the specified data into the persistent store.
    // We just encode the data and write it to a file using the key as the
    // name of the file. The multiple buffers given here need to be written
    // in order - and a scatter/gather like writev() would be fine. But...
    // the data will be read back as a single buffer, so here we first
    // concat a string so that the encoding key lines up with the data the
    // same way it will on the read-back.
    void put(const string& key, const std::vector<mqtt::string_view>& bufs) override
    {
        auto path = path_name(key);

        ofstream os(path, ios_base::binary);
        if (!os)
            throw mqtt::persistence_exception();

        string s;
        for (const auto& b : bufs) s.append(b.data(), b.size());

        encode(s);
        os.write(s.data(), s.size());
    }

    // Gets the specified data out of the persistent store.
    // We look for a file with the name of the key, read the contents,
    // decode, and return it.
    string get(const string& key) const override
    {
        auto path = path_name(key);

        ifstream is(path, ios_base::ate | ios_base::binary);
        if (!is)
            throw mqtt::persistence_exception();

        // Read the whole file into a string
        streamsize sz = is.tellg();
        if (sz == 0)
            return string();

        is.seekg(0);
        string s(sz, '\0');
        is.read(&s[0], sz);
        if (is.gcount() < sz)
            s.resize(is.gcount());

        encode(s);
        return s;
    }

    // Remove the data for the specified key.
    // Just remove the file with the same name as the key, if found.
    void remove(const string& key) override
    {
        auto path = path_name(key);
        fs::remove(path);
    }
};

/////////////////////////////////////////////////////////////////////////////

// Handler for ^C (SIGINT)
void ctrlc_handler(int) { quit.signal(); }

// --------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    string serverURI = (argc > 1) ? string{argv[1]} : DFLT_SERVER_URI;

    // Create a persistence object
    encoded_file_persistence persist{PERSIST_KEY};

    // Create a client to use the persistence.
    mqtt::async_client cli(serverURI, CLIENT_ID, MAX_BUFFERED_MSGS, &persist);

    auto connOpts = mqtt::connect_options_builder()
                        .keep_alive_interval(MAX_BUFFERED_MSGS * PERIOD)
                        .clean_session(false)
                        .automatic_reconnect(true)
                        .finalize();

    // Create a topic object. This is a conventience since we will
    // repeatedly publish messages with the same parameters.
    mqtt::topic top(cli, TOPIC, QOS, true);

    // Random number generator [0 - 100]
    random_device rnd;
    mt19937 gen(rnd());
    uniform_int_distribution<> dis(0, 100);

    try {
        // Connect to the MQTT broker
        cout << "Connecting to server '" << serverURI << "'..." << flush;
        cli.connect(connOpts)->wait();
        cout << "OK\n" << endl;

        char tmbuf[32];
        unsigned nsample = 0;

        // Install a ^C handler for user to signal when to exit
        signal(SIGINT, ctrlc_handler);

        // The steady time at which to read the next sample
        auto tm = steady_clock::now() + 250ms;

        // Pace the sampling by letting the condition variable time out
        // periodically. When 'quit' is signaled, it's time to quit.
        while (!quit.wait_until(tm)) {
            // Get a timestamp and format as a string
            time_t t = system_clock::to_time_t(system_clock::now());
            strftime(tmbuf, sizeof(tmbuf), "%F %T", localtime(&t));

            // Simulate reading some data
            int x = dis(gen);

            // Create the payload as a text CSV string
            string payload = to_string(++nsample) + "," + tmbuf + "," + to_string(x);
            cout << payload << endl;

            // Publish to the topic
            top.publish(std::move(payload));

            tm += PERIOD;
        }

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
