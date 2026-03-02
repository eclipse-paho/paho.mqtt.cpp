// response_options.cpp

/*******************************************************************************
 * Copyright (c) 2019-2025 Frank Pagliughi <fpagliughi@mindspring.com>
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

#include "mqtt/response_options.h"

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

response_options::
    response_options(const token_ptr& tok, int mqttVersion /*=MQTTVERSION_DEFAULT*/)
    : response_options(mqttVersion)
{
    set_token(tok);
}

response_options::response_options(const response_options& other)
    : opts_{other.opts_}, tok_{other.tok_}, props_{other.props_}, subOpts_{other.subOpts_}
{
    update_c_struct();
}

response_options::response_options(response_options&& other)
    : opts_{other.opts_},
      tok_{std::move(other.tok_)},
      props_{std::move(other.props_)},
      subOpts_{std::move(other.subOpts_)}
{
    update_c_struct();
}

response_options& response_options::operator=(const response_options& rhs)
{
    if (&rhs != this) {
        opts_ = rhs.opts_;
        tok_ = rhs.tok_;
        props_ = rhs.props_;
        subOpts_ = rhs.subOpts_;

        update_c_struct();
    }
    return *this;
}

response_options& response_options::operator=(response_options&& rhs)
{
    if (&rhs != this) {
        opts_ = rhs.opts_;
        tok_ = std::move(rhs.tok_);
        props_ = std::move(rhs.props_);
        subOpts_ = std::move(rhs.subOpts_);

        update_c_struct();
    }
    return *this;
}

void response_options::update_c_struct()
{
    opts_.properties = props_.c_struct();

    if (opts_.subscribeOptionsCount != 0)
        opts_.subscribeOptionsList = const_cast<MQTTSubscribe_options*>(subOpts_.data());
}

void response_options::set_mqtt_version(int mqttVersion)
{
    if (mqttVersion < MQTTVERSION_5) {
        opts_.onSuccess = &token::on_success;
        opts_.onFailure = &token::on_failure;
        opts_.onSuccess5 = nullptr;
        opts_.onFailure5 = nullptr;
    }
    else {
        opts_.onSuccess5 = &token::on_success5;
        opts_.onFailure5 = &token::on_failure5;
        opts_.onSuccess = nullptr;
        opts_.onFailure = nullptr;
    }
}

void response_options::set_token(const token_ptr& tok)
{
    tok_ = tok;
    opts_.context = tok.get();
}

void response_options::set_subscribe_options(const subscribe_options& opts)
{
    opts_.subscribeOptions = opts.opts_;
}

std::vector<subscribe_options> response_options::get_subscribe_many_options() const
{
    std::vector<subscribe_options> opts;
    for (const auto& opt : subOpts_) opts.push_back(subscribe_options{opt});
    return opts;
}

void response_options::set_subscribe_many_options(const std::vector<subscribe_options>& opts)
{
    subOpts_.clear();
    for (const auto& opt : opts) subOpts_.push_back(opt.opts_);

    opts_.subscribeOptionsCount = int(opts.size());
    opts_.subscribeOptionsList = const_cast<MQTTSubscribe_options*>(subOpts_.data());
}

/////////////////////////////////////////////////////////////////////////////

delivery_response_options::delivery_response_options(int mqttVersion /*=MQTTVERSION_DEFAULT*/)
    : opts_(MQTTAsync_responseOptions_initializer)
{
    if (mqttVersion < MQTTVERSION_5) {
        opts_.onSuccess = &delivery_token::on_success;
        opts_.onFailure = &delivery_token::on_failure;
    }
    else {
        opts_.onSuccess5 = &delivery_token::on_success5;
        opts_.onFailure5 = &delivery_token::on_failure5;
    }
}

delivery_response_options::delivery_response_options(
    const delivery_token_ptr& tok, int mqttVersion /*=MQTTVERSION_DEFAULT*/
)
    : delivery_response_options(mqttVersion)
{
    set_token(tok);
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
