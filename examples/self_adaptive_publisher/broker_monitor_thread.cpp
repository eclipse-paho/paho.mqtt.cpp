#include "broker_monitor_thread.h"
#include "mqtt/connect_options.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

BrokerMonitorThread::BrokerMonitorThread(std::shared_ptr<BrokerListManager> broker_manager)
    : broker_manager_(broker_manager), running_(false), should_stop_(false) {
}

BrokerMonitorThread::~BrokerMonitorThread() {
    stop();
}

void BrokerMonitorThread::start() {
    if (running_) {
        return;
    }
    
    should_stop_ = false;
    running_ = true;
    monitor_thread_ = std::make_unique<std::thread>(&BrokerMonitorThread::monitor_loop, this);
    
    std::cout << "ブローカーモニタリングスレッドを開始しました" << std::endl;
}

void BrokerMonitorThread::stop() {
    if (!running_) {
        return;
    }
    
    should_stop_ = true;
    running_ = false;
    
    if (monitor_thread_ && monitor_thread_->joinable()) {
        monitor_thread_->join();
    }
    
    std::cout << "ブローカーモニタリングスレッドを停止しました" << std::endl;
}

bool BrokerMonitorThread::is_running() const {
    return running_;
}

void BrokerMonitorThread::set_broker_switch_callback(std::function<void(const std::string&)> callback) {
    on_broker_switch_ = callback;
}

void BrokerMonitorThread::set_metrics_updated_callback(std::function<void(const std::string&, double, double, int)> callback) {
    on_metrics_updated_ = callback;
}

void BrokerMonitorThread::set_monitor_interval(int interval_ms) {
    // 実装では定数として定義されているため、動的変更は未対応
    // 必要に応じて実装を拡張
}

void BrokerMonitorThread::set_latency_test_interval(int interval_ms) {
    // 実装では定数として定義されているため、動的変更は未対応
    // 必要に応じて実装を拡張
}

void BrokerMonitorThread::set_bandwidth_test_interval(int interval_ms) {
    // 実装では定数として定義されているため、動的変更は未対応
    // 必要に応じて実装を拡張
}

void BrokerMonitorThread::set_connection_check_interval(int interval_ms) {
    // 実装では定数として定義されているため、動的変更は未対応
    // 必要に応じて実装を拡張
}

void BrokerMonitorThread::monitor_loop() {
    auto last_latency_check = std::chrono::steady_clock::now();
    auto last_bandwidth_check = std::chrono::steady_clock::now();
    auto last_connection_check = std::chrono::steady_clock::now();
    
    while (!should_stop_) {
        auto now = std::chrono::steady_clock::now();
        
        // レイテンシ測定
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_latency_check).count() >= LATENCY_TEST_INTERVAL_MS) {
            auto broker_uris = broker_manager_->get_broker_uris();
            for (const auto& uri : broker_uris) {
                if (!should_stop_) {
                    measure_latency(uri);
                }
            }
            last_latency_check = now;
        }
        
        // 帯域測定
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_bandwidth_check).count() >= BANDWIDTH_TEST_INTERVAL_MS) {
            auto broker_uris = broker_manager_->get_broker_uris();
            for (const auto& uri : broker_uris) {
                if (!should_stop_) {
                    measure_bandwidth(uri);
                }
            }
            last_bandwidth_check = now;
        }
        
        // 接続数チェック
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_connection_check).count() >= CONNECTION_CHECK_INTERVAL_MS) {
            auto broker_uris = broker_manager_->get_broker_uris();
            for (const auto& uri : broker_uris) {
                if (!should_stop_) {
                    check_connection_count(uri);
                }
            }
            last_connection_check = now;
        }
        
        // メインループの間隔
        std::this_thread::sleep_for(std::chrono::milliseconds(MONITOR_INTERVAL_MS));
    }
}

