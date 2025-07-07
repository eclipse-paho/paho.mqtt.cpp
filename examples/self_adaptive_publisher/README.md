# 自己適応型MQTT Publisher

このサンプルは、Paho MQTT C++クライアントを拡張した自己適応型Publisherを実装しています。複数のブローカー間で自律的に最適な接続先を選択・切り替える機能を提供します。

## 機能

### 主要機能
- **複数ブローカー管理**: 複数のMQTTブローカーURIを管理し、動的に接続先を切り替え
- **自律的ブローカー選択**: レイテンシ、帯域、接続数に基づく総合スコアで最適なブローカーを選択
- **自動再接続**: 接続失敗時の自動的な他のブローカーへの再接続
- **メッセージキュー**: 接続切替時のメッセージ損失を防ぐためのキュー機能
- **定期的メトリクス測定**: 20秒周期でのブローカー性能評価

### 評価指標
1. **レイテンシ**: `test/latency`トピックを使用した往復時間（RTT）測定
2. **帯域**: 固定サイズメッセージの連続送信によるスループット測定
3. **接続数**: EMQXの`$SYS/brokers/+/stats/connections/count`トピックからの取得

### スコア計算
各ブローカーの総合スコアは以下の重みで計算されます：
- レイテンシ: 40% (低いほど高スコア)
- 帯域: 40% (高いほど高スコア)
- 接続数: 20% (低いほど高スコア)

## ビルド

```bash
# プロジェクトルートディレクトリで
mkdir build
cd build
cmake ..
make self_adaptive_publisher
```

## 使用方法

### 基本的な使用方法
```bash
# デフォルトのブローカーリスト（localhost:1883, 1884, 1885）を使用
./self_adaptive_publisher

# カスタムブローカーリストを指定
./self_adaptive_publisher mqtt://broker1.example.com:1883 mqtt://broker2.example.com:1883
```

### 実行例
```bash
$ ./self_adaptive_publisher mqtt://localhost:1883 mqtt://localhost:1884
自己適応型MQTT Publisherを開始します...
設定されたブローカー:
  - mqtt://localhost:1883
  - mqtt://localhost:1884
ブローカーモニタリングを開始しました
ブローカーに接続しました: mqtt://localhost:1883
初期接続が完了しました
メッセージを送信しました: test/message -> Hello from self-adaptive publisher! Message #1
レイテンシ測定完了: mqtt://localhost:1883 = 15.2ms
レイテンシ測定完了: mqtt://localhost:1884 = 12.8ms
帯域測定完了: mqtt://localhost:1883 = 1250000 bytes/s
帯域測定完了: mqtt://localhost:1884 = 1400000 bytes/s
接続数チェック完了: mqtt://localhost:1883 = 45 connections
接続数チェック完了: mqtt://localhost:1884 = 32 connections
ブローカーを切り替えました: mqtt://localhost:1884
```

## アーキテクチャ

### 主要クラス

#### `BrokerInfo`
個別ブローカーの情報を管理
- URI、レイテンシ、帯域、接続数、スコア、利用可能性

#### `BrokerListManager`
ブローカーリストの管理とスコア計算
- ブローカーの追加・削除・更新
- 最適ブローカーの選択
- スコア計算ロジック

#### `BrokerMonitorThread`
定期的なメトリクス測定を実行
- レイテンシ測定（5秒間隔）
- 帯域測定（10秒間隔）
- 接続数チェック（15秒間隔）

#### `SelfAdaptiveMqttManager`
メインのMQTT管理クラス
- 接続管理と切り替え
- メッセージ送信とキュー管理
- コールバック処理

### スレッド構成
- **メインスレッド**: メッセージ送信とユーザーインターフェース
- **モニタリングスレッド**: ブローカー性能測定
- **MQTT内部スレッド**: Pahoライブラリの非同期処理

## 設定

### タイミング設定
- メトリクス測定間隔: 20秒
- レイテンシ測定間隔: 5秒
- 帯域測定間隔: 10秒
- 接続数チェック間隔: 15秒

### スコア計算パラメータ
- レイテンシ基準値: 100ms
- 帯域基準値: 1MB/s
- 接続数基準値: 100接続
- 切り替え閾値: 10%のスコア差

### キュー設定
- 最大キューサイズ: 1000メッセージ
- メッセージタイムアウト: 10秒

## EMQX特有の設定

### $SYSトピックの有効化
EMQXで接続数取得を有効にするには、ACL設定を変更する必要があります：

```bash
# emqx.conf または file_auth.conf で
mqtt.acl_nomatch = allow
mqtt.acl_file = etc/acl.conf
```

```bash
# etc/acl.conf で
{allow, all, subscribe, ["$SYS/#"]}.
```

### クラスタ設定
EMQXクラスタ環境では、各ノードの`$SYS/brokers/{node}/stats/connections/count`トピックから接続数を取得できます。

## トラブルシューティング

### よくある問題

1. **接続できない**
   - ブローカーが起動しているか確認
   - ファイアウォール設定を確認
   - URI形式が正しいか確認

2. **$SYSトピックが取得できない**
   - EMQXのACL設定を確認
   - 管理者権限でログインしているか確認

3. **メトリクス測定が失敗する**
   - ネットワーク接続を確認
   - ブローカーの負荷状況を確認

### ログ出力
プログラムは詳細なログを出力します：
- 接続・切断イベント
- メトリクス測定結果
- ブローカー切り替え
- エラー情報

## ライセンス

このサンプルはEclipse Public License v2.0とEclipse Distribution License v1.0の下で提供されています。 