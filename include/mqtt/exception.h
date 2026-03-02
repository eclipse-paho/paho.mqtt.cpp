/////////////////////////////////////////////////////////////////////////////
/// @file exception.h
/// Declaration of MQTT exception class
/// @date May 1, 2013
/// @author Frank Pagliughi
/////////////////////////////////////////////////////////////////////////////

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

#ifndef __mqtt_exception_h
#define __mqtt_exception_h

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "MQTTAsync.h"
#include "mqtt/types.h"

namespace mqtt {

/** Bring std::bad_cast into the mqtt namespace */
using bad_cast = std::bad_cast;

/////////////////////////////////////////////////////////////////////////////

/**
 * Base mqtt::exception.
 * This wraps the error codes which originate from the underlying C library.
 */
class exception : public std::runtime_error
{
protected:
    /** The error return code from the C library */
    int rc_;
    /** The reason code from the server */
    ReasonCode reasonCode_;
    /** The error message from the C library */
    string msg_;

    /** See if the return code is actually a reason code error value */
    static ReasonCode reason_code(int rc, ReasonCode reasonCode) {
        if (reasonCode == ReasonCode::SUCCESS && rc >= ReasonCode::UNSPECIFIED_ERROR)
            reasonCode = ReasonCode(rc);
        return reasonCode;
    }

public:
    /**
     * Creates an MQTT exception.
     * @param rc The error return code from the C library.
     */
    explicit exception(int rc) : exception(rc, error_str(rc)) {}
    /**
     * Creates an MQTT exception.
     * @param rc The error return code from the C library.
     * @param reasonCode The reason code from the server response.
     */
    explicit exception(int rc, ReasonCode reasonCode)
        : exception(rc, reasonCode, error_str(rc)) {}
    /**
     * Creates an MQTT exception.
     * @param rc The error return code from the C library.
     * @param msg The text message for the error.
     */
    exception(int rc, const string& msg) : exception(rc, ReasonCode::SUCCESS, msg) {}
    /**
     * Creates an MQTT exception.
     * @param rc The error return code from the C library.
     * @param reasonCode The reason code from the server
     * @param msg The text message for the error.
     */
    exception(int rc, ReasonCode reasonCode, const string& msg)
        : std::runtime_error(printable_error(rc, reasonCode, msg)),
          rc_{rc},
          reasonCode_{reason_code(rc, reasonCode)},
          msg_{msg} {}
    /**
     * Gets an error message from an error code.
     * @param rc The error code from the C lib
     * @return A string explanation of the error
     */
    static string error_str(int rc) {
        const char* msg = ::MQTTAsync_strerror(rc);
        return msg ? string(msg) : string();
    }
    /**
     * Gets a string describing the MQTT v5 reason code.
     * @param reasonCode The MQTT v5 reason code.
     * @return A string describing the reason code.
     */
    static string reason_code_str(int reasonCode) {
        auto msg = ::MQTTReasonCode_toString(MQTTReasonCodes(reasonCode));
        return (msg) ? string{msg} : string{};
    }
    /**
     * Gets a detailed error message for an error code.
     * @param rc The error code from the C lib
     * @param reasonCode The MQTT v5 reason code
     * @param msg An optional additional message. If none is provided, the
     *  		  error_str message is used.
     * @return A string error message that includes the error code and an
     *  	   explanation message.
     */
    static string printable_error(
        int rc, ReasonCode reasonCode = ReasonCode::SUCCESS, const string& msg = string()
    ) {
        reasonCode = reason_code(rc, reasonCode);

        string s = "MQTT error [" + std::to_string(rc) + "]";
        if (!msg.empty())
            s += string(": ") + msg;
        if (reasonCode != ReasonCode::SUCCESS)
            s += string(". ") + reason_code_str(reasonCode);
        return s;
    }
    /**
     * Returns the return code for this exception.
     */
    int get_return_code() const { return rc_; }
    /**
     * Gets a string of the error code.
     * @return A string of the error code.
     */
    string get_error_str() const { return error_str(rc_); }
    /**
     * Returns the reason code for this exception.
     * For MQTT v3 connections, this is actually the return code.
     */
    int get_reason_code() const { return reasonCode_; }
    /**
     * Gets a string for the reason code.
     * @return A string for the reason code.
     */
    string get_reason_code_str() const { return reason_code_str(reasonCode_); }
    /**
     * Returns the error message for this exception.
     */
    string get_message() const { return msg_; }
    /**
     * Gets a string representation of this exception.
     * @return A string representation of this exception.
     */
    string to_string() const { return string(what()); }
};

/**
 * Stream inserter writes a fairly verbose message
 * @param os The stream.
 * @param exc The exception to write.
 * @return A reference to the stream.
 */
inline std::ostream& operator<<(std::ostream& os, const exception& exc) {
    os << exc.what();
    return os;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Exception thrown when an expected server response is missing.
 */
class missing_response : public exception
{
public:
    /**
     * Create a missing response error.
     * @param rsp A string for the type of response expected.
     */
    missing_response(const string& rsp)
        : exception(MQTTASYNC_FAILURE, "Missing " + rsp + " response") {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A timeout exception, particularly from the synchronous client.
 */
class timeout_error : public exception
{
public:
    /**
     * Create a timeout error.
     */
    timeout_error() : exception(MQTTASYNC_FAILURE, "Timeout") {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * This exception is thrown by the implementor of the persistence interface
 * if there is a problem reading or writing persistent data.
 */
class persistence_exception : public exception
{
public:
    /**
     * Creates an MQTT persistence exception.
     */
    persistence_exception() : exception(MQTTCLIENT_PERSISTENCE_ERROR) {}
    /**
     * Creates an MQTT persistence exception.
     * @param code The error code from the C library.
     */
    explicit persistence_exception(int code) : exception(code) {}
    /**
     * Creates an MQTT persistence exception.
     * @param msg The text message for the error.
     */
    explicit persistence_exception(const string& msg)
        : exception(MQTTCLIENT_PERSISTENCE_ERROR, msg) {}
    /**
     * Creates an MQTT persistence exception.
     * @param code The error code
     * @param msg The text message for the error.
     */
    persistence_exception(int code, const string& msg) : exception(code, msg) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Thrown when a client is not authorized to perform an operation, or if
 * there is a problem with the security configuration.
 */
class security_exception : public exception
{
public:
    /**
     * Creates an MQTT security exception
     * @param code The error code.
     */
    explicit security_exception(int code) : exception(code) {}
    /**
     * Creates an MQTT security exception
     * @param code The error code.
     * @param msg The text message for the error.
     */
    security_exception(int code, const string& msg) : exception(code, msg) {}
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_exception_h
