// test_connect_options.cpp
//
// Unit tests for the connect_options class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2016-2023 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Frank Pagliughi - Converted to Catch2
 *******************************************************************************/

#define UNIT_TESTS

#include "catch2_version.h"
#include "mock_async_client.h"
#include "mqtt/connect_options.h"

using namespace std::chrono;
using namespace mqtt;

// C struct signature/eyecatcher
static const char* CSIG = "MQTC";
static const size_t CSIG_LEN = std::strlen(CSIG);

// These must match the C init struct
static const int DFLT_KEEP_ALIVE = 60;
static const int DFLT_WS_KEEP_ALIVE = 45;
static const int DFLT_CONNECT_TIMEOUT = 30;
static const bool DFLT_AUTO_RECONNECT = false;

static const std::string USER{"wally"};
static const std::string PASSWD{"xyzpdq"};

static const std::string EMPTY_STR;

static const std::vector<string> URIsVec = {
    "mqtt://server1:1883", "mqtt://server2:1883", "mqtts://server3:8883"
};
const const_string_collection_ptr URIs = std::make_shared<const string_collection>(URIsVec);

static constexpr token::Type TOKEN_TYPE = token::Type::CONNECT;

static const std::string HTTP_PROXY{"http://localhost:80"};
static const std::string HTTPS_PROXY{"https://localhost:443"};

// ----------------------------------------------------------------------
// Test the default constructor
// ----------------------------------------------------------------------

TEST_CASE("connect_options default ctor", "[options]")
{
    connect_options opts;

    REQUIRE(EMPTY_STR == opts.get_user_name());
    REQUIRE(EMPTY_STR == opts.get_password_str());
    REQUIRE(DFLT_KEEP_ALIVE == (int)opts.get_keep_alive_interval().count());
    REQUIRE(DFLT_CONNECT_TIMEOUT == (int)opts.get_connect_timeout().count());
    REQUIRE_FALSE(opts.get_servers());
    REQUIRE(DFLT_AUTO_RECONNECT == opts.get_automatic_reconnect());

    REQUIRE(opts.get_http_proxy().empty());
    REQUIRE(opts.get_https_proxy().empty());

    const auto& c_struct = opts.c_struct();
    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));

    REQUIRE(DFLT_KEEP_ALIVE == c_struct.keepAliveInterval);
    REQUIRE(DFLT_CONNECT_TIMEOUT == c_struct.connectTimeout);

    REQUIRE(c_struct.username == nullptr);
    REQUIRE(c_struct.password == nullptr);
    REQUIRE(0 == c_struct.binarypwd.len);
    REQUIRE(c_struct.binarypwd.data == nullptr);

    // No callbacks without a context token
    REQUIRE(c_struct.context == nullptr);
    REQUIRE(c_struct.onSuccess == nullptr);
    REQUIRE(c_struct.onFailure == nullptr);
    REQUIRE(c_struct.onSuccess5 == nullptr);
    REQUIRE(c_struct.onFailure5 == nullptr);

    // No will or SSL, for default
    REQUIRE(c_struct.will == nullptr);
    REQUIRE(c_struct.ssl == nullptr);

    REQUIRE(c_struct.connectProperties == nullptr);
    REQUIRE(c_struct.willProperties == nullptr);

    REQUIRE(0 == c_struct.serverURIcount);
    REQUIRE(nullptr == c_struct.serverURIs);

    REQUIRE(nullptr == c_struct.httpProxy);
    REQUIRE(nullptr == c_struct.httpsProxy);
}

// ----------------------------------------------------------------------
// Test the constructor that takes user/password
// ----------------------------------------------------------------------

TEST_CASE("connect_options user_constructor", "[options]")
{
    connect_options opts{USER, PASSWD};

    REQUIRE(opts.get_http_proxy().empty());
    REQUIRE(opts.get_https_proxy().empty());

    const auto& c_struct = opts.c_struct();
    REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));

    REQUIRE(USER == opts.get_user_name());
    REQUIRE(PASSWD == opts.get_password_str());

    REQUIRE(0 == strcmp(USER.c_str(), c_struct.username));
    REQUIRE(c_struct.password == nullptr);
    REQUIRE(PASSWD.size() == size_t(c_struct.binarypwd.len));
    REQUIRE(0 == memcmp(PASSWD.data(), c_struct.binarypwd.data, PASSWD.size()));

    // No callbacks without a context token
    REQUIRE(c_struct.context == nullptr);
    REQUIRE(c_struct.onSuccess == nullptr);
    REQUIRE(c_struct.onFailure == nullptr);

    REQUIRE(c_struct.httpProxy == nullptr);
    REQUIRE(c_struct.httpsProxy == nullptr);
}

