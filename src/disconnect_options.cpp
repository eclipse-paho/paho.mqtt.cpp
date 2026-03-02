// disconnect_options.cpp

#include "mqtt/disconnect_options.h"

#include <cstring>
#include <utility>

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////
// disconnect_options

disconnect_options::disconnect_options(const disconnect_options& opt)
    : opts_(opt.opts_), tok_(opt.tok_), props_(opt.props_)
{
    update_c_struct();
}

disconnect_options::disconnect_options(disconnect_options&& opt)
    : opts_(opt.opts_), tok_(std::move(opt.tok_)), props_(std::move(opt.props_))
{
    update_c_struct();
}

void disconnect_options::update_c_struct()
{
    opts_.properties = props_.c_struct();
    opts_.context = tok_.get();
}

disconnect_options& disconnect_options::operator=(const disconnect_options& opt)
{
    opts_ = opt.opts_;
    tok_ = opt.tok_;
    props_ = opt.props_;
    update_c_struct();
    return *this;
}

disconnect_options& disconnect_options::operator=(disconnect_options&& opt)
{
    opts_ = opt.opts_;
    tok_ = std::move(opt.tok_);
    props_ = std::move(opt.props_);
    update_c_struct();
    return *this;
}

void disconnect_options::set_token(const token_ptr& tok, int mqttVersion)
{
    tok_ = tok;
    opts_.context = tok_.get();

    opts_.onSuccess = nullptr;
    opts_.onFailure = nullptr;

    opts_.onSuccess5 = nullptr;
    opts_.onFailure5 = nullptr;

    if (tok) {
        if (mqttVersion >= MQTTVERSION_5) {
            opts_.onSuccess5 = &token::on_success5;
            opts_.onFailure5 = &token::on_failure5;
        }
        else {
            opts_.onSuccess = &token::on_success;
            opts_.onFailure = &token::on_failure;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
}  // namespace mqtt
