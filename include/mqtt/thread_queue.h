/////////////////////////////////////////////////////////////////////////////
/// @file thread_queue.h
/// Implementation of the template class 'thread_queue', a thread-safe,
/// blocking queue for passing data between threads, safe for use with smart
/// pointers.
/// @date 09-Jan-2017
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2017-2022 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_thread_queue_h
#define __mqtt_thread_queue_h

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <limits>
#include <mutex>
#include <queue>
#include <thread>

namespace mqtt {

/**
 * Exception that is thrown when operations are performed on a closed
 * queue.
 */
class queue_closed : public std::runtime_error
{
public:
    queue_closed() : std::runtime_error("queue is closed") {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A thread-safe queue for inter-thread communication.
 *
 * This is a locking queue with blocking operations. The get() operations
 * can always block on an empty queue, but have variations for non-blocking
 * (try_get) and bounded-time blocking (try_get_for, try_get_until).
 * @par
 * The default queue has a capacity that is unbounded in the practical
 * sense, limited by available memory. In this mode the object will not
 * block when placing values into the queue. A capacity can bet set with the
 * constructor or, at any time later by calling the @ref capacity(size_type)
 * method. Using this latter method, the capacity can be set to an amount
 * smaller than the current size of the queue. In that case all put's to the
 * queue will block until the number of items are removed from the queue to
 * bring the size below the new capacity.
 * @par
 * The queue can be closed. After that, no new items can be placed into it;
 * a `put()` calls will fail. Receivers can still continue to get any items
 * out of the queue that were added before it was closed. Once there are no
 * more items left in the queue after it is closed, it is considered "done".
 * Nothing useful can be done with the queue.
 * @par
 * Note that the queue uses move semantics to place items into the queue and
 * remove items from the queue. This means that the type, T, of the data
 * held by the queue only needs to follow move semantics; not copy
 * semantics. In addition, this means that copies of the value will @em not
 * be left in the queue. This is especially useful when creating queues of
 * shared pointers, as the "dead" part of the queue will not hold onto a
 * reference count after the item has been removed from the queue.
 *
 * @tparam T The type of the items to be held in the queue.
 * @tparam Container The type of the underlying container to use. It must
 * support back(), front(), push_back(), pop_front().
 */
template <typename T, class Container = std::deque<T>>
class thread_queue
{
public:
    /** The underlying container type to use for the queue. */
    using container_type = Container;
    /** The type of items to be held in the queue. */
    using value_type = T;
    /** The type used to specify number of items in the container. */
    using size_type = typename Container::size_type;

    /** The maximum capacity of the queue. */
    static constexpr size_type MAX_CAPACITY = std::numeric_limits<size_type>::max();

private:
    /** Object lock */
    mutable std::mutex lock_;
    /** Condition get signaled when item added to empty queue */
    std::condition_variable notEmptyCond_;
    /** Condition gets signaled then item removed from full queue */
    std::condition_variable notFullCond_;
    /** The capacity of the queue */
    size_type cap_{MAX_CAPACITY};
    /** Whether the queue is closed */
    bool closed_{false};

    /** The actual STL container to hold data */
    std::queue<T, Container> que_;

    /** Simple, scope-based lock guard */
    using guard = std::lock_guard<std::mutex>;
    /** General purpose guard */
    using unique_guard = std::unique_lock<std::mutex>;

    /** Checks if the queue is done (unsafe) */
    bool is_done() const { return closed_ && que_.empty(); }

public:
    /**
     * Constructs a queue with the maximum capacity.
     * This is effectively an unbounded queue.
     */
    thread_queue() {}
    /**
     * Constructs a queue with the specified capacity.
     * This is a bounded queue.
     * @param cap The maximum number of items that can be placed in the
     *  		  queue. The minimum capacity is 1.
     */
    explicit thread_queue(size_t cap) : cap_(std::max<size_type>(cap, 1)) {}
    /**
     * Determine if the queue is empty.
     * @return @em true if there are no elements in the queue, @em false if
     *  	   there are any items in the queue.
     */
    bool empty() const {
        guard g{lock_};
        return que_.empty();
    }
    /**
     * Gets the capacity of the queue.
     * @return The maximum number of elements before the queue is full.
     */
    size_type capacity() const {
        guard g{lock_};
        return cap_;
    }
    /**
     * Sets the capacity of the queue.
     * Note that the capacity can be set to a value smaller than the current
     * size of the queue. In that event, all calls to put() will block until
     * a sufficient number of items are removed from the queue.
     */
    void capacity(size_type cap) {
        guard g{lock_};
        cap_ = std::max<size_type>(cap, 1);
        if (cap_ > que_.size())
            notFullCond_.notify_all();
    }
    /**
     * Gets the number of items in the queue.
     * @return The number of items in the queue.
     */
    size_type size() const {
        guard g{lock_};
        return que_.size();
    }
    /**
     * Close the queue.
     * Once closed, the queue will not accept any new items, but receievers
     * will still be able to get any remaining items out of the queue until
     * it is empty.
     */
    void close() {
        guard g{lock_};
        closed_ = true;
        notFullCond_.notify_all();
        notEmptyCond_.notify_all();
    }
    /**
     * Determines if the queue is closed.
     * Once closed, the queue will not accept any new items, but receievers
     * will still be able to get any remaining items out of the queue until
     * it is empty.
     * @return @em true if the queue is closed, @em false otherwise.
     */
    bool closed() const {
        guard g{lock_};
        return closed_;
    }
    /**
     * Determines if all possible operations are done on the queue.
     * If the queue is closed and empty, then no further useful operations
     * can be done on it.
     * @return @em true if the queue is closed and empty, @em false otherwise.
     */
    bool done() const {
        guard g{lock_};
        return is_done();
    }
    /**
     * Clear the contents of the queue.
     * This discards all items in the queue.
     */
    void clear() {
        guard g{lock_};
        while (!que_.empty()) que_.pop();
        notFullCond_.notify_all();
    }
    /**
     * Put an item into the queue.
     * If the queue is full, this will block the caller until items are
     * removed bringing the size less than the capacity.
     * @param val The value to add to the queue.
     */
    void put(value_type val) {
        unique_guard g{lock_};
        notFullCond_.wait(g, [this] { return que_.size() < cap_ || closed_; });
        if (closed_)
            throw queue_closed{};

        que_.emplace(std::move(val));
        notEmptyCond_.notify_one();
    }
    /**
     * Non-blocking attempt to place an item into the queue.
     * @param val The value to add to the queue.
     * @return @em true if the item was added to the queue, @em false if the
     *  	   item was not added because the queue is currently full.
     */
    bool try_put(value_type val) {
        guard g{lock_};
        if (que_.size() >= cap_ || closed_)
            return false;

        que_.emplace(std::move(val));
        notEmptyCond_.notify_one();
        return true;
    }
    /**
     * Attempt to place an item in the queue with a bounded wait.
     * This will attempt to place the value in the queue, but if it is full,
     * it will wait up to the specified time duration before timing out.
     * @param val The value to add to the queue.
     * @param relTime The amount of time to wait until timing out.
     * @return @em true if the value was added to the queue, @em false if a
     *  	   timeout occurred.
     */
    template <typename Rep, class Period>
    bool try_put_for(value_type val, const std::chrono::duration<Rep, Period>& relTime) {
        unique_guard g{lock_};
        bool to = !notFullCond_.wait_for(g, relTime, [this] {
            return que_.size() < cap_ || closed_;
        });
        if (to || closed_)
            return false;

        que_.emplace(std::move(val));
        notEmptyCond_.notify_one();
        return true;
    }
    /**
     * Attempt to place an item in the queue with a bounded wait to an
     * absolute time point.
     * This will attempt to place the value in the queue, but if it is full,
     * it will wait up until the specified time before timing out.
     * @param val The value to add to the queue.
     * @param absTime The absolute time to wait to before timing out.
     * @return @em true if the value was added to the queue, @em false if a
     *  	   timeout occurred.
     */
    template <class Clock, class Duration>
    bool try_put_until(
        value_type val, const std::chrono::time_point<Clock, Duration>& absTime
    ) {
        unique_guard g{lock_};
        bool to = !notFullCond_.wait_until(g, absTime, [this] {
            return que_.size() < cap_ || closed_;
        });

        if (to || closed_)
            return false;

        que_.emplace(std::move(val));
        notEmptyCond_.notify_one();
        return true;
    }
    /**
     * Retrieve a value from the queue.
     * If the queue is empty, this will block indefinitely until a value is
     * added to the queue by another thread,
     * @param val Pointer to a variable to receive the value.
     */
    bool get(value_type* val) {
        if (!val)
            return false;

        unique_guard g{lock_};
        notEmptyCond_.wait(g, [this] { return !que_.empty() || closed_; });
        if (que_.empty())  // We must be done
            return false;

        *val = std::move(que_.front());
        que_.pop();
        notFullCond_.notify_one();
        return true;
    }
    /**
     * Retrieve a value from the queue.
     * If the queue is empty, this will block indefinitely until a value is
     * added to the queue by another thread,
     * @return The value removed from the queue
     */
    value_type get() {
        unique_guard g{lock_};
        notEmptyCond_.wait(g, [this] { return !que_.empty() || closed_; });
        if (que_.empty())  // We must be done
            throw queue_closed{};

        value_type val = std::move(que_.front());
        que_.pop();
        notFullCond_.notify_one();
        return val;
    }
    /**
     * Attempts to remove a value from the queue without blocking.
     * If the queue is currently empty, this will return immediately with a
     * failure, otherwise it will get the next value and return it.
     * @param val Pointer to a variable to receive the value.
     * @return @em true if a value was removed from the queue, @em false if
     *  	   the queue is empty.
     */
    bool try_get(value_type* val) {
        if (!val)
            return false;

        guard g{lock_};
        if (que_.empty())
            return false;

        *val = std::move(que_.front());
        que_.pop();
        notFullCond_.notify_one();
        return true;
    }
    /**
     * Attempt to remove an item from the queue for a bounded amount of time.
     * This will retrieve the next item from the queue. If the queue is
     * empty, it will wait the specified amount of time for an item to arrive
     * before timing out.
     * @param val Pointer to a variable to receive the value.
     * @param relTime The amount of time to wait until timing out.
     * @return @em true if the value was removed the queue, @em false if a
     *  	   timeout occurred.
     */
    template <typename Rep, class Period>
    bool try_get_for(value_type* val, const std::chrono::duration<Rep, Period>& relTime) {
        if (!val)
            return false;

        unique_guard g{lock_};
        notEmptyCond_.wait_for(g, relTime, [this] { return !que_.empty() || closed_; });

        if (que_.empty())
            return false;

        *val = std::move(que_.front());
        que_.pop();
        notFullCond_.notify_one();
        return true;
    }
    /**
     * Attempt to remove an item from the queue for a bounded amount of time.
     * This will retrieve the next item from the queue. If the queue is
     * empty, it will wait until the specified time for an item to arrive
     * before timing out.
     * @param val Pointer to a variable to receive the value.
     * @param absTime The absolute time to wait to before timing out.
     * @return @em true if the value was removed from the queue, @em false
     *  	   if a timeout occurred.
     */
    template <class Clock, class Duration>
    bool try_get_until(
        value_type* val, const std::chrono::time_point<Clock, Duration>& absTime
    ) {
        if (!val)
            return false;

        unique_guard g{lock_};
        notEmptyCond_.wait_until(g, absTime, [this] { return !que_.empty() || closed_; });
        if (que_.empty())
            return false;

        *val = std::move(que_.front());
        que_.pop();
        notFullCond_.notify_one();
        return true;
    }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_thread_queue_h
