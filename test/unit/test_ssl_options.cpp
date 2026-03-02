// test_ssl_options.cpp
//
// Unit tests for the ssl_options class in the Paho MQTT C++ library.
//

/*******************************************************************************
 * Copyright (c) 2016-2020 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *    Guilherme M. Ferreira - add more test cases
 *    Frank Pagliughi - Converted to Catch2
 *******************************************************************************/

#define UNIT_TESTS

#include "catch2_version.h"
#include "mqtt/ssl_options.h"

using namespace std::chrono;
using namespace mqtt;

/////////////////////////////////////////////////////////////////////////////

// C struct signature/eyecatcher
static const char* CSIG = "MQTS";
static const size_t CSIG_LEN = std::strlen(CSIG);

static const bool DFLT_SERVER_CERT = true;

static const std::string EMPTY_STR;
static const std::string TRUST_STORE{"trust store"};
static const std::string KEY_STORE{"key store"};
static const std::string PRIVATE_KEY{"private key"};
static const std::string PRIVATE_KEY_PASSWORD{"private key password"};
static const std::string CA_PATH{"ca path"};
static const std::string ENABLED_CIPHER_SUITES{"cipher suite"};
static const bool SERVER_CERT{false};

static const std::string ALPN0{"mqtt"};
static const std::string ALPN1{"x-iot-mqtt"};

static mqtt::ssl_options orgOpts{TRUST_STORE,          KEY_STORE,     PRIVATE_KEY,
                                 PRIVATE_KEY_PASSWORD, CA_PATH,       ENABLED_CIPHER_SUITES,
                                 SERVER_CERT,          {ALPN0, ALPN1}};

// ----------------------------------------------------------------------
// Test the default constructor
// ----------------------------------------------------------------------

