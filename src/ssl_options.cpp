/*******************************************************************************
 * Copyright (c) 2016 Guilherme Ferreira <guilherme.maciel.ferreira@gmail.com>
 * Copyright (c) 2016-2025 Frank Pagliughi <fpagliughi@mindspring.com>
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
 *******************************************************************************/

#include "mqtt/ssl_options.h"

#include <cstring>
#include <utility>

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

ssl_options::ssl_options(
    const string& trustStore, const string& keyStore, const string& privateKey,
    const string& privateKeyPassword, const string& enabledCipherSuites,
    bool enableServerCertAuth, const std::vector<string> alpnProtos, /*=std::vector<string>()*/
    const string& providerName
)
    : trustStore_(trustStore),
      keyStore_(keyStore),
      privateKey_(privateKey),
      privateKeyPassword_(privateKeyPassword),
      enabledCipherSuites_(enabledCipherSuites),
      providerName_(providerName)
{
    set_alpn_protos(alpnProtos);
    update_c_struct();
    opts_.enableServerCertAuth = enableServerCertAuth;
}

ssl_options::ssl_options(
    const string& trustStore, const string& keyStore, const string& privateKey,
    const string& privateKeyPassword, const string& caPath, const string& enabledCipherSuites,
    bool enableServerCertAuth, const std::vector<string> alpnProtos, /*=std::vector<string>()*/
    const string& providerName
)
    : trustStore_(trustStore),
      keyStore_(keyStore),
      privateKey_(privateKey),
      privateKeyPassword_(privateKeyPassword),
      caPath_(caPath),
      enabledCipherSuites_(enabledCipherSuites),
      providerName_(providerName)
{
    set_alpn_protos(alpnProtos);
    update_c_struct();
    opts_.enableServerCertAuth = enableServerCertAuth;
}

ssl_options::ssl_options(const ssl_options& other)
    : opts_(other.opts_),
      trustStore_(other.trustStore_),
      keyStore_(other.keyStore_),
      privateKey_(other.privateKey_),
      privateKeyPassword_(other.privateKeyPassword_),
      caPath_(other.caPath_),
      enabledCipherSuites_(other.enabledCipherSuites_),
      errHandler_(other.errHandler_),
      pskHandler_(other.pskHandler_),
      protos_(other.protos_),
      providerName_(other.providerName_)
{
    update_c_struct();
}

ssl_options::ssl_options(ssl_options&& other)
    : opts_(other.opts_),
      trustStore_(std::move(other.trustStore_)),
      keyStore_(std::move(other.keyStore_)),
      privateKey_(std::move(other.privateKey_)),
      privateKeyPassword_(std::move(other.privateKeyPassword_)),
      caPath_(std::move(other.caPath_)),
      enabledCipherSuites_(std::move(other.enabledCipherSuites_)),
      errHandler_(std::move(other.errHandler_)),
      pskHandler_(std::move(other.pskHandler_)),
      protos_(std::move(other.protos_)),
      providerName_(std::move(other.providerName_))
{
    update_c_struct();
}

void ssl_options::update_c_struct()
{
    opts_.trustStore = c_str(trustStore_);
    opts_.keyStore = c_str(keyStore_);
    opts_.privateKey = c_str(privateKey_);
    opts_.privateKeyPassword = c_str(privateKeyPassword_);
    opts_.CApath = c_str(caPath_);
    opts_.enabledCipherSuites = c_str(enabledCipherSuites_);

    if (errHandler_) {
        opts_.ssl_error_cb = &ssl_options::on_error;
        opts_.ssl_error_context = this;
    }
    else {
        opts_.ssl_error_cb = nullptr;
        opts_.ssl_error_context = nullptr;
    }

    if (pskHandler_) {
        opts_.ssl_psk_cb = &ssl_options::on_psk;
        opts_.ssl_psk_context = this;
    }
    else {
        opts_.ssl_psk_cb = nullptr;
        opts_.ssl_psk_context = nullptr;
    }

    if (!protos_.empty()) {
        opts_.protos = protos_.data();
        opts_.protos_len = unsigned(protos_.size());
    }
    else {
        opts_.protos = nullptr;
        opts_.protos_len = 0;
    }
	if (!providerName_.empty()) {
		opts_.providerName = c_str(providerName_);
	} else {
		opts_.providerName = nullptr;
	}
}

// --------------------------------------------------------------------------

int ssl_options::on_error(const char* str, size_t len, void* context)
{
    try {
        if (context && str && len > 0) {
            string errMsg{str, len};

            ssl_options* opts = static_cast<ssl_options*>(context);
            auto& errHandler = opts->errHandler_;

            if (errHandler)
                errHandler(errMsg);

            return MQTTASYNC_SUCCESS;
        }
    }
    catch (...) {
    }

    return MQTTASYNC_FAILURE;
}

unsigned ssl_options::on_psk(
    const char* hint, char* identity, unsigned max_identity_len, unsigned char* psk,
    unsigned max_psk_len, void* context
)
{
    unsigned ret = 0;

    try {
        if (context) {
            auto hintStr = (hint) ? string(hint) : string();

            ssl_options* opts = static_cast<ssl_options*>(context);
            auto& pskHandler = opts->pskHandler_;

            if (pskHandler) {
                ret = pskHandler(
                    hintStr, identity, size_t(max_identity_len), psk, size_t(max_psk_len)
                );
            }
        }
    }
    catch (...) {
    }

    return ret;
}

