/////////////////////////////////////////////////////////////////////////////
/// @file ssl_options.h
/// Declaration of MQTT ssl_options class
/// @date Jul 7, 2016
/// @author Frank Pagliughi, Guilherme Ferreira
/////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Copyright (c) 2016-2024 Frank Pagliughi <fpagliughi@mindspring.com>
 * Copyright (c) 2016 Guilherme Ferreira <guilherme.maciel.ferreira@gmail.com>
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
 *    Guilherme Ferreira - initial implementation and documentation
 *    Frank Pagliughi - added copy & move operations
 *    Frank Pagliughi - upgraded compatibility to Paho C 1.3
 *******************************************************************************/

#ifndef __mqtt_ssl_options_h
#define __mqtt_ssl_options_h

#include <functional>
#include <vector>

#include "MQTTAsync.h"
#include "mqtt/message.h"
#include "mqtt/platform.h"
#include "mqtt/topic.h"
#include "mqtt/types.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

/**
 * Holds the set of SSL options for connection.
 */
class ssl_options
{
public:
    /** Smart/shared pointer to an object of this class. */
    using ptr_t = std::shared_ptr<ssl_options>;
    /** Smart/shared pointer to a const object of this class. */
    using const_ptr_t = std::shared_ptr<const ssl_options>;
    /** Unique pointer to an object of this class. */
    using unique_ptr_t = std::unique_ptr<ssl_options>;

    /** Handler type for error message callbacks */
    using error_handler = std::function<void(const string& errMsg)>;
    /**
     * Handler type for TLS-PSK option callback.
     * On success, the callback should return the length of the PSK (in
     * bytes). On failure, it should throw or return zero.
     */
    using psk_handler = std::function<unsigned(
        const string& hint, char* identity, size_t max_identity_len, unsigned char* psk,
        size_t max_psk_len
    )>;

private:
    /** The default C struct */
    static constexpr MQTTAsync_SSLOptions DFLT_C_STRUCT MQTTAsync_SSLOptions_initializer;

    /** The underlying C SSL options */
    MQTTAsync_SSLOptions opts_{DFLT_C_STRUCT};

    /**
     * The file containing the public digital certificates trusted by
     * the client.
     */
    string trustStore_;

    /** The file containing the public certificate chain of the client. */
    string keyStore_;

    /** The file containing the client's private key. */
    string privateKey_;

    /** The password to load the client's privateKey if encrypted. */
    string privateKeyPassword_;

    /** Path to a directory containing CA certificates in PEM format */
    string caPath_;

    /**
     * The list of cipher suites that the client will present to the
     * server during the SSL handshake.
     */
    string enabledCipherSuites_;

    /** Error message callback handler  */
    error_handler errHandler_;

    /** PSK callback handler */
    psk_handler pskHandler_;

    /** ALPN protocol list, in wire format */
    std::basic_string<unsigned char> protos_;

	/** OpenSSL provider name to be used if enabled. */
	string providerName_;


    /** Callbacks from the C library */
    static int on_error(const char* str, size_t len, void* context);
    static unsigned on_psk(
        const char* hint, char* identity, unsigned int max_identity_len, unsigned char* psk,
        unsigned int max_psk_len, void* context
    );

    /** The connect options has special access */
    friend class connect_options;