TEST_CASE("ssl_options dflt constructor", "[options]")
{
    mqtt::ssl_options opts;

    REQUIRE(opts.get_trust_store().empty());
    REQUIRE(opts.get_key_store().empty());
    REQUIRE(opts.get_private_key().empty());
    REQUIRE(opts.get_private_key_password().empty());
    REQUIRE(opts.get_ca_path().empty());
    REQUIRE(DFLT_SERVER_CERT == opts.get_enable_server_cert_auth());
    REQUIRE(opts.get_alpn_protos().empty());

    //  Make sure the empty string represents a nullptr for C library
    const auto& c_struct = opts.c_struct();

    REQUIRE(!memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(c_struct.trustStore == nullptr);
    REQUIRE(c_struct.keyStore == nullptr);
    REQUIRE(c_struct.privateKey == nullptr);
    REQUIRE(c_struct.privateKeyPassword == nullptr);
    REQUIRE(c_struct.CApath == nullptr);
    REQUIRE(c_struct.enabledCipherSuites == nullptr);
    // REQUIRE(DFLT_SERVER_CERT == c_struct.enableServerCertAuth != 0);
    REQUIRE(c_struct.protos == nullptr);
    REQUIRE(c_struct.protos_len == 0);
}

// ----------------------------------------------------------------------
// Test the constructor that takes user arguments
// ----------------------------------------------------------------------

TEST_CASE("ssl_options user constructor", "[options]")
{
    mqtt::ssl_options opts{TRUST_STORE,          KEY_STORE,     PRIVATE_KEY,
                           PRIVATE_KEY_PASSWORD, CA_PATH,       ENABLED_CIPHER_SUITES,
                           SERVER_CERT,          {ALPN0, ALPN1}};

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    auto protos = opts.get_alpn_protos();
    REQUIRE(2 == protos.size());
    REQUIRE(ALPN0 == protos[0]);
    REQUIRE(ALPN1 == protos[1]);

    // Check the underlying C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(!memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(!strcmp(c_struct.trustStore, TRUST_STORE.c_str()));
    REQUIRE(!strcmp(c_struct.keyStore, KEY_STORE.c_str()));
    REQUIRE(!strcmp(c_struct.privateKey, PRIVATE_KEY.c_str()));
    REQUIRE(!strcmp(c_struct.privateKeyPassword, PRIVATE_KEY_PASSWORD.c_str()));
    REQUIRE(!strcmp(c_struct.CApath, CA_PATH.c_str()));
    REQUIRE(!strcmp(c_struct.enabledCipherSuites, ENABLED_CIPHER_SUITES.c_str()));
    // REQUIRE(SERVER_CERT == c_struct.enableServerCertAuth != 0);

    auto n0 = ALPN0.length();
    auto n1 = ALPN1.length();

    REQUIRE(c_struct.protos_len == n0 + n1 + 2);
    REQUIRE(c_struct.protos[0] == n0);
    REQUIRE(memcmp(c_struct.protos + 1, ALPN0.data(), n0) == 0);
    REQUIRE(c_struct.protos[n0 + 1] == n1);
    REQUIRE(memcmp(c_struct.protos + n0 + 2, ALPN1.data(), n1) == 0);
}

// ----------------------------------------------------------------------
// Test the copy constructor
// ----------------------------------------------------------------------

TEST_CASE("ssl_options copy constructor", "[options]")
{
    mqtt::ssl_options org{TRUST_STORE,          KEY_STORE,     PRIVATE_KEY,
                          PRIVATE_KEY_PASSWORD, CA_PATH,       ENABLED_CIPHER_SUITES,
                          SERVER_CERT,          {ALPN0, ALPN1}};

    mqtt::ssl_options opts{org};

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    auto protos = opts.get_alpn_protos();
    REQUIRE(2 == protos.size());
    REQUIRE(ALPN0 == protos[0]);
    REQUIRE(ALPN1 == protos[1]);

    // Check the underlying C struct
    const auto& c_struct = opts.c_struct();

    REQUIRE(!memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(!strcmp(c_struct.trustStore, TRUST_STORE.c_str()));
    REQUIRE(!strcmp(c_struct.keyStore, KEY_STORE.c_str()));
    REQUIRE(!strcmp(c_struct.privateKey, PRIVATE_KEY.c_str()));
    REQUIRE(!strcmp(c_struct.privateKeyPassword, PRIVATE_KEY_PASSWORD.c_str()));
    REQUIRE(!strcmp(c_struct.CApath, CA_PATH.c_str()));
    REQUIRE(!strcmp(c_struct.enabledCipherSuites, ENABLED_CIPHER_SUITES.c_str()));
    // REQUIRE(SERVER_CERT == c_struct.enableServerCertAuth != 0);

    // Make sure it's a true copy, not linked to the original
    org.set_trust_store(EMPTY_STR);
    org.set_key_store(EMPTY_STR);
    org.set_private_key(EMPTY_STR);
    org.set_private_key_password(EMPTY_STR);
    org.set_ca_path(EMPTY_STR);
    org.set_enabled_cipher_suites(EMPTY_STR);
    org.set_enable_server_cert_auth(!SERVER_CERT);

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    auto n0 = ALPN0.length();
    auto n1 = ALPN1.length();

    REQUIRE(c_struct.protos_len == n0 + n1 + 2);
    REQUIRE(c_struct.protos[0] == n0);
    REQUIRE(memcmp(c_struct.protos + 1, ALPN0.data(), n0) == 0);
    REQUIRE(c_struct.protos[n0 + 1] == n1);
    REQUIRE(memcmp(c_struct.protos + n0 + 2, ALPN1.data(), n1) == 0);
}

// ----------------------------------------------------------------------
// Test the move constructor
// ----------------------------------------------------------------------

TEST_CASE("ssl_options move constructor", "[options]")
{
    mqtt::ssl_options org{orgOpts};
    mqtt::ssl_options opts(std::move(org));

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    auto protos = opts.get_alpn_protos();
    REQUIRE(2 == protos.size());
    REQUIRE(ALPN0 == protos[0]);
    REQUIRE(ALPN1 == protos[1]);

    // Check that the original was moved
    REQUIRE(org.get_trust_store().empty());
    REQUIRE(org.get_key_store().empty());
    REQUIRE(org.get_private_key().empty());
    REQUIRE(org.get_private_key_password().empty());
    REQUIRE(org.get_ca_path().empty());
    REQUIRE(org.get_enabled_cipher_suites().empty());
    REQUIRE(org.get_alpn_protos().empty());

    // Check the underlying C struct
    const auto& c_struct = opts.c_struct();

    auto n0 = ALPN0.length();
    auto n1 = ALPN1.length();

    REQUIRE(c_struct.protos_len == n0 + n1 + 2);
    REQUIRE(c_struct.protos[0] == n0);
    REQUIRE(memcmp(c_struct.protos + 1, ALPN0.data(), n0) == 0);
    REQUIRE(c_struct.protos[n0 + 1] == n1);
    REQUIRE(memcmp(c_struct.protos + n0 + 2, ALPN1.data(), n1) == 0);
}

// ----------------------------------------------------------------------
// Test the copy assignment operator=(const&)
// ----------------------------------------------------------------------

TEST_CASE("ssl_options copy assignment", "[options]")
{
    mqtt::ssl_options org{orgOpts};
    mqtt::ssl_options opts;

    opts = orgOpts;

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    // Make sure it's a true copy, not linked to the original
    org.set_trust_store(EMPTY_STR);
    org.set_key_store(EMPTY_STR);
    org.set_private_key(EMPTY_STR);
    org.set_private_key_password(EMPTY_STR);
    org.set_ca_path(EMPTY_STR);
    org.set_enabled_cipher_suites(EMPTY_STR);
    org.set_enable_server_cert_auth(!SERVER_CERT);
    org.set_alpn_protos({});

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    // Self assignment should cause no harm
    opts = opts;

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());
}

// ----------------------------------------------------------------------
// Test the move assignment, operator=(&&)
// ----------------------------------------------------------------------

TEST_CASE("ssl_options move assignment", "[options]")
{
    mqtt::ssl_options org{orgOpts};
    mqtt::ssl_options opts;

    opts = std::move(org);

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    auto protos = opts.get_alpn_protos();
    REQUIRE(2 == protos.size());
    REQUIRE(ALPN0 == protos[0]);
    REQUIRE(ALPN1 == protos[1]);

    // Check that the original was moved
    REQUIRE(org.get_trust_store().empty());
    REQUIRE(org.get_key_store().empty());
    REQUIRE(org.get_private_key().empty());
    REQUIRE(org.get_private_key_password().empty());
    REQUIRE(org.get_ca_path().empty());
    REQUIRE(org.get_enabled_cipher_suites().empty());
    REQUIRE(org.get_alpn_protos().empty());

// Self assignment should cause no harm
// (clang++ is smart enough to warn about this)
#if !defined(__clang__)
    opts = std::move(opts);
    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());
#endif
}