// ----------------------------------------------------------------------
// Test the copy constructor
// ----------------------------------------------------------------------

TEST_CASE("connect_options copy ctor", "[options]")
{
    connect_options orgOpts{USER, PASSWD};

    SECTION("simple options")
    {
        connect_options opts{orgOpts};

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        const auto& c_struct = opts.c_struct();

        REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));

        REQUIRE(0 == strcmp(USER.c_str(), c_struct.username));
        REQUIRE(c_struct.password == nullptr);
        REQUIRE(PASSWD.size() == size_t(c_struct.binarypwd.len));
        REQUIRE(0 == memcmp(PASSWD.data(), c_struct.binarypwd.data, PASSWD.size()));

        // Make sure it's a true copy, not linked to the original
        orgOpts.set_user_name(EMPTY_STR);
        orgOpts.set_password(EMPTY_STR);

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());
    }

    SECTION("proxy options")
    {
        orgOpts.set_http_proxy(HTTP_PROXY);

        connect_options opts{orgOpts};
        REQUIRE(HTTP_PROXY == opts.get_http_proxy());
        REQUIRE(opts.get_https_proxy().empty());
    }

    SECTION("secure proxy options")
    {
        orgOpts.set_https_proxy(HTTPS_PROXY);

        connect_options opts{orgOpts};
        REQUIRE(HTTPS_PROXY == opts.get_https_proxy());
        REQUIRE(opts.get_http_proxy().empty());
    }

    SECTION("properties")
    {
        orgOpts.set_properties({{property::SESSION_EXPIRY_INTERVAL, 0}});

        connect_options opts{orgOpts};

        const auto& copts = opts.c_struct();
        const auto& orgCopts = orgOpts.c_struct();

        // Make sure it's an actual copy
        REQUIRE(copts.connectProperties->array != orgCopts.connectProperties->array);
        orgOpts.get_properties().clear();

        REQUIRE(1 == opts.get_properties().size());
        REQUIRE(opts.get_properties().contains(property::SESSION_EXPIRY_INTERVAL));
        REQUIRE(opts.c_struct().connectProperties == &opts.get_properties().c_struct());
    }
}

// ----------------------------------------------------------------------
// Test the move constructor
// ----------------------------------------------------------------------

TEST_CASE("connect_options move_constructor", "[options]")
{
    connect_options orgOpts{USER, PASSWD};

    SECTION("simple options")
    {
        connect_options opts{std::move(orgOpts)};

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        const auto& copts = opts.c_struct();

        REQUIRE(0 == memcmp(&copts.struct_id, CSIG, CSIG_LEN));

        REQUIRE(0 == strcmp(USER.c_str(), copts.username));
        REQUIRE(copts.password == nullptr);
        REQUIRE(PASSWD.size() == size_t(copts.binarypwd.len));
        REQUIRE(0 == memcmp(PASSWD.data(), copts.binarypwd.data, PASSWD.size()));
        REQUIRE(nullptr == copts.connectProperties);

        // Make sure it's a true copy, not linked to the original
        orgOpts.set_user_name(EMPTY_STR);
        orgOpts.set_password(EMPTY_STR);

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        // Check that the original was moved
        REQUIRE(EMPTY_STR == orgOpts.get_user_name());
        REQUIRE(EMPTY_STR == orgOpts.get_password_str());
    }

    SECTION("properties")
    {
        orgOpts.set_properties({{property::SESSION_EXPIRY_INTERVAL, 42}});

        connect_options opts{std::move(orgOpts)};

        const auto& copts = opts.c_struct();

        // Check that the original was moved
        REQUIRE(orgOpts.get_properties().empty());

        // Check that we got the correct properties
        REQUIRE(1 == opts.get_properties().size());
        REQUIRE(opts.get_properties().contains(property::SESSION_EXPIRY_INTERVAL));
        REQUIRE(
            42 == get<uint32_t>(opts.get_properties(), property::SESSION_EXPIRY_INTERVAL)
        );

        REQUIRE(copts.connectProperties == &opts.get_properties().c_struct());
    }
}

