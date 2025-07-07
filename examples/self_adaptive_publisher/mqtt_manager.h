#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <functional>
#include "mqtt/async_client.h"
#include "broker_list_manager.h"
#include "broker_monitor_thread.h"

/**
 * 自己適応型MQTTマネージャークラス
 * 複数ブローカー間での自律的な接続切り替えを管理する
 */
class SelfAdaptiveMqttManager : public virtual mqtt::callback {
private:
    std::unique_ptr<mqtt::async_client> client_;
    std::shared_ptr<BrokerListManager> broker_manager_;
    std::unique_ptr<BrokerMonitorThread> monitor_thread_;
    
    // 接続設定
    std::string client_id_;
    std::string persistence_dir_;
    mqtt::connect_options connect_options_;
    
    // 状態管理
    std::atomic<bool> is_connected_;
    std::atomic<bool> is_connecting_;
    mutable std::mutex connection_mutex_;
    
    // メッセージキュー（接続切替時の再送用）
    struct QueuedMessage {
        std::string topic;
        std::string payload;
        int qos;
        bool retained;
        std::chrono::steady_clock::time_point timestamp;
    };
    std::queue<QueuedMessage> message_queue_;
    mutable std::mutex queue_mutex_;
    
    // コールバック関数
    std::function<void(const std::string&)> on_connection_lost_;
    std::function<void()> on_connected_;
    std::function<void(const std::string&, const std::string&)> on_message_received_;
    std::function<void(mqtt::delivery_token_ptr)> on_delivery_complete_;
    
    // 設定
    const int MAX_RECONNECT_ATTEMPTS = 5;
    const int RECONNECT_DELAY_MS = 1000;
    const int MAX_QUEUE_SIZE = 1000;
    const std::chrono::seconds MESSAGE_TIMEOUT{10};
    
    // ブローカー試行管理
    size_t current_broker_try_index_;
    std::vector<std::string> tried_brokers_;

public:
    SelfAdaptiveMqttManager(const std::string& client_id, 
                           const std::string& persistence_dir = "./persist",
                           const std::string& category = "sensor");
    ~SelfAdaptiveMqttManager();
    
    // 初期化と設定
    void add_broker(const std::string& broker_uri);
    void remove_broker(const std::string& broker_uri);
    void set_brokers(const std::vector<std::string>& broker_uris);
    void set_connect_options(const mqtt::connect_options& options);
    
    // 接続管理
    bool connect();
    void disconnect();
    bool is_connected() const;
    
    // メッセージ送信
    mqtt::delivery_token_ptr publish(const std::string& topic, 
                                    const std::string& payload, 
                                    int qos = 1, 
                                    bool retained = false);
    mqtt::delivery_token_ptr publish(mqtt::message_ptr msg);
    
    // サブスクリプション
    mqtt::token_ptr subscribe(const std::string& topic, int qos = 1);
    mqtt::token_ptr unsubscribe(const std::string& topic);
    
    // コールバック設定
    void set_connection_lost_callback(std::function<void(const std::string&)> callback);
    void set_connected_callback(std::function<void()> callback);
    void set_message_received_callback(std::function<void(const std::string&, const std::string&)> callback);
    void set_delivery_complete_callback(std::function<void(mqtt::delivery_token_ptr)> callback);
    
    // 統計情報
    std::vector<std::shared_ptr<BrokerInfo>> get_broker_stats() const;
    std::string get_current_broker_uri() const;
    size_t get_queued_message_count() const;
    
    // モニタリング制御
    void start_monitoring();
    void stop_monitoring();
    bool is_monitoring() const;

protected:
    // MQTTコールバック実装
    void connection_lost(const std::string& cause) override;
    void connected(const std::string& cause) override;
    void message_arrived(mqtt::const_message_ptr msg) override;
    void delivery_complete(mqtt::delivery_token_ptr tok) override;

private:
    // 内部ヘルパー関数
    void create_client(const std::string& broker_uri);
    void destroy_client();
    bool try_connect_to_broker(const std::string& broker_uri);
    void handle_connection_failure(const std::string& broker_uri);
    void switch_to_best_broker();
    void resend_queued_messages();
    void add_message_to_queue(const std::string& topic, 
                             const std::string& payload, 
                             int qos, 
                             bool retained);
    void clear_message_queue();
    
    // ブローカー切り替え時の処理
    void on_broker_switch(const std::string& new_broker_uri);
    void on_metrics_updated(const std::string& broker_uri, 
                           double latency, 
                           double bandwidth, 
                           int connection_count);
};

#endif // MQTT_MANAGER_H 