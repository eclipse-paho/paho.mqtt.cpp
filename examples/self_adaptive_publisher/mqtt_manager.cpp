#include "mqtt_manager.h"
#include <iostream>
#include <chrono>
#include <thread>

SelfAdaptiveMqttManager::SelfAdaptiveMqttManager(const std::string& client_id, 
                                                 const std::string& persistence_dir,
                                                 const std::string& category)
    : client_id_(client_id), persistence_dir_(persistence_dir), 
      is_connected_(false), is_connecting_(false), current_broker_try_index_(0) {
    
    // ブローカーマネージャーを初期化
    broker_manager_ = std::make_shared<BrokerListManager>(category);
    
    // モニタリングスレッドを初期化
    monitor_thread_ = std::make_unique<BrokerMonitorThread>(broker_manager_);
    
    // デフォルト接続オプションを設定
    connect_options_ = mqtt::connect_options_builder()
                          .connect_timeout(std::chrono::seconds(10))
                          .clean_session()
                          .finalize();
    
    // コールバックを設定
    monitor_thread_->set_broker_switch_callback(
        [this](const std::string& new_broker_uri) {
            this->on_broker_switch(new_broker_uri);
        });
    
    monitor_thread_->set_metrics_updated_callback(
        [this](const std::string& broker_uri, double latency, double bandwidth, int connection_count) {
            this->on_metrics_updated(broker_uri, latency, bandwidth, connection_count);
        });
}

SelfAdaptiveMqttManager::~SelfAdaptiveMqttManager() {
    stop_monitoring();
    disconnect();
}

void SelfAdaptiveMqttManager::add_broker(const std::string& broker_uri) {
    broker_manager_->add_broker(broker_uri);
}

void SelfAdaptiveMqttManager::remove_broker(const std::string& broker_uri) {
    broker_manager_->remove_broker(broker_uri);
}

void SelfAdaptiveMqttManager::set_brokers(const std::vector<std::string>& broker_uris) {
    broker_manager_->clear_brokers();
    for (const auto& uri : broker_uris) {
        broker_manager_->add_broker(uri);
    }
}

void SelfAdaptiveMqttManager::set_connect_options(const mqtt::connect_options& options) {
    connect_options_ = options;
}

bool SelfAdaptiveMqttManager::connect() {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    
    if (is_connected_ || is_connecting_) {
        return is_connected_;
    }
    
    is_connecting_ = true;
    
    try {
        // 利用可能なブローカーを順番に試行
        auto available_brokers = broker_manager_->get_all_brokers();
        std::vector<std::string> broker_uris;
        
        for (const auto& broker : available_brokers) {
            if (broker->is_available) {
                broker_uris.push_back(broker->uri);
            }
        }
        
        if (broker_uris.empty()) {
            std::cerr << "利用可能なブローカーがありません" << std::endl;
            is_connecting_ = false;
            return false;
        }
        
        // 各ブローカーを順番に試行
        for (size_t i = 0; i < broker_uris.size(); ++i) {
            std::string target_uri = broker_uris[i];
            std::cout << "初期接続を試行します (" << (i + 1) << "/" << broker_uris.size() << "): " << target_uri << std::endl;
            
            if (try_connect_to_broker(target_uri)) {
                broker_manager_->set_current_broker(target_uri);
                is_connected_ = true;
                is_connecting_ = false;
                current_broker_try_index_ = i;  // 成功したブローカーのインデックスを設定
                
                std::cout << "初期接続が完了しました: " << target_uri << std::endl;
                
                // キューされたメッセージを再送
                resend_queued_messages();
                
                return true;
            } else {
                std::cout << "初期接続に失敗しました: " << target_uri << std::endl;
                broker_manager_->mark_broker_unavailable(target_uri);
            }
        }
        
        // すべてのブローカーで接続に失敗
        std::cerr << "すべてのブローカーで初期接続に失敗しました" << std::endl;
        is_connecting_ = false;
        return false;
        
    } catch (const std::exception& e) {
        std::cerr << "接続エラー: " << e.what() << std::endl;
        is_connecting_ = false;
        return false;
    }
}

