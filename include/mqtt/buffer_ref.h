/////////////////////////////////////////////////////////////////////////////
/// @file buffer_ref.h
/// Buffer reference type for the Paho MQTT C++ library.
/// @date April 18, 2017
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2017-2024 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_buffer_ref_h
#define __mqtt_buffer_ref_h

#include <cstring>
#include <iostream>

#include "mqtt/types.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * A reference object for holding immutable data buffers, with cheap copy
 * semantics and lifetime management.
 *
 * Each object of this class contains a reference-counted pointer to an
 * immutable data buffer. Objects can be copied freely and easily, even
 * across threads, since all instances promise not to modify the contents
 * of the buffer.
 *
 * The buffer is immutable but the reference itself acts like a normal
 * variable. It can be reassigned to point to a different buffer.
 *
 * If no value has been assigned to a reference, then it is in a default
 * "null" state. It is not safe to call any member functions on a null
 * reference, other than to check if the object is null or empty.
 * @verbatim
 * string_ref sr;
 * if (!sr)
 *   cout << "null reference" << endl;
 * else
 *   cout.write(sr.data(), sr.size());
 * @endverbatim
 */
template <typename T>
class buffer_ref
{
public:
    /**
     * The underlying type for the buffer.
     * Normally byte-wide data (char or uint8_t) for Paho.
     */
    using value_type = T;
    /**
     * The type for the buffer.
     * We use basic_string for compatibility with string data.
     */
    using blob = std::basic_string<value_type>;
    /**
     *  The pointer we use.
     *  Note that it is a pointer to a _const_ blob.
     */
    using pointer_type = std::shared_ptr<const blob>;

private:
    /** Our data is a shared pointer to a const buffer */
    pointer_type data_;

public:
    /**
     * Default constructor creates a null reference.
     */
    buffer_ref() = default;
    /**
     * Copy constructor only copies a shared pointer.
     * @param buf Another buffer reference.
     */
    buffer_ref(const buffer_ref& buf) = default;
    /**
     * Move constructor only moves a shared pointer.
     * @param buf Another buffer reference.
     */
    buffer_ref(buffer_ref&& buf) = default;
    /**
     * Creates a reference to a new buffer by copying data.
     * @param b A string from which to create a new buffer.
     */
    buffer_ref(const blob& b) : data_{std::make_shared<blob>(b)} {}
    /**
     * Creates a reference to a new buffer by moving a string into the
     * buffer.
     * @param b A string from which to create a new buffer.
     */
    buffer_ref(blob&& b) : data_{std::make_shared<blob>(std::move(b))} {}
    /**
     * Creates a reference to an existing buffer by copying the shared
     * pointer.
     * Note that it is up to the caller to insure that there are no mutable
     * references to the buffer.
     * @param p A shared pointer to a string.
     */
    buffer_ref(const pointer_type& p) : data_(p) {}
    /**
     * Creates a reference to an existing buffer by moving the shared
     * pointer.
     * Note that it is up to the caller to insure that there are no mutable
     * references to the buffer.
     * @param p A shared pointer to a string.
     */
    buffer_ref(pointer_type&& p) : data_(std::move(p)) {}
    /**
     * Creates a reference to a new buffer containing a copy of the data.
     * @param buf The memory to copy
     * @param n The number of bytes to copy.
     */
    buffer_ref(const value_type* buf, size_t n) : data_{std::make_shared<blob>(buf, n)} {}
    /**
     * Creates a reference to a new buffer containing a copy of the
     * NUL-terminated char array.
     * @param buf A NUL-terminated char array (C string).
     */
    buffer_ref(const char* buf)
        : buffer_ref(reinterpret_cast<const value_type*>(buf), std::strlen(buf)) {
        static_assert(
            sizeof(char) == sizeof(T), "can only use C arr with char or byte buffers"
        );
    }