// ----------------------------------------------------------------------
// Test set/get of the user and password.
// ----------------------------------------------------------------------

TEST_CASE("ssl_options set user", "[options]")
{
    mqtt::ssl_options opts;

    opts.set_trust_store(TRUST_STORE);
    opts.set_key_store(KEY_STORE);
    opts.set_private_key(PRIVATE_KEY);
    opts.set_private_key_password(PRIVATE_KEY_PASSWORD);
    opts.set_ca_path(CA_PATH);
    opts.set_enabled_cipher_suites(ENABLED_CIPHER_SUITES);
    opts.set_enable_server_cert_auth(SERVER_CERT);
    opts.set_alpn_protos({ALPN0, ALPN1});

    REQUIRE(TRUST_STORE == opts.get_trust_store());
    REQUIRE(KEY_STORE == opts.get_key_store());
    REQUIRE(PRIVATE_KEY == opts.get_private_key());
    REQUIRE(PRIVATE_KEY_PASSWORD == opts.get_private_key_password());
    REQUIRE(CA_PATH == opts.get_ca_path());
    REQUIRE(ENABLED_CIPHER_SUITES == opts.get_enabled_cipher_suites());
    REQUIRE(SERVER_CERT == opts.get_enable_server_cert_auth());

    auto protos = opts.get_alpn_protos();
    REQUIRE(2 == protos.size());
    REQUIRE(ALPN0 == protos[0]);
    REQUIRE(ALPN1 == protos[1]);
}

// ----------------------------------------------------------------------
// Test if empty strings gives nullptr opts
// ----------------------------------------------------------------------

TEST_CASE("ssl_options set empty strings", "[options]")
{
    mqtt::ssl_options opts{orgOpts};

    opts.set_trust_store(EMPTY_STR);
    opts.set_key_store(EMPTY_STR);
    opts.set_private_key(EMPTY_STR);
    opts.set_private_key_password(EMPTY_STR);
    opts.set_ca_path(EMPTY_STR);
    opts.set_enabled_cipher_suites(EMPTY_STR);
    opts.set_alpn_protos({});

    // Make sure the empty string represents a nullptr for C library
    const auto& c_struct = opts.c_struct();

    REQUIRE(!memcmp(&c_struct.struct_id, CSIG, CSIG_LEN));
    REQUIRE(c_struct.trustStore == nullptr);
    REQUIRE(c_struct.keyStore == nullptr);
    REQUIRE(c_struct.privateKey == nullptr);
    REQUIRE(c_struct.privateKeyPassword == nullptr);
    REQUIRE(c_struct.CApath == nullptr);
    REQUIRE(c_struct.enabledCipherSuites == nullptr);
    REQUIRE(c_struct.protos == nullptr);
    REQUIRE(c_struct.protos_len == 0);
}

// ----------------------------------------------------------------------
// Test if empty strings gives nullptr opts
// ----------------------------------------------------------------------

TEST_CASE("ssl_options test error handler", "[options]")
{
    mqtt::ssl_options opts{orgOpts};

    orgOpts.set_error_handler([](const std::string& msg) {
        std::cerr << "SSL Error: " << msg << std::endl;
    });
}
