/////////////////////////////////////////////////////////////////////////////
/// @file token.h
/// Declaration of MQTT token class
/// @date May 1, 2013
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2013-2019 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi - MQTT v5 support & server responses
 *******************************************************************************/

#ifndef __mqtt_token_h
#define __mqtt_token_h

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "MQTTAsync.h"
#include "mqtt/buffer_ref.h"
#include "mqtt/exception.h"
#include "mqtt/iaction_listener.h"
#include "mqtt/properties.h"
#include "mqtt/server_response.h"
#include "mqtt/string_collection.h"
#include "mqtt/types.h"

namespace mqtt {

class iasync_client;

/////////////////////////////////////////////////////////////////////////////

/**
 * Provides a mechanism for tracking the completion of an asynchronous
 * action.
 */
class token
{
public:
    /** Smart/shared pointer to an object of this class */
    using ptr_t = std::shared_ptr<token>;
    /** Smart/shared pointer to an object of this class */
    using const_ptr_t = std::shared_ptr<const token>;
    /** Weak pointer to an object of this class */
    using weak_ptr_t = std::weak_ptr<token>;

    /** The type of request that the token is tracking */
    enum Type { CONNECT, SUBSCRIBE, PUBLISH, UNSUBSCRIBE, DISCONNECT };

private:
    /** Lock guard type for this class. */
    using guard = std::lock_guard<std::mutex>;
    /** Unique type for this class. */
    using unique_lock = std::unique_lock<std::mutex>;

    /** Object monitor mutex. */
    mutable std::mutex lock_;
    /** Condition variable signals when the action completes */
    mutable std::condition_variable cond_;

    /** The type of request that the token is tracking */
    Type type_;
    /** The MQTT client that is processing this action */
    iasync_client* cli_;
    /** The action success/failure code */
    int rc_{0};
    /** MQTT v5 reason code */
    ReasonCode reasonCode_{ReasonCode::SUCCESS};
    /** Error message from the C lib (if any) */
    string errMsg_;
    /** The underlying C token. Note that this is just an integer */
    MQTTAsync_token msgId_;
    /** The topic string(s) for the action being tracked by this token */
    const_string_collection_ptr topics_;
    /** User supplied context */
    void* userContext_;

    /**
     * User supplied listener.
     * Note that the user listener fires after the action is marked
     * complete, but before the token is signaled.
     */
    iaction_listener* listener_;
    /** The number of expected responses */
    size_t nExpected_;
    /** Whether the action has yet to complete */
    bool complete_;

    /** Connection response (null if not available) */
    std::unique_ptr<connect_response> connRsp_;
    /** Subscribe response (null if not available) */
    std::unique_ptr<subscribe_response> subRsp_;
    /** Unsubscribe response (null if not available) */
    std::unique_ptr<unsubscribe_response> unsubRsp_;

    /** Client and token-related options have special access */
    friend class async_client;
    friend class mock_async_client;

    friend class connect_options;
    friend class response_options;
    friend class delivery_response_options;
    friend class disconnect_options;

    /**
     * Resets the token back to a non-signaled state.
     */
    void reset();
    /**
     * Sets the ID for the message.
     * This is a guaranteed atomic operation.
     * @param msgId The ID of the message.
     */
    void set_message_id(MQTTAsync_token msgId) {
        guard g(lock_);
        msgId_ = msgId;
    }
    /**
     * C-style callback for success.
     * This simply passes the call on to the proper token object for
     * processing.
     * @param tokObj The token object to process the call. Note that this is
     *  			 @em not the user-supplied context pointer. That is
     *  			 kept in the object itself.
     * @param rsp The success response.
     */
    static void on_success(void* tokObj, MQTTAsync_successData* rsp);
    static void on_success5(void* tokObj, MQTTAsync_successData5* rsp);
    /**
     * C-style callback for failure.
     * This simply passes the call on to the proper token object for
     * processing.
     * @param tokObj The token object to process the call. Note that this is
     *  			 @em not the user-supplied context pointer. That is
     *  			 kept in the object itself.
     * @param rsp The failure response.
     */
    static void on_failure(void* tokObj, MQTTAsync_failureData* rsp);
    static void on_failure5(void* tokObj, MQTTAsync_failureData5* rsp);
    /**
     * C-style callback for client (re)connection.
     * This is normally only used to process a reconnect completion message.
     * The initial connect() is processed via on_success/failure.
     * @param tokObj Pointer to the token object used to process the call.
     */
    static void on_connected(void* tokObj, char* /*cause*/);
    /**
     * Internal handler for the success callback.
     * @param rsp The success response.
     */
    void on_success(MQTTAsync_successData* rsp);
    void on_success5(MQTTAsync_successData5* rsp);
    /**
     * Internal handler for the failure callback.
     * @param rsp The failure response.
     */
    void on_failure(MQTTAsync_failureData* rsp);
    void on_failure5(MQTTAsync_failureData5* rsp);