    /**
     * Gets a pointer to the C-language NUL-terminated strings for the
     * struct.
     * @note In the SSL options, by default, the Paho C treats nullptr char
     * arrays as unset values, so we keep that semantic and only set those
     * char arrays if the string is non-empty.
     * @param str The C++ string object.
     * @return Pointer to a NUL terminated string. This is only valid until
     *  	   the next time the string is updated.
     */
    const char* c_str(const string& str) { return str.empty() ? nullptr : str.c_str(); }
    /**
     * Updates the underlying C structure to match our strings.
     */
    void update_c_struct();

public:
    /**
     * Constructs a new MqttConnectOptions object using the default values.
     */
    ssl_options() {}
    /**
     * Argument constructor.
     * @param trustStore The file containing the public digital certificates
     * trusted by the client.
     * @param keyStore The file containing the public certificate chain of the
     * client.
     * @param privateKey The file containing the client's private key.
     * @param privateKeyPassword The password to load the client's privateKey
     * if encrypted.
     * @param enabledCipherSuites The list of cipher suites that the client
     * will present to the server during the SSL handshake.
     * @param enableServerCertAuth True/False option to enable verification of
     * the server certificate
     * @param alpnProtos The ALPN protocols to try.
     * @param providerName Name of the OpenSSL provider to use.
     */
    ssl_options(
        const string& trustStore, const string& keyStore, const string& privateKey,
        const string& privateKeyPassword, const string& enabledCipherSuites,
        bool enableServerCertAuth,
        const std::vector<string> alpnProtos = std::vector<string>(),
        const string& providerName = ""
    );
    /**
     * Argument constructor.
     * @param trustStore The file containing the public digital certificates
     *  				 trusted by the client.
     * @param keyStore The file containing the public certificate chain of
     *  			   the client.
     * @param privateKey The file containing the client's private key.
     * @param privateKeyPassword The password to load the client's
     *  						 privateKey if encrypted.
     * @param caPath The name of a directory containing CA certificates in
     *  			 PEM format.
     * @param enabledCipherSuites The list of cipher suites that the client
     *  						  will present to the server during the SSL
     *  						  handshake.
     * @param enableServerCertAuth True/False option to enable verification
     *  						   of the server certificate
     * @param alpnProtos The ALPN protocols to try.
     * @param providerName Name of the OpenSSL provider to use.
     */
    ssl_options(
        const string& trustStore, const string& keyStore, const string& privateKey,
        const string& privateKeyPassword, const string& caPath,
        const string& enabledCipherSuites, bool enableServerCertAuth,
        const std::vector<string> alpnProtos = std::vector<string>(),
        const string& providerName = ""
    );
    /**
     * Copy constructor.
     * @param opt The other options to copy.
     */
    ssl_options(const ssl_options& opt);
    /**
     * Move constructor.
     * @param opt The other options to move to this one.
     */
    ssl_options(ssl_options&& opt);
    /**
     * Copy assignment.
     * @param opt The other options to copy.
     * @return A reference to this object.
     */
    ssl_options& operator=(const ssl_options& opt);
    /**
     * Move assignment.
     * @param opt The other options to move to this one.
     * @return A reference to this object.
     */
    ssl_options& operator=(ssl_options&& opt);
    /**
     * Expose the underlying C struct for the unit tests.
     */
#if defined(UNIT_TESTS)
    const MQTTAsync_SSLOptions& c_struct() const { return opts_; }
#endif
    /**
     * Returns the file containing the public digital certificates trusted by
     * the client.
     * @return string
     */
    string get_trust_store() const { return trustStore_; }
    /**
     * Returns the file containing the public certificate chain of the client.
     * @return string
     */
    string get_key_store() const { return keyStore_; }
    /**
     * Gets the name of file containing the client's private key.
     * @return The name of file containing the client's private key.
     */
    string get_private_key() const { return privateKey_; }
    /**
     * Gets the password to load the client's privateKey if encrypted.
     * @return The password to load the client's privateKey if encrypted.
     */
    string get_private_key_password() const { return privateKeyPassword_; }
    /**
     * Returns the list of cipher suites that the client will present to the
     * server during the SSL handshake.
     * @return string
     */
    string get_enabled_cipher_suites() const { return enabledCipherSuites_; }
    /**
     * Returns the true/false to enable verification of the server certificate .
     * @return bool
     */
    bool get_enable_server_cert_auth() const { return to_bool(opts_.enableServerCertAuth); }
    /**
     * Sets the file containing the public digital certificates trusted by
     * the client.
     * @param trustStore The file in PEM format containing the public
     *  				 digital certificates trusted by the client.
     */
    void set_trust_store(const string& trustStore);
    /**
     * Sets the file containing the public certificate chain of the client.
     * @param keyStore The file in PEM format containing the public
     *  			   certificate chain of the client. It may also include
     *				   the client's private key.
     */
    void set_key_store(const string& keyStore);
    /**
     * Sets the file containing the client's private key.
     * @param privateKey If not included in the sslKeyStore, this is the
     *  				 file in PEM format containing the client's private
     *  				 key.
     */
    void set_private_key(const string& privateKey);
    /**
     * Sets the password to load the client's privateKey if encrypted.
     * @param privateKeyPassword The password to load the privateKey if
     *  						 encrypted.
     */
    void set_private_key_password(const string& privateKeyPassword);
    /**
     * Sets the list of cipher suites that the client will present to the server
     * during the SSL handshake.
     * @param enabledCipherSuites The list of cipher suites that the client
     *  						  will present to the server during the SSL
     *  						  handshake. For a  full explanation of the
     *  						  cipher list format, please see the OpenSSL
     *  						  on-line documentation:
     *  						  https://www.openssl.org/docs/manmaster/man1/openssl-ciphers.html
     *  						  If this setting is omitted, its default
     *  						  value will be "ALL", that is, all the
     *  						  cipher suites -excluding those offering no
     *  						  encryption- will be considered. This
     *  						  setting can be used to set an SSL
     *  						  anonymous connection (empty string value,
     *  						  for instance).
     */
    void set_enabled_cipher_suites(const string& enabledCipherSuites);
    /**
     * Enables or disables verification of the server certificate.
     * @param enableServerCertAuth enable/disable verification of the server
     *  						  certificate
     */
    void set_enable_server_cert_auth(bool enableServerCertAuth);
    /**
     * Gets the requested SSL/TLS version.
     * @return The requested SSL/TLS version.
     */
    int get_ssl_version() const { return opts_.sslVersion; }
    /**
     * Set the SSL/TLS version to use.
     *
     * @param ver The desired SSL/TLS version. Specify one of:
     *  	@li MQTT_SSL_VERSION_DEFAULT (0)
     *  	@li MQTT_SSL_VERSION_TLS_1_0 (1)
     *  	@li MQTT_SSL_VERSION_TLS_1_1 (2)
     *  	@li MQTT_SSL_VERSION_TLS_1_2 (3)
     */
    void set_ssl_version(int ver) { opts_.sslVersion = ver; }
    /**
     * Determines whether it will carry out post-connect checks, including
     * that a certificate matches the given host name.
     * @return Whether it will carry out post-connect checks.
     */
    bool get_verify() const { return to_bool(opts_.verify); }
    /**
     * Sets whether it should carry out post-connect checks, including that
     * a certificate matches the given host name.
     * @param v Whether it should carry out post-connect checks.
     */
    void set_verify(bool v) { opts_.verify = to_int(v); }
    /**
     * Gets the path to a directory containing CA certificates in PEM
     * format.
     *
     * @return Path to a directory containing CA certificates in PEM format,
     *  	   if set. If this isn't set, returns an empty string.
     */
    string get_ca_path() const { return caPath_; }
    string ca_path() const { return caPath_; }
    /**
     * Sets the path to a directory containing CA certificates in PEM
     * format.
     *
     * @param path Path to a directory containing CA certificates in PEM
     *  	   format.
     */
    void set_ca_path(const string& path);
    void ca_path(const string& path) { set_ca_path(path); }
    /**
     * Registers the error message callback handler.
     * @param cb The callback to receive error messages.
     */
    void set_error_handler(error_handler cb);
    /**
     * Registers a callback handler to set the TLS-PSK options.
     * See: OpenSSL SSL_CTX_set_psk_client_callback()
     * @param cb The callback.
     */
    void set_psk_handler(psk_handler cb);
    /**
     * Gets the list of supported ALPN protocols.
     * @return A vector containing the supported ALPN protocols.
     */
    std::vector<string> get_alpn_protos() const;
    /**
     * Sets the list of supported ALPN protocols.
     * See:
     * https://www.openssl.org/docs/man1.1.0/man3/SSL_CTX_set_alpn_protos.html
     * @param protos The list of ALPN protocols to be negotiated.
     */
    void set_alpn_protos(const std::vector<string>& protos);
    /*
	 * Returns current provider name which is in use.
	 * @return string containing provider name.
	 */
	string get_provider_name() const { return providerName_; }
 	/**
	 * Sets the provider name to be used.
	 * @param name provider name to use
	 */
	void set_provider_name(const string& name);
};