// ----------------------------------------------------------------------
// Test the copy assignment operator=(const&)
// ----------------------------------------------------------------------

TEST_CASE("connect_options copy_assignment", "[options]")
{
    SECTION("v3")
    {
        connect_options orgOpts{USER, PASSWD};
        connect_options opts;

        opts = orgOpts;

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        const auto& copts = opts.c_struct();

        REQUIRE(0 == memcmp(&copts.struct_id, CSIG, CSIG_LEN));

        REQUIRE(0 == strcmp(USER.c_str(), copts.username));
        REQUIRE(copts.password == nullptr);
        REQUIRE(PASSWD.size() == size_t(copts.binarypwd.len));
        REQUIRE(0 == memcmp(PASSWD.data(), copts.binarypwd.data, PASSWD.size()));
        REQUIRE(nullptr == copts.connectProperties);

        // Make sure it's a true copy, not linked to the original
        orgOpts.set_user_name(EMPTY_STR);
        orgOpts.set_password(EMPTY_STR);

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        // Self assignment should cause no harm
        opts = opts;

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());
    }

    SECTION("v5")
    {
        auto orgOpts = connect_options::v5();
        orgOpts.set_properties({{property::SESSION_EXPIRY_INTERVAL, 42}});

        connect_options opts = orgOpts;
        const auto& copts = opts.c_struct();
        const auto& orgCopts = orgOpts.c_struct();

        // Make sure it's an actual copy
        REQUIRE(copts.connectProperties->array != orgCopts.connectProperties->array);
        orgOpts.get_properties().clear();

        // Check that we got the correct properties
        REQUIRE(1 == opts.get_properties().size());
        REQUIRE(opts.get_properties().contains(property::SESSION_EXPIRY_INTERVAL));
        REQUIRE(
            42 == get<uint32_t>(opts.get_properties(), property::SESSION_EXPIRY_INTERVAL)
        );

        REQUIRE(copts.connectProperties == &opts.get_properties().c_struct());
    }
}

// ----------------------------------------------------------------------
// Test the move assignment, operator=(&&)
// ----------------------------------------------------------------------

TEST_CASE("connect_options move_assignment", "[options]")
{
    SECTION("v3")
    {
        connect_options orgOpts{USER, PASSWD};
        connect_options opts{std::move(orgOpts)};

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        const auto& c_struct = opts.c_struct();

        REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));

        REQUIRE(0 == strcmp(USER.c_str(), c_struct.username));
        REQUIRE(c_struct.password == nullptr);
        REQUIRE(PASSWD.size() == size_t(c_struct.binarypwd.len));
        REQUIRE(0 == memcmp(PASSWD.data(), c_struct.binarypwd.data, PASSWD.size()));

        // Make sure it's a true copy, not linked to the original
        orgOpts.set_user_name(EMPTY_STR);
        orgOpts.set_password(EMPTY_STR);

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        // Check that the original was moved
        REQUIRE(EMPTY_STR == orgOpts.get_user_name());
        REQUIRE(EMPTY_STR == orgOpts.get_password_str());

// Self assignment should cause no harm
// (clang++ is smart enough to warn about this)
#if !defined(__clang__)
        opts = std::move(opts);
        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());
#endif
    }

    SECTION("properties")
    {
        auto orgOpts = connect_options::v5();
        orgOpts.set_properties({{property::SESSION_EXPIRY_INTERVAL, 42}});

        connect_options opts = std::move(orgOpts);

        const auto& copts = opts.c_struct();

        // Check that the original was moved
        REQUIRE(orgOpts.get_properties().empty());

        // Check that we got the correct properties
        REQUIRE(1 == opts.get_properties().size());
        REQUIRE(opts.get_properties().contains(property::SESSION_EXPIRY_INTERVAL));
        REQUIRE(
            42 == get<uint32_t>(opts.get_properties(), property::SESSION_EXPIRY_INTERVAL)
        );

        REQUIRE(copts.connectProperties == &opts.get_properties().c_struct());
    }
}

// ----------------------------------------------------------------------
// Test set/get of the user and password.
// ----------------------------------------------------------------------