    /**
     * Copy the reference to the buffer.
     * @param rhs Another buffer
     * @return A reference to this object
     */
    buffer_ref& operator=(const buffer_ref& rhs) = default;
    /**
     * Move a reference to a buffer.
     * @param rhs The other reference to move.
     * @return A reference to this object.
     */
    buffer_ref& operator=(buffer_ref&& rhs) = default;
    /**
     * Copy a string into this object, creating a new buffer.
     * Modifies the reference for this object, pointing it to a
     * newly-created buffer. Other references to the old object remain
     * unchanges, so this follows copy-on-write semantics.
     * @param b A new blob/string to copy.
     * @return A reference to this object.
     */
    buffer_ref& operator=(const blob& b) {
        data_.reset(new blob(b));
        return *this;
    }
    /**
     * Move a string into this object, creating a new buffer.
     * Modifies the reference for this object, pointing it to a
     * newly-created buffer. Other references to the old object remain
     * unchanges, so this follows copy-on-write semantics.
     * @param b A new blob/string to move.
     * @return A reference to this object.
     */
    buffer_ref& operator=(blob&& b) {
        data_.reset(new blob(std::move(b)));
        return *this;
    }
    /**
     * Copy a NUL-terminated C char array into a new buffer
     * @param cstr A NUL-terminated C string.
     * @return A reference to this object
     */
    buffer_ref& operator=(const char* cstr) {
        static_assert(
            sizeof(char) == sizeof(T), "can only use C arr with char or byte buffers"
        );
        data_.reset(new blob(reinterpret_cast<const value_type*>(cstr), strlen(cstr)));
        return *this;
    }
    /**
     * Copy another type of buffer reference to this one.
     * This can copy a buffer of different types, provided that the size of
     * the data elements are the same. This is typically used to convert
     * from char to byte, where the data is the same, but the interpretation
     * is different. Note that this copies the underlying buffer.
     * @param rhs A reference to a different type of buffer.
     * @return A reference to this object.
     */
    template <typename OT>
    buffer_ref& operator=(const buffer_ref<OT>& rhs) {
        static_assert(
            sizeof(OT) == sizeof(T), "Can only assign buffers if values the same size"
        );
        data_.reset(new blob(reinterpret_cast<const value_type*>(rhs.data()), rhs.size()));
        return *this;
    }
    /**
     * Clears the reference to nil.
     */
    void reset() { data_.reset(); }
    /**
     * Determines if the reference is valid.
     * If the reference is invalid then it is not safe to call @em any
     * member functions other than @ref is_null() and @ref empty()
     * @return @em true if referring to a valid buffer, @em false if the
     *  	   reference (pointer) is null.
     */
    explicit operator bool() const { return bool(data_); }
    /**
     * Determines if the reference is invalid.
     * If the reference is invalid then it is not safe to call @em any
     * member functions other than @ref is_null() and @ref empty()
     * @return @em true if the reference is null, @em false if it is
     *  	   referring to a valid buffer,
     */
    bool is_null() const { return !data_; }
    /**
     * Determines if the buffer is empty.
     * @return @em true if the buffer is empty or the reference is null,
     *  	   @em false if the buffer contains data.
     */
    bool empty() const { return !data_ || data_->empty(); }
    /**
     * Gets a const pointer to the data buffer.
     * @return A pointer to the data buffer.
     */
    const value_type* data() const { return data_->data(); }
    /**
     * Gets the size of the data buffer.
     * @return The size of the data buffer.
     */
    size_t size() const { return data_->size(); }
    /**
     * Gets the size of the data buffer.
     * @return The size of the data buffer.
     */
    size_t length() const { return data_->length(); }
    /**
     * Gets the data buffer as a string.
     * @return The data buffer as a string.
     */
    const blob& str() const { return *data_; }
    /**
     * Gets the data buffer as a string.
     * @return The data buffer as a string.
     */
    const blob& to_string() const { return str(); }
    /**
     * Gets the data buffer as NUL-terminated C string.
     * Note that the reference must be set to call this function.
     * @return The data buffer as a string.
     */
    const char* c_str() const { return data_->c_str(); }
    /**
     * Gets a shared pointer to the (const) data buffer.
     * @return A shared pointer to the (const) data buffer.
     */
    const pointer_type& ptr() const { return data_; }
    /**
     * Gets elemental access to the data buffer (read only)
     * @param i The index into the buffer.
     * @return The value at the specified index.
     */
    const value_type& operator[](size_t i) const { return (*data_)[i]; }
};

/**
 * Stream inserter for a buffer reference.
 * This does a binary write of the data in the buffer.
 * @param os The output stream.
 * @param buf The buffer reference to write.
 * @return A reference to the output stream.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const buffer_ref<T>& buf) {
    if (!buf.empty())
        os.write(buf.data(), buf.size());
    return os;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * A reference to a text buffer.
 */
using string_ref = buffer_ref<char>;

/**
 * A reference to a binary buffer.
 * Note that we're using char for the underlying data type to allow
 * efficient moves to and from std::string's. Using a separate type
 * indicates that the data may be arbitrary binary.
 */
using binary_ref = buffer_ref<char>;

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_buffer_ref_h
