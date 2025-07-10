#include "broker_list_manager.h"
#include <algorithm>
#include <iostream>
#include <chrono>

BrokerInfo::BrokerInfo(const std::string& broker_uri) 
    : uri(broker_uri), latency(0.0), bandwidth(0.0), 
      connection_count(0), score(0.0), is_available(true),
      last_check(std::chrono::steady_clock::now()) {
}

void BrokerInfo::update_score(const ScoreWeights& weights) {
    // スコア計算の基準値
    const double LATENCY_BASELINE = 100.0;  // 100ms
    const double BANDWIDTH_BASELINE = 1000000.0;  // 1MB/s
    const int CONNECTION_BASELINE = 100;  // 100接続
    
    // 各指標の正規化スコアを計算
    double latency_score = 0.0;
    if (latency > 0) {
        latency_score = std::max(0.0, 1.0 - (latency / LATENCY_BASELINE));
    }
    
    double bandwidth_score = 0.0;
    if (bandwidth > 0) {
        bandwidth_score = std::min(1.0, bandwidth / BANDWIDTH_BASELINE);
    }
    
    double connection_score = 0.0;
    if (connection_count > 0) {
        connection_score = std::max(0.0, 1.0 - (connection_count / (double)CONNECTION_BASELINE));
    }
    
    // 重み付き総合スコアを計算
    score = (latency_score * weights.latency) + 
            (bandwidth_score * weights.bandwidth) + 
            (connection_score * weights.connection);
    
    // 利用不可の場合はスコアを0にする
    if (!is_available) {
        score = 0.0;
    }
}

void BrokerInfo::reset_metrics() {
    latency = 0.0;
    bandwidth = 0.0;
    connection_count = 0;
    score = 0.0;
    last_check = std::chrono::steady_clock::now();
}

BrokerListManager::BrokerListManager(const std::string& category) : current_broker_index_(0), category_(category) {
}

void BrokerListManager::add_broker(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 既存のブローカーかチェック
    for (const auto& broker : brokers_) {
        if (broker->uri == uri) {
            return;  // 既に存在する
        }
    }
    
    // 新しいブローカーを追加
    brokers_.push_back(std::make_shared<BrokerInfo>(uri));
    
    // 最初のブローカーの場合、現在のブローカーに設定
    if (brokers_.size() == 1) {
        current_broker_index_ = 0;
    }
}

void BrokerListManager::remove_broker(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(brokers_.begin(), brokers_.end(),
                          [&uri](const std::shared_ptr<BrokerInfo>& broker) {
                              return broker->uri == uri;
                          });
    
    if (it != brokers_.end()) {
        size_t removed_index = std::distance(brokers_.begin(), it);
        brokers_.erase(it);
        
        // 現在のブローカーが削除された場合、インデックスを調整
        if (removed_index == current_broker_index_) {
            if (brokers_.empty()) {
                current_broker_index_ = 0;
            } else if (current_broker_index_ >= brokers_.size()) {
                current_broker_index_ = brokers_.size() - 1;
            }
        } else if (removed_index < current_broker_index_) {
            current_broker_index_--;
        }
    }
}

void BrokerListManager::clear_brokers() {
    std::lock_guard<std::mutex> lock(mutex_);
    brokers_.clear();
    current_broker_index_ = 0;
}

std::vector<std::string> BrokerListManager::get_broker_uris() const {
    return get_broker_uris_internal();
}

std::vector<std::string> BrokerListManager::get_broker_uris_internal() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> uris;
    uris.reserve(brokers_.size());
    
    for (const auto& broker : brokers_) {
        uris.push_back(broker->uri);
    }
    
    return uris;
}

std::shared_ptr<BrokerInfo> BrokerListManager::get_current_broker() const {
    return get_current_broker_internal();
}

std::shared_ptr<BrokerInfo> BrokerListManager::get_current_broker_internal() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (brokers_.empty() || current_broker_index_ >= brokers_.size()) {
        return nullptr;
    }
    
    return brokers_[current_broker_index_];
}

std::string BrokerListManager::get_current_broker_uri() const {
    auto current_broker = get_current_broker();
    return current_broker ? current_broker->uri : "";
}

bool BrokerListManager::set_current_broker(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (size_t i = 0; i < brokers_.size(); ++i) {
        if (brokers_[i]->uri == uri) {
            current_broker_index_ = i;
            return true;
        }
    }
    
    return false;  // 指定されたURIのブローカーが見つからない
}

std::shared_ptr<BrokerInfo> BrokerListManager::find_best_broker() const {
    return find_best_broker_internal();
}

std::shared_ptr<BrokerInfo> BrokerListManager::find_best_broker_internal() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (brokers_.empty()) {
        return nullptr;
    }
    
    // 利用可能なブローカーの中で最高スコアのものを探す
    std::shared_ptr<BrokerInfo> best_broker = nullptr;
    double best_score = -1.0;
    
    for (const auto& broker : brokers_) {
        if (broker->is_available && broker->score > best_score) {
            best_score = broker->score;
            best_broker = broker;
        }
    }
    
    return best_broker;
}

bool BrokerListManager::should_switch_broker() const {
    auto current_broker = get_current_broker();
    auto best_broker = find_best_broker();
    
    if (!current_broker || !best_broker) {
        return false;
    }
    
    // 現在のブローカーが最適でない場合
    if (current_broker->uri != best_broker->uri) {
        // スコア差が十分大きい場合のみ切り替え（ヒステリシス）
        const double SWITCH_THRESHOLD = 0.1;  // 10%の差
        return (best_broker->score - current_broker->score) > SWITCH_THRESHOLD;
    }
    
    return false;
}

void BrokerListManager::update_broker_metrics(const std::string& uri, 
                                             double latency, 
                                             double bandwidth, 
                                             int connection_count) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // カテゴリに応じた重みを取得
    ScoreWeights weights = CATEGORY_WEIGHTS.count(category_) ? CATEGORY_WEIGHTS.at(category_) : CATEGORY_WEIGHTS.at("sensor");
    
    for (auto& broker : brokers_) {
        if (broker->uri == uri) {
            broker->latency = latency;
            broker->bandwidth = bandwidth;
            broker->connection_count = connection_count;
            broker->last_check = std::chrono::steady_clock::now();
            broker->update_score(weights);
            break;
        }
    }
}

void BrokerListManager::mark_broker_unavailable(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& broker : brokers_) {
        if (broker->uri == uri) {
            broker->is_available = false;
            broker->score = 0.0;
            break;
        }
    }
}

void BrokerListManager::mark_broker_available(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    ScoreWeights weights = CATEGORY_WEIGHTS.count(category_) ? CATEGORY_WEIGHTS.at(category_) : CATEGORY_WEIGHTS.at("sensor");
    for (auto& broker : brokers_) {
        if (broker->uri == uri) {
            broker->is_available = true;
            broker->update_score(weights);
            std::cout << "ブローカーを利用可能に設定しました: " << uri << " (スコア: " << broker->score << ")" << std::endl;
            break;
        }
    }
}

bool BrokerListManager::is_broker_available(const std::string& uri) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& broker : brokers_) {
        if (broker->uri == uri) {
            return broker->is_available;
        }
    }
    return false;
}

size_t BrokerListManager::get_broker_count() const {
    return get_broker_count_internal();
}

size_t BrokerListManager::get_broker_count_internal() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return brokers_.size();
}

std::vector<std::shared_ptr<BrokerInfo>> BrokerListManager::get_all_brokers() const {
    return get_all_brokers_internal();
}

std::vector<std::shared_ptr<BrokerInfo>> BrokerListManager::get_all_brokers_internal() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return brokers_;
}