void SelfAdaptiveMqttManager::disconnect() {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    
    if (client_) {
        try {
            client_->disconnect()->wait_for(std::chrono::seconds(5));
        } catch (const std::exception& e) {
            std::cerr << "切断エラー: " << e.what() << std::endl;
        }
    }
    
    is_connected_ = false;
    is_connecting_ = false;
    destroy_client();
}

bool SelfAdaptiveMqttManager::is_connected() const {
    return is_connected_;
}

mqtt::delivery_token_ptr SelfAdaptiveMqttManager::publish(const std::string& topic, 
                                                         const std::string& payload, 
                                                         int qos, 
                                                         bool retained) {
    if (!is_connected_) {
        // 接続されていない場合はキューに追加
        add_message_to_queue(topic, payload, qos, retained);
        return nullptr;
    }
    
    try {
        return client_->publish(topic, payload, qos, retained);
    } catch (const std::exception& e) {
        std::cerr << "パブリッシュエラー: " << e.what() << std::endl;
        // エラー時もキューに追加
        add_message_to_queue(topic, payload, qos, retained);
        return nullptr;
    }
}

mqtt::delivery_token_ptr SelfAdaptiveMqttManager::publish(mqtt::message_ptr msg) {
    if (!is_connected_) {
        // 接続されていない場合はキューに追加
        add_message_to_queue(msg->get_topic(), msg->get_payload_str(), 
                           msg->get_qos(), msg->is_retained());
        return nullptr;
    }
    
    try {
        return client_->publish(msg);
    } catch (const std::exception& e) {
        std::cerr << "パブリッシュエラー: " << e.what() << std::endl;
        // エラー時もキューに追加
        add_message_to_queue(msg->get_topic(), msg->get_payload_str(), 
                           msg->get_qos(), msg->is_retained());
        return nullptr;
    }
}

mqtt::token_ptr SelfAdaptiveMqttManager::subscribe(const std::string& topic, int qos) {
    if (!is_connected_) {
        throw std::runtime_error("接続されていません");
    }
    
    return client_->subscribe(topic, qos);
}

mqtt::token_ptr SelfAdaptiveMqttManager::unsubscribe(const std::string& topic) {
    if (!is_connected_) {
        throw std::runtime_error("接続されていません");
    }
    
    return client_->unsubscribe(topic);
}

void SelfAdaptiveMqttManager::set_connection_lost_callback(std::function<void(const std::string&)> callback) {
    on_connection_lost_ = callback;
}

void SelfAdaptiveMqttManager::set_connected_callback(std::function<void()> callback) {
    on_connected_ = callback;
}

void SelfAdaptiveMqttManager::set_message_received_callback(std::function<void(const std::string&, const std::string&)> callback) {
    on_message_received_ = callback;
}

void SelfAdaptiveMqttManager::set_delivery_complete_callback(std::function<void(mqtt::delivery_token_ptr)> callback) {
    on_delivery_complete_ = callback;
}

std::vector<std::shared_ptr<BrokerInfo>> SelfAdaptiveMqttManager::get_broker_stats() const {
    return broker_manager_->get_all_brokers();
}

std::string SelfAdaptiveMqttManager::get_current_broker_uri() const {
    return broker_manager_->get_current_broker_uri();
}

size_t SelfAdaptiveMqttManager::get_queued_message_count() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return message_queue_.size();
}

void SelfAdaptiveMqttManager::start_monitoring() {
    monitor_thread_->start();
}

void SelfAdaptiveMqttManager::stop_monitoring() {
    monitor_thread_->stop();
}

bool SelfAdaptiveMqttManager::is_monitoring() const {
    return monitor_thread_->is_running();
}

void SelfAdaptiveMqttManager::connection_lost(const std::string& cause) {
    std::cout << "接続が切断されました: " << cause << std::endl;
    
    is_connected_ = false;
    
    // コールバックを呼び出し
    if (on_connection_lost_) {
        on_connection_lost_(cause);
    }
    
    // 再接続を試行
    switch_to_best_broker();
}

void SelfAdaptiveMqttManager::connected(const std::string& cause) {
    std::cout << "接続が確立されました: " << cause << std::endl;
    
    is_connected_ = true;
    
    // コールバックを呼び出し
    if (on_connected_) {
        on_connected_();
    }
}

