#ifndef BROKER_LIST_MANAGER_H
#define BROKER_LIST_MANAGER_H

#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include "score_weights.h"

/**
 * ブローカー情報を管理するクラス
 */
class BrokerInfo {
public:
    std::string uri;
    double latency;      // レイテンシ（ミリ秒）
    double bandwidth;    // 帯域（バイト/秒）
    int connection_count; // 接続数
    double score;        // 総合スコア
    bool is_available;   // 利用可能かどうか
    std::chrono::steady_clock::time_point last_check;

    BrokerInfo(const std::string& broker_uri);
    void update_score(const ScoreWeights& weights);
    void reset_metrics();
};

/**
 * ブローカーリストを管理するクラス
 */
class BrokerListManager {
private:
    std::vector<std::shared_ptr<BrokerInfo>> brokers_;
    mutable std::mutex mutex_;
    size_t current_broker_index_;
    std::string category_; // 追加: カテゴリ名
    
    // スコア計算の重み
    const double LATENCY_WEIGHT = 0.4;
    const double BANDWIDTH_WEIGHT = 0.4;
    const double CONNECTION_WEIGHT = 0.2;
    
    // スコア計算の基準値
    const double LATENCY_BASELINE = 100.0;  // 100ms
    const double BANDWIDTH_BASELINE = 1000000.0;  // 1MB/s
    const int CONNECTION_BASELINE = 100;  // 100接続

public:
    BrokerListManager(const std::string& category = "sensor"); // コンストラクタにカテゴリ追加
    
    // ブローカーリストの管理
    void add_broker(const std::string& uri);
    void remove_broker(const std::string& uri);
    void clear_brokers();
    std::vector<std::string> get_broker_uris() const;
    
    // 現在のブローカー管理
    std::shared_ptr<BrokerInfo> get_current_broker() const;
    std::string get_current_broker_uri() const;
    bool set_current_broker(const std::string& uri);
    
    // 最適ブローカーの選択
    std::shared_ptr<BrokerInfo> find_best_broker() const;
    bool should_switch_broker() const;
    
    // メトリクス更新
    void update_broker_metrics(const std::string& uri, 
                              double latency, 
                              double bandwidth, 
                              int connection_count);
    
    // ブローカー状態管理
    void mark_broker_unavailable(const std::string& uri);
    void mark_broker_available(const std::string& uri);
    
    // 統計情報
    size_t get_broker_count() const;
    std::vector<std::shared_ptr<BrokerInfo>> get_all_brokers() const;
    
    // 内部ヘルパー関数（const関数内でmutexを使用するため）
    std::vector<std::string> get_broker_uris_internal() const;
    std::shared_ptr<BrokerInfo> get_current_broker_internal() const;
    std::shared_ptr<BrokerInfo> find_best_broker_internal() const;
    size_t get_broker_count_internal() const;
    std::vector<std::shared_ptr<BrokerInfo>> get_all_brokers_internal() const;
    
    std::string get_category() const { return category_; }
    void set_category(const std::string& category) { category_ = category; }
};

#endif // BROKER_LIST_MANAGER_H 