/**
 * Shared pointer to the ssl options class.
 */
using ssl_options_ptr = ssl_options::ptr_t;
/**
 * Unique pointer to the ssl options class.
 */
using ssl_options_unique_ptr = ssl_options::unique_ptr_t;

/////////////////////////////////////////////////////////////////////////////

/**
 * Class to build the SSL options for connections.
 */
class ssl_options_builder
{
    /** The underlying options */
    ssl_options opts_;

public:
    /** This class */
    using self = ssl_options_builder;
    /**
     * Default constructor.
     */
    ssl_options_builder() {}
    /**
     * Sets the file containing the public digital certificates trusted by
     * the client.
     * @param store The file in PEM format containing the public digital
     *  			certificates trusted by the client.
     */
    auto trust_store(const string& store) -> self& {
        opts_.set_trust_store(store);
        return *this;
    }
    /**
     * Sets the file containing the public certificate chain of the client.
     * @param store The file in PEM format containing the public certificate
     *  			chain of the client. It may also include the client's
     *  			private key.
     */
    auto key_store(const string& store) -> self& {
        opts_.set_key_store(store);
        return *this;
    }
    /**
     * Sets the file containing the client's private key.
     * @param key If not included in the sslKeyStore, this is the file in
     *  		  PEM format containing the client's private key.
     */
    auto private_key(const string& key) -> self& {
        opts_.set_private_key(key);
        return *this;
    }
    /**
     * Sets the password to load the client's privateKey if encrypted.
     * @param passwd The password to load the privateKey if encrypted.
     */
    auto private_keypassword(const string& passwd) -> self& {
        opts_.set_private_key_password(passwd);
        return *this;
    }
    /**
     * Sets the list of cipher suites that the client will present to the server
     * during the SSL handshake.
     * @param suites The list of cipher suites that the client will present to
     *  			 the server during the SSL handshake. For a full
     *  			 explanation of the cipher list format, please see the
     *  			 OpenSSL on-line documentation:
     *  			 http://www.openssl.org/docs/apps/ciphers.html#CIPHER_LIST_FORMAT
     *  			 If this setting is omitted, its default value will be
     *  			 "ALL", that is, all the cipher suites -excluding those
     *  			 offering no encryption- will be considered. This setting
     *  			 can be used to set an SSL anonymous connection (empty
     *  			 string value, for instance).
     */
    auto enabled_cipher_suites(const string& suites) -> self& {
        opts_.set_enabled_cipher_suites(suites);
        return *this;
    }
    /**
     * Enables or disables verification of the server certificate.
     * @param on enable/disable verification of the server certificate
     */
    auto enable_server_cert_auth(bool on) -> self& {
        opts_.set_enable_server_cert_auth(on);
        return *this;
    }
    /**
     * Set the SSL/TLS version to use.
     *
     * @param ver The desired SSL/TLS version. Specify one of:
     *  	@li MQTT_SSL_VERSION_DEFAULT (0)
     *  	@li MQTT_SSL_VERSION_TLS_1_0 (1)
     *  	@li MQTT_SSL_VERSION_TLS_1_1 (2)
     *  	@li MQTT_SSL_VERSION_TLS_1_2 (3)
     */
    auto ssl_version(int ver) -> self& {
        opts_.set_ssl_version(ver);
        return *this;
    }
    /**
     * Sets whether it should carry out post-connect checks, including that
     * a certificate matches the given host name.
     * @param on Whether it should carry out post-connect checks.
     */
    auto verify(bool on = true) -> self& {
        opts_.set_verify(on);
        return *this;
    }
    /**
     * Sets the path to a directory containing CA certificates in PEM format.
     * @param path Path to a directory containing CA certificates in PEM
     *  	   format.
     */
    auto ca_path(const string& path) -> self& {
        opts_.ca_path(path);
        return *this;
    }
    /**
     * Registers an error callback handler.
     * @param cb The callback to receive error messages.
     */
    auto error_handler(ssl_options::error_handler cb) -> self& {
        opts_.set_error_handler(cb);
        return *this;
    }
    /**
     * Registers a callback handler to set the TLS-PSK options.
     * See: OpenSSL SSL_CTX_set_psk_client_callback()
     * @param cb The callback.
     */
    auto psk_handler(ssl_options::psk_handler cb) -> self& {
        opts_.set_psk_handler(cb);
        return *this;
    }
    /**
     * Sets the list of supported ALPN protocols.
     * @param protos The list of ALPN protocols to be negotiated.
     */
    auto alpn_protos(const std::vector<string>& protos) -> self& {
        opts_.set_alpn_protos(protos);
        return *this;
    }
	/**
	 * Sets the provider name
	 * @param name provider name
	 */
	auto provider_name(const string& name) -> self& {
		opts_.set_provider_name(name);
		return *this;
	}
    /**
     * Finish building the options and return them.
     * @return The option struct as built.
     */
    ssl_options finalize() { return opts_; }
};

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt

#endif  // __mqtt_ssl_options_h
