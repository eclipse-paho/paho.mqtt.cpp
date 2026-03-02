/////////////////////////////////////////////////////////////////////////////
/// @file buffer_view.h
/// Buffer reference type for the Paho MQTT C++ library.
/// @date April 18, 2017
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2017-2020 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_buffer_view_h
#define __mqtt_buffer_view_h

#include <iostream>

#include "mqtt/types.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * A reference to a contiguous sequence of items, with no ownership.
 * This simply contains a pointer to a const array of items, and a size.
 * This is a similar, but simplified version of the std::string_view
 * class(es) in the C++17 standard.
 */
template <typename T>
class buffer_view
{
public:
    /** The type of items to be held in the queue. */
    using value_type = T;
    /** The type used to specify number of items in the container. */
    using size_type = size_t;

private:
    /** Const pointer to the data array */
    const value_type* data_;
    /** The size of the array */
    size_type sz_;

public:
    /**
     * Constructs a buffer view.
     * @param data The data pointer
     * @param n The number of items
     */
    buffer_view(const value_type* data, size_type n) : data_(data), sz_(n) {}
    /**
     * Constructs a buffer view to a whole string.
     * This the starting pointer and length of the whole string.
     * @param str The string.
     */
    buffer_view(const std::basic_string<value_type>& str)
        : data_(str.data()), sz_(str.size()) {}
    /**
     * Gets a pointer the first item in the view.
     * @return A const pointer the first item in the view.
     */
    const value_type* data() const { return data_; }
    /**
     * Gets the number of items in the view.
     * @return The number of items in the view.
     */
    size_type size() const { return sz_; }
    /**
     * Gets the number of items in the view.
     * @return The number of items in the view.
     */
    size_type length() const { return sz_; }
    /**
     * Access an item in the view.
     * @param i The index of the item.
     * @return A const reference to the requested item.
     */
    const value_type& operator[](size_t i) const { return data_[i]; }
    /**
     * Gets a copy of the view as a string.
     * @return A copy of the view as a string.
     */
    std::basic_string<value_type> str() const {
        return std::basic_string<value_type>(data_, sz_);
    }
    /**
     * Gets a copy of the view as a string.
     * @return A copy of the view as a string.
     */
    string to_string() const {
        static_assert(
            sizeof(char) == sizeof(T), "can only get string for char or byte buffers"
        );
        return string(reinterpret_cast<const char*>(data_), sz_);
    }
};

/**
 * Stream inserter for a buffer view.
 * This does a binary write of the data in the buffer.
 * @param os The output stream.
 * @param buf The buffer reference to write.
 * @return A reference to the output stream.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const buffer_view<T>& buf) {
    if (buf.size() > 0)
        os.write(buf.data(), sizeof(T) * buf.size());
    return os;
}

/** A buffer view for character string data. */
using string_view = buffer_view<char>;

/** A buffer view for binary data */
using binary_view = buffer_view<char>;

/////////////////////////////////////////////////////////////////////////////
// end namespace mqtt
}  // namespace mqtt

#endif  // __mqtt_buffer_view_h