TEST_CASE("connect_options set_user", "[options]")
{
    connect_options opts;
    const auto& c_struct = opts.c_struct();

    opts.set_user_name(USER);
    opts.set_password(PASSWD);

    REQUIRE(USER == opts.get_user_name());
    REQUIRE(PASSWD == opts.get_password_str());

    REQUIRE(0 == strcmp(USER.c_str(), c_struct.username));
    REQUIRE(c_struct.password == nullptr);
    REQUIRE(PASSWD.size() == size_t(c_struct.binarypwd.len));
    REQUIRE(0 == memcmp(PASSWD.data(), c_struct.binarypwd.data, PASSWD.size()));
}

// ----------------------------------------------------------------------
// Test set/get of a long user name and password.
// ----------------------------------------------------------------------

TEST_CASE("connect_options set_long_user", "[options]")
{
    std::string user;
    std::string passwd;
    for (int i = 0; i < 1053; ++i) {
        auto by = byte(i % 128);
        if (isprint(char(by)))
            user.push_back(char(by));
        passwd.push_back(by);
    }

    connect_options orgOpts;

    orgOpts.set_user_name(user);
    orgOpts.set_password(passwd);

    REQUIRE(user == orgOpts.get_user_name());
    REQUIRE(passwd == orgOpts.get_password_str());

    connect_options opts;
    opts = orgOpts;

    REQUIRE(user == opts.get_user_name());
    REQUIRE(passwd == opts.get_password_str());

    const auto& c_struct = opts.c_struct();

    REQUIRE(0 == strcmp(user.c_str(), c_struct.username));
    REQUIRE(c_struct.password == nullptr);
    REQUIRE(passwd.size() == size_t(c_struct.binarypwd.len));
    REQUIRE(0 == memcmp(passwd.data(), c_struct.binarypwd.data, PASSWD.size()));
}

// ----------------------------------------------------------------------
// Test set/get of will options
// ----------------------------------------------------------------------

TEST_CASE("connect_options set_will", "[options]")
{
    connect_options opts;
    const auto& c_struct = opts.c_struct();

    REQUIRE(nullptr == c_struct.will);
    will_options willOpts;
    opts.set_will(willOpts);
    REQUIRE(nullptr != c_struct.will);
    // REQUIRE(&opts.will_.opts_ == c_struct.will);
}

// ----------------------------------------------------------------------
// Test set/get of ssl options
// ----------------------------------------------------------------------

TEST_CASE("connect_options set_ssl", "[options]")
{
    connect_options opts;
    const auto& c_struct = opts.c_struct();

    REQUIRE(nullptr == c_struct.ssl);
    ssl_options sslOpts;
    opts.set_ssl(sslOpts);
    REQUIRE(nullptr != c_struct.ssl);
    // REQUIRE(&opts.ssl_.opts_ == c_struct.ssl);
}

// ----------------------------------------------------------------------
// Test set/get
// ----------------------------------------------------------------------