void BrokerMonitorThread::measure_latency(const std::string& broker_uri) {
    try {
        double latency = calculate_latency(broker_uri);
        
        // 現在のメトリクスを取得
        auto brokers = broker_manager_->get_all_brokers();
        for (const auto& broker : brokers) {
            if (broker->uri == broker_uri) {
                double current_bandwidth = broker->bandwidth;
                int current_connections = broker->connection_count;
                
                // メトリクスを更新
                broker_manager_->update_broker_metrics(broker_uri, latency, current_bandwidth, current_connections);
                
                // コールバックを呼び出し
                if (on_metrics_updated_) {
                    on_metrics_updated_(broker_uri, latency, current_bandwidth, current_connections);
                }
                
                std::cout << "レイテンシ測定完了: " << broker_uri << " = " << latency << "ms" << std::endl;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "レイテンシ測定エラー (" << broker_uri << "): " << e.what() << std::endl;
        broker_manager_->mark_broker_unavailable(broker_uri);
    }
}

void BrokerMonitorThread::measure_bandwidth(const std::string& broker_uri) {
    try {
        double bandwidth = calculate_bandwidth(broker_uri);
        
        // 現在のメトリクスを取得
        auto brokers = broker_manager_->get_all_brokers();
        for (const auto& broker : brokers) {
            if (broker->uri == broker_uri) {
                double current_latency = broker->latency;
                int current_connections = broker->connection_count;
                
                // メトリクスを更新
                broker_manager_->update_broker_metrics(broker_uri, current_latency, bandwidth, current_connections);
                
                // コールバックを呼び出し
                if (on_metrics_updated_) {
                    on_metrics_updated_(broker_uri, current_latency, bandwidth, current_connections);
                }
                
                std::cout << "帯域測定完了: " << broker_uri << " = " << bandwidth << " bytes/s" << std::endl;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "帯域測定エラー (" << broker_uri << "): " << e.what() << std::endl;
        broker_manager_->mark_broker_unavailable(broker_uri);
    }
}

void BrokerMonitorThread::check_connection_count(const std::string& broker_uri) {
    try {
        int connection_count = get_connection_count(broker_uri);
        
        // 現在のメトリクスを取得
        auto brokers = broker_manager_->get_all_brokers();
        for (const auto& broker : brokers) {
            if (broker->uri == broker_uri) {
                double current_latency = broker->latency;
                double current_bandwidth = broker->bandwidth;
                
                // メトリクスを更新
                broker_manager_->update_broker_metrics(broker_uri, current_latency, current_bandwidth, connection_count);
                
                // コールバックを呼び出し
                if (on_metrics_updated_) {
                    on_metrics_updated_(broker_uri, current_latency, current_bandwidth, connection_count);
                }
                
                std::cout << "接続数チェック完了: " << broker_uri << " = " << connection_count << " connections" << std::endl;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "接続数チェックエラー (" << broker_uri << "): " << e.what() << std::endl;
        // 接続数取得の失敗は致命的ではないため、ブローカーを無効化しない
    }
}

std::unique_ptr<mqtt::async_client> BrokerMonitorThread::create_test_client(const std::string& broker_uri) {
    std::string client_id = "monitor_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    
    auto client = std::make_unique<mqtt::async_client>(broker_uri, client_id);
    
    // 接続オプションを設定
    auto conn_opts = mqtt::connect_options_builder()
                        .connect_timeout(std::chrono::seconds(5))
                        .clean_session()
                        .finalize();
    
    // 接続
    auto token = client->connect(conn_opts);
    token->wait_for(std::chrono::seconds(5));
    
    if (token->get_return_code() != 0) {
        throw std::runtime_error("接続に失敗しました: " + broker_uri);
    }
    
    return client;
}

double BrokerMonitorThread::calculate_latency(const std::string& broker_uri) {
    auto client = create_test_client(broker_uri);
    
    // レイテンシ測定用のコールバッククラス
    class LatencyCallback : public virtual mqtt::callback {
    public:
        std::atomic<bool> message_received{false};
        std::chrono::steady_clock::time_point send_time;
        std::chrono::steady_clock::time_point receive_time;
        
        void message_arrived(mqtt::const_message_ptr msg) override {
            receive_time = std::chrono::steady_clock::now();
            message_received = true;
        }
    };
    
    LatencyCallback callback;
    client->set_callback(callback);
    
    // レイテンシ測定トピックをサブスクライブ
    client->subscribe(LATENCY_TOPIC, TEST_QOS)->wait_for(std::chrono::seconds(5));
    
    // タイムスタンプ付きメッセージを送信
    auto now = std::chrono::steady_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::ostringstream payload;
    payload << "latency_test:" << timestamp;
    
    callback.send_time = now;
    client->publish(LATENCY_TOPIC, payload.str().c_str(), payload.str().length(), TEST_QOS, false)->wait_for(std::chrono::seconds(5));
    
    // 応答を待機（最大5秒）
    auto start_wait = std::chrono::steady_clock::now();
    while (!callback.message_received && 
           std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_wait).count() < 5000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    if (!callback.message_received) {
        throw std::runtime_error("レイテンシ測定タイムアウト");
    }
    
    // レイテンシを計算
    auto latency_ms = std::chrono::duration_cast<std::chrono::microseconds>(
        callback.receive_time - callback.send_time).count() / 1000.0;
    
    // 切断
    client->disconnect()->wait_for(std::chrono::seconds(5));
    
    return latency_ms;
}

double BrokerMonitorThread::calculate_bandwidth(const std::string& broker_uri) {
    auto client = create_test_client(broker_uri);
    
    // 帯域測定用のコールバッククラス
    class BandwidthCallback : public virtual mqtt::callback {
    public:
        std::atomic<int> messages_sent{0};
        std::atomic<int> messages_delivered{0};
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;
        
        void delivery_complete(mqtt::delivery_token_ptr tok) override {
            messages_delivered++;
            if (messages_delivered == messages_sent) {
                end_time = std::chrono::steady_clock::now();
            }
        }
    };
    
    BandwidthCallback callback;
    client->set_callback(callback);
    
    // ダミーメッセージを作成
    std::string dummy_message(BANDWIDTH_TEST_MESSAGE_SIZE, 'A');
    
    // 連続送信開始
    callback.start_time = std::chrono::steady_clock::now();
    
    for (int i = 0; i < BANDWIDTH_TEST_MESSAGE_COUNT; ++i) {
        client->publish(BANDWIDTH_TOPIC, dummy_message.c_str(), dummy_message.length(), TEST_QOS, false);
        callback.messages_sent++;
    }
    
    // 全メッセージの配信完了を待機
    auto start_wait = std::chrono::steady_clock::now();
    while (callback.messages_delivered < callback.messages_sent && 
           std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_wait).count() < 10000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    if (callback.messages_delivered < callback.messages_sent) {
        throw std::runtime_error("帯域測定タイムアウト");
    }
    
    // 帯域を計算
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        callback.end_time - callback.start_time).count();
    
    if (duration_ms == 0) {
        duration_ms = 1;  // ゼロ除算を防ぐ
    }
    
    double total_bytes = BANDWIDTH_TEST_MESSAGE_SIZE * BANDWIDTH_TEST_MESSAGE_COUNT;
    double bandwidth_bytes_per_sec = (total_bytes * 1000.0) / duration_ms;
    
    // 切断
    client->disconnect()->wait_for(std::chrono::seconds(5));
    
    return bandwidth_bytes_per_sec;
}

int BrokerMonitorThread::get_connection_count(const std::string& broker_uri) {
    auto client = create_test_client(broker_uri);
    
    // 接続数取得用のコールバッククラス
    class ConnectionCountCallback : public virtual mqtt::callback {
    public:
        std::atomic<bool> message_received{false};
        std::string connection_count_str;
        
        void message_arrived(mqtt::const_message_ptr msg) override {
            connection_count_str = msg->get_payload_str();
            message_received = true;
        }
    };
    
    ConnectionCountCallback callback;
    client->set_callback(callback);
    
    // $SYSトピックをサブスクライブ（EMQXの場合）
    try {
        client->subscribe(CONNECTION_COUNT_TOPIC, TEST_QOS)->wait_for(std::chrono::seconds(5));
        
        // 応答を待機（最大5秒）
        auto start_wait = std::chrono::steady_clock::now();
        while (!callback.message_received && 
               std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start_wait).count() < 5000) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (callback.message_received) {
            // 接続数をパース
            try {
                return std::stoi(callback.connection_count_str);
            } catch (const std::exception& e) {
                std::cerr << "接続数パースエラー: " << e.what() << std::endl;
                return 0;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "$SYSトピックサブスクライブエラー: " << e.what() << std::endl;
    }
    
    // 切断
    client->disconnect()->wait_for(std::chrono::seconds(5));
    
    // デフォルト値（取得できない場合）
    return 0;
}
