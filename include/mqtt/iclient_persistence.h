/////////////////////////////////////////////////////////////////////////////
/// @file iclient_persistence.h
/// Declaration of MQTT iclient_persistence interface
/// @date May 1, 2013
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2013-2016 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_iclient_persistence_h
#define __mqtt_iclient_persistence_h

#include <vector>

#include "MQTTAsync.h"
#include "mqtt/buffer_view.h"
#include "mqtt/string_collection.h"
#include "mqtt/types.h"

namespace mqtt {

/**
 * Allocate memory for use with user persistence.
 *
 * @param n The number of bytes for the buffer.
 * @return A pointer to the allocated memory
 */
inline void* persistence_malloc(size_t n) { return MQTTAsync_malloc(n); }

/**
 * Frees memory allocated with @ref persistence_malloc().
 * @param p Pointer to a buffer obtained by persistence_malloc.
 */
inline void persistence_free(void* p) { MQTTAsync_free(p); }

/////////////////////////////////////////////////////////////////////////////

/**
 * Represents a persistent data store, used to store outbound and inbound
 * messages while they are in flight, enabling delivery to the QoS
 * specified. You can specify an implementation of this interface using
 * client::client(string, string, iclient_persistence), which the
 * client will use to persist QoS 1 and 2 messages.
 *
 * If the methods defined throw the MqttPersistenceException then the state
 * of the data persisted should remain as prior to the method being called.
 * For example, if put(string, persistable) throws an exception at any
 * point then the data will be assumed to not be in the persistent store.
 * Similarly if remove(string) throws an exception then the data will be
 * assumed to still be held in the persistent store.
 *
 * It is up to the persistence interface to log any exceptions or error
 * information which may be required when diagnosing a persistence failure.
 */
class iclient_persistence
{
    friend class async_client;
    friend class mock_persistence;

    /** Callbacks from the C library */
    static int persistence_open(
        void** handle, const char* clientID, const char* serverURI, void* context
    );
    static int persistence_close(void* handle);
    static int persistence_put(
        void* handle, char* key, int bufcount, char* buffers[], int buflens[]
    );
    static int persistence_get(void* handle, char* key, char** buffer, int* buflen);
    static int persistence_remove(void* handle, char* key);
    static int persistence_keys(void* handle, char*** keys, int* nkeys);
    static int persistence_clear(void* handle);
    static int persistence_containskey(void* handle, char* key);

public:
    /** Smart/shared pointer to an object of this class. */
    using ptr_t = std::shared_ptr<iclient_persistence>;
    /** Smart/shared pointer to a const object of this class. */
    using const_ptr_t = std::shared_ptr<const iclient_persistence>;

    /**
     * Virtual destructor.
     */
    virtual ~iclient_persistence() {}
    /**
     * Initialize the persistent store.
     * This uses the client ID and server name to create a unique location
     * for the data store.
     * @param clientId The identifier string for the client.
     * @param serverURI The server to which the client is connected.
     */
    virtual void open(const string& clientId, const string& serverURI) = 0;
    /**
     * Close the persistent store that was previously opened.
     */
    virtual void close() = 0;
    /**
     * Clears persistence, so that it no longer contains any persisted data.
     */
    virtual void clear() = 0;
    /**
     * Returns whether or not data is persisted using the specified key.
     * @param key The key to find
     * @return @em true if the key exists, @em false if not.
     */
    virtual bool contains_key(const string& key) = 0;
    /**
     * Returns a collection of keys in this persistent data store.
     * @return A collection of strings representing the keys in the store.
     */
    virtual string_collection keys() const = 0;
    /**
     * Puts the specified data into the persistent store.
     * @param key The key.
     * @param bufs The data to store
     */
    virtual void put(const string& key, const std::vector<string_view>& bufs) = 0;
    /**
     * Gets the specified data out of the persistent store.
     * @param key The key
     * @return A const view of the data associated with the key.
     */
    virtual string get(const string& key) const = 0;
    /**
     * Remove the data for the specified key.
     * @param key The key
     */
    virtual void remove(const string& key) = 0;
};

/** Smart/shared pointer to a persistence client */
using iclient_persistence_ptr = iclient_persistence::ptr_t;

/** Smart/shared pointer to a persistence client */
using const_iclient_persistence_ptr = iclient_persistence::const_ptr_t;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_iclient_persistence_h