// --------------------------------------------------------------------------

ssl_options& ssl_options::operator=(const ssl_options& rhs)
{
    if (&rhs == this)
        return *this;

    opts_ = rhs.opts_;

    trustStore_ = rhs.trustStore_;
    keyStore_ = rhs.keyStore_;
    privateKey_ = rhs.privateKey_;
    privateKeyPassword_ = rhs.privateKeyPassword_;
    caPath_ = rhs.caPath_;
    enabledCipherSuites_ = rhs.enabledCipherSuites_;

    errHandler_ = rhs.errHandler_;
    pskHandler_ = rhs.pskHandler_;

    protos_ = rhs.protos_;
    providerName_ = rhs.providerName_;

    update_c_struct();
    return *this;
}

ssl_options& ssl_options::operator=(ssl_options&& rhs)
{
    if (&rhs == this)
        return *this;

    opts_ = rhs.opts_;

    trustStore_ = std::move(rhs.trustStore_);
    keyStore_ = std::move(rhs.keyStore_);
    privateKey_ = std::move(rhs.privateKey_);
    privateKeyPassword_ = std::move(rhs.privateKeyPassword_);
    caPath_ = std::move(rhs.caPath_);
    enabledCipherSuites_ = std::move(rhs.enabledCipherSuites_);

    errHandler_ = std::move(rhs.errHandler_);
    pskHandler_ = std::move(rhs.pskHandler_);

    protos_ = std::move(rhs.protos_);
    providerName_ = std::move(rhs.providerName_);

    update_c_struct();
    return *this;
}

// --------------------------------------------------------------------------

void ssl_options::set_trust_store(const string& trustStore)
{
    trustStore_ = trustStore;
    opts_.trustStore = c_str(trustStore_);
}

void ssl_options::set_key_store(const string& keyStore)
{
    keyStore_ = keyStore;
    opts_.keyStore = c_str(keyStore_);
}

void ssl_options::set_private_key(const string& privateKey)
{
    privateKey_ = privateKey;
    opts_.privateKey = c_str(privateKey_);
}

void ssl_options::set_private_key_password(const string& privateKeyPassword)
{
    privateKeyPassword_ = privateKeyPassword;
    opts_.privateKeyPassword = c_str(privateKeyPassword_);
}

void ssl_options::set_enabled_cipher_suites(const string& enabledCipherSuites)
{
    enabledCipherSuites_ = enabledCipherSuites;
    opts_.enabledCipherSuites = c_str(enabledCipherSuites_);
}

void ssl_options::set_enable_server_cert_auth(bool enableServerCertAuth)
{
    opts_.enableServerCertAuth = to_int(enableServerCertAuth);
}

void ssl_options::set_ca_path(const string& path)
{
    caPath_ = path;
    opts_.CApath = c_str(caPath_);
}

void ssl_options::set_error_handler(error_handler cb)
{
    errHandler_ = cb;

    if (errHandler_) {
        opts_.ssl_error_cb = &ssl_options::on_error;
        opts_.ssl_error_context = this;
    }
    else {
        opts_.ssl_error_cb = nullptr;
        opts_.ssl_error_context = nullptr;
    }
}

void ssl_options::set_psk_handler(psk_handler cb)
{
    pskHandler_ = cb;

    if (pskHandler_) {
        opts_.ssl_psk_cb = &ssl_options::on_psk;
        opts_.ssl_psk_context = this;
    }
    else {
        opts_.ssl_psk_cb = nullptr;
        opts_.ssl_psk_context = nullptr;
    }
}

// Gets the list of ALPN protocols.
// To do so, it must recover the strings from the wire format.
std::vector<string> ssl_options::get_alpn_protos() const
{
    std::vector<string> protos;
    size_t i = 0, n = protos_.size();

    while (i < n) {
        size_t sn = protos_[i++];
        if (i + sn > n)
            break;

        string s;
        s.reserve(sn);

        sn += i;
        while (i < sn) s.push_back(char(protos_[i++]));
        protos.push_back(std::move(s));
    }
    return protos;
}

// Converts the vector of names into the binary string in wire format.
// This is a single string uf unsigned characters with each protocol
// prepended by a byte of its length.
void ssl_options::set_alpn_protos(const std::vector<string>& protos)
{
    using uchar = unsigned char;

    if (!protos.empty()) {
        std::vector<uchar> protoBin;
        for (const auto& proto : protos) {
            protoBin.push_back(uchar(proto.length()));
            for (const char c : proto) protoBin.push_back(uchar(c));
        }
        protos_ = std::move(protoBin);

        opts_.protos = protos_.data();
        opts_.protos_len = static_cast<unsigned>(protos_.size());
    }
    else {
        protos_.clear();
        opts_.protos = nullptr;
        opts_.protos_len = 0;
    }
}

void ssl_options::set_provider_name(const string& name)
{
	providerName_ = name;
	opts_.providerName = c_str(providerName_);
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
