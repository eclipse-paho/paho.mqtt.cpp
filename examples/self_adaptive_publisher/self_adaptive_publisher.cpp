// self_adaptive_publisher.cpp
//
// 自己適応型MQTT Publisherのサンプルアプリケーション
//
// このサンプルは以下の機能を実装しています：
//  - 複数ブローカー間での自律的な接続切り替え
//  - レイテンシ、帯域、接続数に基づくブローカー評価
//  - 接続失敗時の自動再接続
//  - メッセージキューの管理と再送
//  - 定期的なメトリクス測定と最適化

#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <atomic>
#include "mqtt_manager.h"

using namespace std;
using namespace std::chrono;

// グローバル変数
atomic<bool> running(true);

// シグナルハンドラー
void signal_handler(int signal) {
    cout << "\n終了シグナルを受信しました。終了処理を開始します..." << endl;
    running = false;
}

// メイン関数
int main(int argc, char* argv[]) {
    // シグナルハンドラーを設定
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    cout << "自己適応型MQTT Publisherを開始します..." << endl;
    
    // カテゴリとブローカーリストを設定
    std::string category = "sensor";
    std::vector<string> broker_uris;
    int broker_arg_start = 1;
    if (argc > 1) {
        category = argv[1];
        broker_arg_start = 2;
    }
    for (int i = broker_arg_start; i < argc; ++i) {
        string uri = argv[i];
        broker_uris.push_back(uri);
    }
    if (broker_uris.empty()) {
        broker_uris = {
            "mqtt://localhost:1883",
            "mqtt://localhost:1884",
            "mqtt://localhost:1885"
        };
    }
    cout << "カテゴリ: " << category << endl;
    cout << "設定されたブローカー:" << endl;
    for (const auto& uri : broker_uris) {
        cout << "  - " << uri << endl;
    }
    
    // 自己適応型MQTTマネージャーを初期化
    SelfAdaptiveMqttManager mqtt_manager("self_adaptive_publisher", "./persist", category);
    
    // ブローカーを設定
    mqtt_manager.set_brokers(broker_uris);
    
    // 接続オプションを設定
    auto conn_opts = mqtt::connect_options_builder()
                        .connect_timeout(seconds(10))
                        .clean_session()
                        .will(mqtt::message("test/status", "Publisher disconnected", 1, false))
                        .finalize();
    mqtt_manager.set_connect_options(conn_opts);
    
    // コールバックを設定
    mqtt_manager.set_connection_lost_callback([](const string& cause) {
        cout << "接続が切断されました: " << cause << endl;
    });
    
    mqtt_manager.set_connected_callback([]() {
        cout << "接続が確立されました" << endl;
    });
    
    mqtt_manager.set_message_received_callback([](const string& topic, const string& payload) {
        cout << "メッセージを受信しました: " << topic << " -> " << payload << endl;
    });
    
    mqtt_manager.set_delivery_complete_callback([](mqtt::delivery_token_ptr tok) {
        // cout << "メッセージ配信完了: " << (tok ? tok->get_message_id() : -1) << endl;
    });
    
    // モニタリングを開始
    mqtt_manager.start_monitoring();
    cout << "ブローカーモニタリングを開始しました" << endl;
    
    // 接続を試行
    cout << "初期接続を開始します..." << endl;
    if (!mqtt_manager.connect()) {
        cerr << "初期接続に失敗しました" << endl;
        return 1;
    }
    
    cout << "初期接続が完了しました" << endl;
    cout << "現在のブローカー: " << mqtt_manager.get_current_broker_uri() << endl;
    
    // メインループ
    int message_count = 0;
    auto last_stats_time = steady_clock::now();
    auto last_publish_time = steady_clock::now();
    const int PUBLISH_INTERVAL_SECONDS = 5;  // 5秒間隔でメッセージ送信
    
    while (running) {
        try {
            auto now = steady_clock::now();
            
            // 定期的にメッセージを送信
            if (mqtt_manager.is_connected() && 
                duration_cast<seconds>(now - last_publish_time).count() >= PUBLISH_INTERVAL_SECONDS) {
                
                string topic = "test/message";
                string payload = "Hello from self-adaptive publisher! Message #" + to_string(++message_count);
                
                auto token = mqtt_manager.publish(topic, payload, 1, false);
                if (token) {
                    cout << "メッセージを送信しました: " << topic << " -> " << payload << endl;
                } else {
                    cout << "メッセージ送信に失敗しました（トークンがnull）" << endl;
                }
                
                last_publish_time = now;
            }
            
            // 定期的に統計情報を表示
            if (duration_cast<seconds>(now - last_stats_time).count() >= 30) {
                cout << "\n=== 統計情報 ===" << endl;
                cout << "現在のブローカー: " << mqtt_manager.get_current_broker_uri() << endl;
                cout << "接続状態: " << (mqtt_manager.is_connected() ? "接続中" : "切断中") << endl;
                cout << "キューされたメッセージ数: " << mqtt_manager.get_queued_message_count() << endl;
                cout << "モニタリング状態: " << (mqtt_manager.is_monitoring() ? "実行中" : "停止中") << endl;
                
                auto broker_stats = mqtt_manager.get_broker_stats();
                cout << "ブローカー統計:" << endl;
                for (const auto& broker : broker_stats) {
                    cout << "  " << broker->uri << ":" << endl;
                    cout << "    レイテンシ: " << broker->latency << "ms" << endl;
                    cout << "    帯域: " << broker->bandwidth << " bytes/s" << endl;
                    cout << "    接続数: " << broker->connection_count << endl;
                    cout << "    スコア: " << broker->score << endl;
                    cout << "    利用可能: " << (broker->is_available ? "はい" : "いいえ") << endl;
                }
                cout << "================\n" << endl;
                
                last_stats_time = now;
            }
            
            // 接続状態の監視
            if (!mqtt_manager.is_connected()) {
                cout << "接続が切断されました。再接続を試行します..." << endl;
                if (mqtt_manager.connect()) {
                    cout << "再接続が完了しました: " << mqtt_manager.get_current_broker_uri() << endl;
                } else {
                    cout << "再接続に失敗しました" << endl;
                }
            }
            
            // 1秒待機
            this_thread::sleep_for(seconds(1));
            
        } catch (const exception& e) {
            cerr << "エラーが発生しました: " << e.what() << endl;
            this_thread::sleep_for(seconds(5));
        }
    }
    
    // 終了処理
    cout << "終了処理を開始します..." << endl;
    
    mqtt_manager.stop_monitoring();
    mqtt_manager.disconnect();
    
    cout << "自己適応型MQTT Publisherを終了しました" << endl;
    
    return 0;
} 