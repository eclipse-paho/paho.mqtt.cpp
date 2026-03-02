/////////////////////////////////////////////////////////////////////////////
/// @file types.h
/// Basic types and type conversions for the Paho MQTT C++ library.
/// @date May 17, 2015 @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2015-2017 Frank Pagliughi <fpagliughi@mindspring.com>
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

#ifndef __mqtt_types_h
#define __mqtt_types_h

#include <chrono>
#include <memory>
#include <string>
#include <vector>

// Pull in reason codes here for backward compatability with old version
#include "mqtt/reason_code.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////
// Basic data types

/** A 'byte' is an 8-bit, unsigned int */
using byte = uint8_t;

/** An mqtt string is just a std::string */
using string = std::string;
/** A binary blob of data is, umm, just a string too! */
using binary = std::string;

/** Smart/shared pointer to a const string */
using string_ptr = std::shared_ptr<const string>;
/** Smart/shared pointer to a const binary blob */
using binary_ptr = std::shared_ptr<const binary>;

/////////////////////////////////////////////////////////////////////////////
// Time functions

/**
 * Convert a chrono duration to seconds.
 * This casts away precision to get integer seconds.
 * @param dur A chrono duration type
 * @return The duration as a chrono seconds value
 */
template <class Rep, class Period>
std::chrono::seconds to_seconds(const std::chrono::duration<Rep, Period>& dur) {
    return std::chrono::duration_cast<std::chrono::seconds>(dur);
}

/**
 * Convert a chrono duration to a number of seconds.
 * This casts away precision to get integer seconds.
 * @param dur A chrono duration type
 * @return The duration as a number of seconds
 */
template <class Rep, class Period>
long to_seconds_count(const std::chrono::duration<Rep, Period>& dur) {
    return (long)to_seconds(dur).count();
}

/**
 * Convert a chrono duration to milliseconds.
 * This casts away precision to get integer milliseconds.
 * @param dur A chrono duration type
 * @return The duration as a chrono milliseconds value
 */
template <class Rep, class Period>
std::chrono::milliseconds to_milliseconds(const std::chrono::duration<Rep, Period>& dur) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(dur);
}

/**
 * Convert a chrono duration to a number of milliseconds.
 * This casts away precision to get integer milliseconds.
 * @param dur A chrono duration type
 * @return The duration as a number of milliseconds
 */
template <class Rep, class Period>
long to_milliseconds_count(const std::chrono::duration<Rep, Period>& dur) {
    return (long)to_milliseconds(dur).count();
}

/////////////////////////////////////////////////////////////////////////////
// Misc

/**
 * Converts an into to a bool.
 * @param n An integer.
 * @return @em true if n not equal to zero, @em false otherwise
 */
inline bool to_bool(int n) { return n != 0; }
/**
 * Converts the boolean into a C integer true/false value.
 * @param b A boolean
 * @return Zero if b is false, non-zero if b is true.
 */
inline int to_int(bool b) { return b ? (!0) : 0; }

/**
 * Gets a valid string for the char pointer, returning an empty string if
 * the pointer is NULL.
 * @param cstr A C-string pointer
 * @return A string copy of the C array. If `cstr` is NULL, this returns an
 *  	   empty string.
 */
inline string to_string(const char* cstr) { return cstr ? string(cstr) : string(); }

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_types_h