void SelfAdaptiveMqttManager::message_arrived(mqtt::const_message_ptr msg) {
    // コールバックを呼び出し
    if (on_message_received_) {
        on_message_received_(msg->get_topic(), msg->get_payload_str());
    }
}

void SelfAdaptiveMqttManager::delivery_complete(mqtt::delivery_token_ptr tok) {
    // コールバックを呼び出し
    if (on_delivery_complete_) {
        on_delivery_complete_(tok);
    }
}

void SelfAdaptiveMqttManager::create_client(const std::string& broker_uri) {
    client_ = std::make_unique<mqtt::async_client>(broker_uri, client_id_, persistence_dir_);
    client_->set_callback(*this);
}

void SelfAdaptiveMqttManager::destroy_client() {
    client_.reset();
}

bool SelfAdaptiveMqttManager::try_connect_to_broker(const std::string& broker_uri) {
    std::cout << "try_connect_to_broker() を開始: " << broker_uri << std::endl;
    
    try {
        std::cout << "クライアントを作成します..." << std::endl;
        create_client(broker_uri);
        
        std::cout << "接続トークンを取得します..." << std::endl;
        auto token = client_->connect(connect_options_);
        
        std::cout << "接続完了を待機します..." << std::endl;
        token->wait_for(std::chrono::seconds(10));
        
        std::cout << "接続結果を確認します..." << std::endl;
        if (token->get_return_code() == 0) {
            std::cout << "接続成功: " << broker_uri << std::endl;
            return true;
        } else {
            std::cerr << "接続失敗: " << broker_uri << " (コード: " << token->get_return_code() << ")" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "接続エラー (" << broker_uri << "): " << e.what() << std::endl;
        return false;
    }
}

void SelfAdaptiveMqttManager::handle_connection_failure(const std::string& failed_uri) {
    std::cout << "handle_connection_failure() を開始: " << failed_uri << std::endl;
    
    // 失敗したブローカーを一時的に無効化
    broker_manager_->mark_broker_unavailable(failed_uri);
    std::cout << "ブローカーを無効化しました: " << failed_uri << std::endl;
    
    // 次のブローカーを試行
    std::cout << "次のブローカーを試行します..." << std::endl;
    
    // 利用可能なブローカーを確認
    auto available_brokers = broker_manager_->get_all_brokers();
    std::vector<std::string> broker_uris;
    
    for (const auto& broker : available_brokers) {
        if (broker->is_available) {
            broker_uris.push_back(broker->uri);
        }
    }
    
    if (broker_uris.empty()) {
        std::cerr << "利用可能なブローカーがありません" << std::endl;
        return;
    }
    
    // 次のブローカーを試行
    if (current_broker_try_index_ < broker_uris.size()) {
        std::string next_uri = broker_uris[current_broker_try_index_];
        std::cout << "次のブローカーを試行します: " << next_uri << std::endl;
        
        if (try_connect_to_broker(next_uri)) {
            broker_manager_->set_current_broker(next_uri);
            is_connected_ = true;
            is_connecting_ = false;
            current_broker_try_index_ = 0;  // 成功時はリセット
            
            std::cout << "ブローカーに接続しました: " << next_uri << std::endl;
            
            // キューされたメッセージを再送
            resend_queued_messages();
        } else {
            // 接続失敗時は次のブローカーを試行
            current_broker_try_index_++;
            handle_connection_failure(next_uri);
        }
    } else {
        // すべてのブローカーを試行した場合
        std::cout << "すべてのブローカーを試行しました。しばらく待機してから再試行します..." << std::endl;
        current_broker_try_index_ = 0;  // リセット
        is_connecting_ = false;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

// TODO: どこでスコア計算をしているか
// > broker_manager_でスコアを計算しているが、実際の選択では使用していない
void SelfAdaptiveMqttManager::switch_to_best_broker() {
    std::cout << "switch_to_best_broker() を開始します..." << std::endl;
    
    std::lock_guard<std::mutex> lock(connection_mutex_);
    
    if (is_connecting_) {
        std::cout << "既に接続試行中のため、切り替えをスキップします" << std::endl;
        return;  // 既に接続試行中
    }
    
    is_connecting_ = true;
    
    // 現在のクライアントを破棄
    std::cout << "現在のクライアントを破棄します..." << std::endl;
    destroy_client();
    
    // 利用可能なブローカーを順番に試行
    auto available_brokers = broker_manager_->get_all_brokers();
    std::vector<std::string> broker_uris;
    
    for (const auto& broker : available_brokers) {
        if (broker->is_available) {
            broker_uris.push_back(broker->uri);
        }
    }
    
    if (broker_uris.empty()) {
        std::cerr << "利用可能なブローカーがありません" << std::endl;
        is_connecting_ = false;
        return;
    }
    
    // 現在の試行インデックスを進める
    if (current_broker_try_index_ >= broker_uris.size()) {
        current_broker_try_index_ = 0;  // 最初に戻る
    }
    
    std::string target_uri = broker_uris[current_broker_try_index_];
    std::cout << "ブローカーを試行します (" << (current_broker_try_index_ + 1) << "/" << broker_uris.size() << "): " << target_uri << std::endl;
    
    // 新しいブローカーに接続
    if (try_connect_to_broker(target_uri)) {
        broker_manager_->set_current_broker(target_uri);
        is_connected_ = true;
        is_connecting_ = false;
        current_broker_try_index_ = 0;  // 成功時はリセット
        
        std::cout << "ブローカーに接続しました: " << target_uri << std::endl;
        
        // キューされたメッセージを再送
        resend_queued_messages();
    } else {
        // 接続失敗時は次のブローカーを試行
        std::cout << "接続に失敗しました。次のブローカーを試行します..." << std::endl;
        current_broker_try_index_++;
        handle_connection_failure(target_uri);
        is_connecting_ = false;
    }
}

void SelfAdaptiveMqttManager::resend_queued_messages() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    while (!message_queue_.empty()) {
        const auto& queued_msg = message_queue_.front();
        
        try {
            client_->publish(queued_msg.topic, queued_msg.payload, queued_msg.qos, queued_msg.retained);
            std::cout << "キューされたメッセージを再送しました: " << queued_msg.topic << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "メッセージ再送エラー: " << e.what() << std::endl;
            // 再送に失敗した場合はキューに残す
            break;
        }
        
        message_queue_.pop();
    }
}

void SelfAdaptiveMqttManager::add_message_to_queue(const std::string& topic, 
                                                  const std::string& payload, 
                                                  int qos, 
                                                  bool retained) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    // キューサイズの制限
    if (message_queue_.size() >= MAX_QUEUE_SIZE) {
        std::cerr << "メッセージキューが満杯です。古いメッセージを削除します。" << std::endl;
        message_queue_.pop();
    }
    
    QueuedMessage msg{topic, payload, qos, retained, std::chrono::steady_clock::now()};
    message_queue_.push(msg);
    
    std::cout << "メッセージをキューに追加しました: " << topic << " (キューサイズ: " << message_queue_.size() << ")" << std::endl;
}

void SelfAdaptiveMqttManager::clear_message_queue() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!message_queue_.empty()) {
        message_queue_.pop();
    }
}

void SelfAdaptiveMqttManager::on_broker_switch(const std::string& new_broker_uri) {
    std::cout << "ブローカー切り替えが推奨されました: " << new_broker_uri << std::endl;
    
    // 現在のブローカーと比較して切り替えを決定
    if (broker_manager_->should_switch_broker()) {
        std::cout << "ブローカーを切り替えます: " << new_broker_uri << std::endl;
        switch_to_best_broker();
    }
}

void SelfAdaptiveMqttManager::on_metrics_updated(const std::string& broker_uri, 
                                                double latency, 
                                                double bandwidth, 
                                                int connection_count) {
    std::cout << "メトリクス更新: " << broker_uri 
              << " (レイテンシ: " << latency << "ms, "
              << "帯域: " << bandwidth << " bytes/s, "
              << "接続数: " << connection_count << ")" << std::endl;
    
    // ブローカー切り替えの必要性をチェック
    if (broker_manager_->should_switch_broker()) {
        auto best_broker = broker_manager_->find_best_broker();
        if (best_broker) {
            on_broker_switch(best_broker->uri);
        }
    }
}
