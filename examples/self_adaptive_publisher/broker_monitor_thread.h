#ifndef BROKER_MONITOR_THREAD_H
#define BROKER_MONITOR_THREAD_H

#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include "mqtt/async_client.h"
#include "broker_list_manager.h"

/**
 * ブローカーモニタリングスレッドクラス
 * 定期的にブローカーのメトリクスを測定し、最適なブローカーを選択する
 */
class BrokerMonitorThread {
private:
    std::unique_ptr<std::thread> monitor_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> should_stop_;
    
    std::shared_ptr<BrokerListManager> broker_manager_;
    
    // コールバック関数
    std::function<void(const std::string&)> on_broker_switch_;
    std::function<void(const std::string&, double, double, int)> on_metrics_updated_;
    
    // 設定
    const int MONITOR_INTERVAL_MS = 20000;  // 20秒
    const int LATENCY_TEST_INTERVAL_MS = 5000;  // 5秒
    const int BANDWIDTH_TEST_INTERVAL_MS = 10000;  // 10秒
    const int CONNECTION_CHECK_INTERVAL_MS = 15000;  // 15秒
    
    // テスト用設定
    const std::string LATENCY_TOPIC = "test/latency";
    const std::string BANDWIDTH_TOPIC = "test/bandwidth";
    const std::string CONNECTION_COUNT_TOPIC = "$SYS/brokers/+/stats/connections/count";
    const int TEST_QOS = 1;
    const int BANDWIDTH_TEST_MESSAGE_COUNT = 10;
    const int BANDWIDTH_TEST_MESSAGE_SIZE = 1024;  // 1KB

public:
    BrokerMonitorThread(std::shared_ptr<BrokerListManager> broker_manager);
    ~BrokerMonitorThread();
    
    // スレッド制御
    void start();
    void stop();
    bool is_running() const;
    
    // コールバック設定
    void set_broker_switch_callback(std::function<void(const std::string&)> callback);
    void set_metrics_updated_callback(std::function<void(const std::string&, double, double, int)> callback);
    
    // 設定変更
    void set_monitor_interval(int interval_ms);
    void set_latency_test_interval(int interval_ms);
    void set_bandwidth_test_interval(int interval_ms);
    void set_connection_check_interval(int interval_ms);

private:
    void monitor_loop();
    void measure_latency(const std::string& broker_uri);
    void measure_bandwidth(const std::string& broker_uri);
    void check_connection_count(const std::string& broker_uri);
    
    // テスト用の一時的なMQTTクライアントを作成
    std::unique_ptr<mqtt::async_client> create_test_client(const std::string& broker_uri);
    
    // メトリクス測定のヘルパー関数
    double calculate_latency(const std::string& broker_uri);
    double calculate_bandwidth(const std::string& broker_uri);
    int get_connection_count(const std::string& broker_uri);
};

#endif // BROKER_MONITOR_THREAD_H 