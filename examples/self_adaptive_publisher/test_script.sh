#!/bin/bash

# 自己適応型MQTT Publisher テストスクリプト
# このスクリプトは複数のMQTTブローカーを起動し、Publisherをテストします

set -e

echo "自己適応型MQTT Publisher テストを開始します..."

# テスト用ディレクトリを作成
TEST_DIR="./test_brokers"
mkdir -p $TEST_DIR

# 既存のプロセスをクリーンアップ
cleanup() {
    echo "クリーンアップを実行中..."
    pkill -f "mosquitto" || true
    rm -rf $TEST_DIR
    exit 0
}

trap cleanup SIGINT SIGTERM

# Mosquittoブローカーを起動（複数ポート）
echo "テスト用MQTTブローカーを起動中..."

# ポート1883のブローカー
mosquitto -p 1883 -c /dev/null &
BROKER1_PID=$!

# ポート1884のブローカー
mosquitto -p 1884 -c /dev/null &
BROKER2_PID=$!

# ポート1885のブローカー
mosquitto -p 1885 -c /dev/null &
BROKER3_PID=$!

echo "ブローカーを起動しました:"
echo "  - localhost:1883 (PID: $BROKER1_PID)"
echo "  - localhost:1884 (PID: $BROKER2_PID)"
echo "  - localhost:1885 (PID: $BROKER3_PID)"

# ブローカーの起動を待機
sleep 2

# ブローカーの状態を確認
echo "ブローカーの状態を確認中..."
for port in 1883 1884 1885; do
    if nc -z localhost $port 2>/dev/null; then
        echo "  ✓ localhost:$port は起動中"
    else
        echo "  ✗ localhost:$port は起動していません"
        exit 1
    fi
done

echo ""
echo "自己適応型Publisherを起動します..."
echo "Ctrl+C で終了します"
echo ""

# Publisherを起動
./self_adaptive_publisher mqtt://localhost:1883 mqtt://localhost:1884 mqtt://localhost:1885

# クリーンアップ
cleanup 