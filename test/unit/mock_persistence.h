// mock_persistence.h
//
// Dummy implementation of mqtt::iclient_persistence for Unit Test.
//

/*******************************************************************************
 * Copyright (c) 2016 Guilherme M. Ferreira <guilherme.maciel.ferreira@gmail.com>
 * Copyright (c) 2020 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Guilherme M. Ferreira - initial implementation and documentation
 *    Frank Pagliughi - Renamed 'mock', refactored for coding conventions
 *******************************************************************************/

#ifndef __mqtt_mock_persistence_h
#define __mqtt_mock_persistence_h

#include "mqtt/exception.h"
#include "mqtt/iclient_persistence.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

// Dummy persistence is a working in-memory persistence class.
class mock_persistence : virtual public iclient_persistence
{
    using base = iclient_persistence;

    // Whether the store is open
    bool open_{false};

    // Use an STL map to store shared persistence pointers
    // against string keys.
    std::map<std::string, std::string> store_;

public:
    static int persistence_open(
        void** handle, const char* clientID, const char* serverURI, void* context
    )
    {
        return base::persistence_open(handle, clientID, serverURI, context);
    }
    static int persistence_close(void* handle) { return base::persistence_close(handle); }
    static int persistence_put(
        void* handle, char* key, int bufcount, char* buffers[], int buflens[]
    )
    {
        return base::persistence_put(handle, key, bufcount, buffers, buflens);
    }
    static int persistence_get(void* handle, char* key, char** buffer, int* buflen)
    {
        return base::persistence_get(handle, key, buffer, buflen);
    }
    static int persistence_remove(void* handle, char* key)
    {
        return base::persistence_remove(handle, key);
    }
    static int persistence_keys(void* handle, char*** keys, int* nkeys)
    {
        return base::persistence_keys(handle, keys, nkeys);
    }
    static int persistence_clear(void* handle) { return base::persistence_clear(handle); }
    static int persistence_containskey(void* handle, char* key)
    {
        return base::persistence_containskey(handle, key);
    }

    // "Open" the store
    void open(const std::string& clientId, const std::string& serverURI) override
    {
        open_ = true;
    }

    // Close the persistent store that was previously opened.
    void close() override { open_ = false; }

    // Clears persistence, so that it no longer contains any persisted data.
    void clear() override { store_.clear(); }

    // Returns whether or not data is persisted using the specified key.
    bool contains_key(const std::string& key) override
    {
        return store_.find(key) != store_.end();
    }

    // Returns the keys in this persistent data store.
    // This could be more efficient, but you get the point.
    mqtt::string_collection keys() const override
    {
        mqtt::string_collection ks;
        for (const auto& k : store_) ks.push_back(k.first);
        return ks;
    }

    // Puts the specified data into the persistent store.
    void put(const std::string& key, const std::vector<mqtt::string_view>& bufs) override
    {
        std::string str;
        for (const auto& b : bufs) str += b.str();
        store_[key] = std::move(str);
    }

    // Gets the specified data out of the persistent store.
    std::string get(const std::string& key) const override
    {
        auto p = store_.find(key);
        if (p == store_.end())
            throw mqtt::persistence_exception();
        return p->second;
    }

    // Remove the data for the specified key.
    void remove(const std::string& key) override
    {
        auto p = store_.find(key);
        if (p == store_.end())
            throw mqtt::persistence_exception();
        store_.erase(p);
    }
};

/////////////////////////////////////////////////////////////////////////////
// end namespace mqtt
}  // namespace mqtt

#endif  // __mqtt_mock_persistence_h