    /**
     * Check the current return code and throw an exception if it is not a
     * success code.
     */
    void check_ret() const {
        if (rc_ != MQTTASYNC_SUCCESS || reasonCode_ >= 0x80)
            throw exception(rc_, reasonCode_, errMsg_);
    }

public:
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     */
    token(Type typ, iasync_client& cli) : token{typ, cli, MQTTAsync_token(0)} {}
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when subscribe has
     *  		 completed
     */
    token(Type typ, iasync_client& cli, void* userContext, iaction_listener& cb)
        : token{typ, cli, const_string_collection_ptr(), userContext, cb} {}

    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topic The topic associated with the token
     */
    token(Type typ, iasync_client& cli, const string& topic)
        : token{typ, cli, string_collection::create(topic)} {}
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topic The topic associated with the token
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when subscribe has
     *  		 completed
     */
    token(
        Type typ, iasync_client& cli, const string& topic, void* userContext,
        iaction_listener& cb
    )
        : token{typ, cli, string_collection::create(topic), userContext, cb} {}

    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topics The topics associated with the token
     */
    token(Type typ, iasync_client& cli, const_string_collection_ptr topics);
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topics The topics associated with the token
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when subscribe has
     *  		 completed
     */
    token(
        Type typ, iasync_client& cli, const_string_collection_ptr topics, void* userContext,
        iaction_listener& cb
    );
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param tok The message ID
     */
    token(Type typ, iasync_client& cli, MQTTAsync_token tok);
    /**
     * Virtual destructor.
     */
    virtual ~token() {}
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @return A smart/shared pointer to a token.
     */
    static ptr_t create(Type typ, iasync_client& cli) {
        return std::make_shared<token>(typ, cli);
    }
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when subscribe has
     *  		 completed
     */
    static ptr_t create(
        Type typ, iasync_client& cli, void* userContext, iaction_listener& cb
    ) {
        return std::make_shared<token>(typ, cli, userContext, cb);
    }
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topic The topic associated with the token
     */
    static ptr_t create(Type typ, iasync_client& cli, const string& topic) {
        return std::make_shared<token>(typ, cli, topic);
    }
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topic The topic associated with the token
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when subscribe has
     *  		 completed
     */
    static ptr_t create(
        Type typ, iasync_client& cli, const string& topic, void* userContext,
        iaction_listener& cb
    ) {
        return std::make_shared<token>(typ, cli, topic, userContext, cb);
    }
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topics The topics associated with the token
     */
    static ptr_t create(Type typ, iasync_client& cli, const_string_collection_ptr topics) {
        return std::make_shared<token>(typ, cli, topics);
    }
    /**
     * Constructs a token object.
     * @param typ The type of request that the token is tracking.
     * @param cli The client that created the token.
     * @param topics The topics associated with the token
     *
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     * @param cb callback listener that will be notified when subscribe has
     */
    static ptr_t create(
        Type typ, iasync_client& cli, const_string_collection_ptr topics, void* userContext,
        iaction_listener& cb
    ) {
        return std::make_shared<token>(typ, cli, topics, userContext, cb);
    }
    /**
     * Gets the type of request the token is tracking, like CONNECT,
     * PUBLISH, etc.
     * @return The type of request that the token is tracking.
     */
    Type get_type() const { return type_; }
    /**
     * Gets the action listener for this token.
     * @return The action listener for this token.
     */
    virtual iaction_listener* get_action_callback() const {
        guard g(lock_);
        return listener_;
    }
    /**
     * Returns the MQTT client that is responsible for processing the
     * asynchronous action.
     * @return The client to which this token is connected.
     */
    virtual iasync_client* get_client() const { return cli_; }
    /**
     * Returns the ID of the message that is associated with the token.
     * @return The message ID of the transaction being tracked.
     */
    virtual int get_message_id() const {
        static_assert(sizeof(msgId_) <= sizeof(int), "MQTTAsync_token must fit into int");
        return int(msgId_);
    }
    /**
     * Gets the topic string(s) for the action being tracked by this
     * token.
     * @return A const pointer to the collection of topics being tracked by
     *  	   the token.
     */
    virtual const_string_collection_ptr get_topics() const { return topics_; }
    /**
     * Retrieve the context associated with an action.
     * @return The context associated with an action.
     */
    virtual void* get_user_context() const {
        guard g(lock_);
        return userContext_;
    }
    /**
     * Returns whether or not the action has finished.
     * @return @em true if the transaction has completed, @em false if not.
     */
    virtual bool is_complete() const { return complete_; }
    /**
     * Determines if the reference is valid.
     * If the reference is invalid then it is not safe to call @em any
     * member functions other than @em is_null() and @em empty()
     * @return @em true if referring to a valid buffer, @em false if the
     *  	   reference (pointer) is null.
     */
    explicit operator bool() const {
        guard g(lock_);
        return rc_ == MQTTASYNC_SUCCESS && reasonCode_ < 0x80;
    }
    /**
     * Gets the return code from the action.
     * This is only valid after the action has completed (i.e. if @ref
     * is_complete() returns @em true).
     * @return The return code from the action.
     */
    virtual int get_return_code() const { return rc_; }
    /**
     * Register a listener to be notified when an action completes.
     * @param listener The callback to be notified when actions complete.
     */
    virtual void set_action_callback(iaction_listener& listener);
    /**
     * Store some context associated with an action.
     * @param userContext optional object used to pass context to the
     *  				  callback. Use @em nullptr if not required.
     */
    virtual void set_user_context(void* userContext) {
        guard g(lock_);
        userContext_ = userContext;
    }
    /**
     * Sets the number of results expected.
     * This is only required for subscribe many() with < MQTTv5
     * @param n The number of results expected.
     */
    void set_num_expected(size_t n) { nExpected_ = n; }
    /**
     * Gets the reason code for the operation.
     * @return The reason code for the operation.
     */
    ReasonCode get_reason_code() const { return reasonCode_; }
    /**
     * Get the error message from the C library
     * @return Error message for the operation
     */
    string get_error_message() const { return errMsg_; }
    /**
     * Blocks the current thread until the action this token is associated
     * with has completed.
     */
    virtual void wait();
    /**
     * Non-blocking check to see if the action has completed.
     * @return @em true if the wait finished successfully, @em false if the
     *  	   action has not completed yet.
     */
    virtual bool try_wait() {
        guard g(lock_);
        if (complete_)
            check_ret();
        return complete_;
    }
    /**
     * Blocks the current thread until the action this token is associated
     * with has completed.
     * @param timeout The timeout (in milliseconds)
     * @return @em true if the wait finished successfully, @em false if a
     *  	   timeout occurred.
     */
    virtual bool wait_for(long timeout) {
        return wait_for(std::chrono::milliseconds(timeout));
    }
    /**
     * Waits a relative amount of time for the action to complete.
     * @param relTime The amount of time to wait for the event.
     * @return @em true if the event gets signaled in the specified time,
     *  	   @em false on a timeout.
     */
    template <class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& relTime) {
        unique_lock g(lock_);
        if (!cond_.wait_for(g, std::chrono::milliseconds(relTime), [this] {
                return complete_;
            }))
            return false;
        check_ret();
        return true;
    }
    /**
     * Waits until an absolute time for the action to complete.
     * @param absTime The absolute time to wait for the event.
     * @return @em true if the event gets signaled in the specified time,
     *  	   @em false on a timeout.
     */
    template <class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& absTime) {
        unique_lock g(lock_);
        if (!cond_.wait_until(g, absTime, [this] { return complete_; }))
            return false;
        check_ret();
        return true;
    }

    /**
     * Gets the response from a connect operation.
     * This returns the result of the completed operation. If the
     * operation is not yet complete this will block until the result
     * is available.
     * @return The result of the operation.
     */
    connect_response get_connect_response() const;
    /**
     * Gets the response from a connect operation.
     * This returns the result of the completed operation. If the
     * operation is not yet complete this will block until the result
     * is available.
     * @return The result of the operation.
     */
    subscribe_response get_subscribe_response() const;
    /**
     * Gets the response from a connect operation.
     * This returns the result of the completed operation. If the
     * operation is not yet complete this will block until the result
     * is available.
     * @return The result of the operation.
     */
    unsubscribe_response get_unsubscribe_response() const;
};

/** Smart/shared pointer to a token object */
using token_ptr = token::ptr_t;

/** Smart/shared pointer to a const token object */
using const_token_ptr = token::const_ptr_t;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_token_h