TEST_CASE("set_token", "[options]")
{
    connect_options opts;
    const auto& c_struct = opts.c_struct();

    REQUIRE(nullptr == c_struct.context);

    SECTION("set token")
    {
        mock_async_client ac;
        auto tok = token::create(TOKEN_TYPE, ac);
        opts.set_token(tok);
        REQUIRE(tok == opts.get_token());
        REQUIRE(tok.get() == c_struct.context);
    }

    SECTION("keep alive")
    {
        // Set as an int
        const int KEEP_ALIVE_SEC = 30;
        opts.set_keep_alive_interval(KEEP_ALIVE_SEC);

        REQUIRE(KEEP_ALIVE_SEC == (int)opts.get_keep_alive_interval().count());
        REQUIRE(KEEP_ALIVE_SEC == c_struct.keepAliveInterval);

        // Set as an chrono
        opts.set_keep_alive_interval(milliseconds(2 * KEEP_ALIVE_SEC * 1000));

        REQUIRE(2 * KEEP_ALIVE_SEC == (int)opts.get_keep_alive_interval().count());
        REQUIRE(2 * KEEP_ALIVE_SEC == c_struct.keepAliveInterval);
    }

    SECTION("connect timeout")
    {
        // Set as an int
        const int TIMEOUT_SEC = 10;
        opts.set_connect_timeout(TIMEOUT_SEC);

        REQUIRE(TIMEOUT_SEC == (int)opts.get_connect_timeout().count());
        REQUIRE(TIMEOUT_SEC == c_struct.connectTimeout);

        // Set as an chrono
        opts.set_connect_timeout(std::chrono::milliseconds(2 * TIMEOUT_SEC * 1000));

        REQUIRE(2 * TIMEOUT_SEC == (int)opts.get_connect_timeout().count());
        REQUIRE(2 * TIMEOUT_SEC == c_struct.connectTimeout);
    }

    SECTION("set servers")
    {
        opts.set_servers(URIs);

        REQUIRE(URIs.get() == opts.get_servers().get());

        // Check the C struct
        REQUIRE((int)URIsVec.size() == c_struct.serverURIcount);
        REQUIRE(0 == strcmp(URIsVec[0].c_str(), c_struct.serverURIs[0]));
        REQUIRE(0 == strcmp(URIsVec[1].c_str(), c_struct.serverURIs[1]));
        REQUIRE(0 == strcmp(URIsVec[2].c_str(), c_struct.serverURIs[2]));
    }

    SECTION("set auto reconnect")
    {
        // Set as an int
        const int TIMEOUT_SEC = 10;
        opts.set_automatic_reconnect(TIMEOUT_SEC, 2 * TIMEOUT_SEC);

        REQUIRE(opts.get_automatic_reconnect());
        REQUIRE(TIMEOUT_SEC == (int)opts.get_min_retry_interval().count());
        REQUIRE(2 * TIMEOUT_SEC == (int)opts.get_max_retry_interval().count());

        REQUIRE(TIMEOUT_SEC == c_struct.minRetryInterval);
        REQUIRE(2 * TIMEOUT_SEC == c_struct.maxRetryInterval);

        // Set as an chrono
        opts.set_automatic_reconnect(
            milliseconds(2000 * TIMEOUT_SEC), milliseconds(4000 * TIMEOUT_SEC)
        );

        REQUIRE(opts.get_automatic_reconnect());
        REQUIRE(2 * TIMEOUT_SEC == (int)opts.get_min_retry_interval().count());
        REQUIRE(4 * TIMEOUT_SEC == (int)opts.get_max_retry_interval().count());

        REQUIRE(2 * TIMEOUT_SEC == c_struct.minRetryInterval);
        REQUIRE(4 * TIMEOUT_SEC == c_struct.maxRetryInterval);

        // Turn it off
        opts.set_automatic_reconnect(false);
        REQUIRE_FALSE(opts.get_automatic_reconnect());
    }
}

// ----------------------------------------------------------------------
// Test the builder constructors
// ----------------------------------------------------------------------

TEST_CASE("connect_options_builder default generator", "[options]")
{
    connect_options opts;

    // Default is v3.x

    opts = connect_options_builder().finalize();

    REQUIRE(MQTTVERSION_DEFAULT == opts.get_mqtt_version());
    REQUIRE(DFLT_KEEP_ALIVE == (int)opts.get_keep_alive_interval().count());

    // Explicit v3

    opts = connect_options_builder::v3().finalize();

    REQUIRE(MQTTVERSION_DEFAULT == opts.get_mqtt_version());
    REQUIRE(DFLT_KEEP_ALIVE == (int)opts.get_keep_alive_interval().count());

    // v5

    opts = connect_options_builder::v5().finalize();

    REQUIRE(MQTTVERSION_5 == opts.get_mqtt_version());
    REQUIRE(DFLT_KEEP_ALIVE == (int)opts.get_keep_alive_interval().count());

    // WebSocket

    opts = connect_options_builder::ws().finalize();

    REQUIRE(MQTTVERSION_DEFAULT == opts.get_mqtt_version());
    REQUIRE(DFLT_WS_KEEP_ALIVE == (int)opts.get_keep_alive_interval().count());

    // Explicit WebSocket v5

    opts = connect_options_builder::v5_ws().finalize();

    REQUIRE(MQTTVERSION_5 == opts.get_mqtt_version());
    REQUIRE(DFLT_WS_KEEP_ALIVE == (int)opts.get_keep_alive_interval().count());
}

// ----------------------------------------------------------------------
// Test the builder
// ----------------------------------------------------------------------

TEST_CASE("connect_options_builder set", "[options]")
{
    const uint32_t INTERVAL = 80000;

    properties conn_props{property{property::SESSION_EXPIRY_INTERVAL, INTERVAL}};

    auto opts = connect_options_builder().properties(conn_props).finalize();

    auto& props = opts.get_properties();

    REQUIRE(!props.empty());
    REQUIRE(1 == props.size());
    REQUIRE(INTERVAL == get<uint32_t>(props, property::SESSION_EXPIRY_INTERVAL));

    const auto& copts = opts.c_struct();
    REQUIRE(nullptr != copts.connectProperties);
}

// ----------------------------------------------------------------------
// Test the builder's copy assignment operator=(const&)
// ----------------------------------------------------------------------

TEST_CASE("connect_options_builder copy_assignment", "[options]")
{
    SECTION("v3")
    {
        auto orgOptsBldr = connect_options_builder::v3();
        orgOptsBldr.user_name(USER).password(PASSWD);

        connect_options_builder optsBldr;
        optsBldr = orgOptsBldr;

        connect_options opts = optsBldr.finalize();

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        const auto& copts = opts.c_struct();

        REQUIRE(0 == memcmp(&copts.struct_id, CSIG, CSIG_LEN));

        REQUIRE(0 == strcmp(USER.c_str(), copts.username));
        REQUIRE(copts.password == nullptr);
        REQUIRE(PASSWD.size() == size_t(copts.binarypwd.len));
        REQUIRE(0 == memcmp(PASSWD.data(), copts.binarypwd.data, PASSWD.size()));
        REQUIRE(nullptr == copts.connectProperties);
    }

    SECTION("v5")
    {
        auto orgOptsBldr = connect_options_builder::v5();
        orgOptsBldr.properties({{property::SESSION_EXPIRY_INTERVAL, 42}});

        connect_options_builder optsBldr;
        optsBldr = orgOptsBldr;

        connect_options opts = optsBldr.finalize();

        // Check that we got the correct properties
        REQUIRE(1 == opts.get_properties().size());
        REQUIRE(opts.get_properties().contains(property::SESSION_EXPIRY_INTERVAL));
        REQUIRE(
            42 == get<uint32_t>(opts.get_properties(), property::SESSION_EXPIRY_INTERVAL)
        );
    }
}

// ----------------------------------------------------------------------
// Test the builder's move assignment, operator=(&&)
// ----------------------------------------------------------------------

TEST_CASE("connect_options_builder move_assignment", "[options]")
{
    SECTION("v3")
    {
        auto orgOptsBldr = connect_options_builder::v3();
        orgOptsBldr.user_name(USER).password(PASSWD);

        connect_options_builder optsBldr;

        optsBldr = std::move(orgOptsBldr);
        connect_options opts = optsBldr.finalize();

        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());

        const auto& c_struct = opts.c_struct();

        REQUIRE(0 == memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));

        REQUIRE(0 == strcmp(USER.c_str(), c_struct.username));
        REQUIRE(c_struct.password == nullptr);
        REQUIRE(PASSWD.size() == size_t(c_struct.binarypwd.len));
        REQUIRE(0 == memcmp(PASSWD.data(), c_struct.binarypwd.data, PASSWD.size()));

// Self assignment should cause no harm
// (clang++ is smart enough to warn about this)
#if !defined(__clang__)
        optsBldr = std::move(optsBldr);
        opts = optsBldr.finalize();
        REQUIRE(USER == opts.get_user_name());
        REQUIRE(PASSWD == opts.get_password_str());
#endif
    }

    SECTION("properties")
    {
        auto orgOptsBldr = connect_options_builder::v5();
        orgOptsBldr.properties({{property::SESSION_EXPIRY_INTERVAL, 42}});

        connect_options_builder optsBldr;
        optsBldr = std::move(orgOptsBldr);

        connect_options opts = optsBldr.finalize();

        const auto& copts = opts.c_struct();

        // Check that we got the correct properties
        REQUIRE(1 == opts.get_properties().size());
        REQUIRE(opts.get_properties().contains(property::SESSION_EXPIRY_INTERVAL));
        REQUIRE(
            42 == get<uint32_t>(opts.get_properties(), property::SESSION_EXPIRY_INTERVAL)
        );
    }
}
