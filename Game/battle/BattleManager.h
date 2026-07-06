/**
 * BattleManager.h
 * バトル管理
 */
#pragma once
#include <future>
#include "actor/EventCharacter.h"
#include "actor/EventCharacterSpawnManagerObject.h"
#include "camera/CameraCommon.h"
#include "camera/CameraSteering.h"
#include "ui/Layout.h"
#include "effect/EffectManager.h"
#include "effect/EffectManager2D.h"
#include "battle/IDamagePopListener.h"

namespace nsK2EngineLow
{
    class SkyCube;
}
namespace app
{
    namespace actor
    {
        class BattleCharacter;
        class EventCharacter;
        class CharacterSteering;
        class StaticGimmick;
        class PipeGimmick;
        class MoonGimmick;
        class EventCharacterSpawnManager;
        class EventCharacterSpawnManagerObject;
    }
    namespace collision
    {
        class GhostBody;
    }
    namespace core
    {
        class PauseManagerObject;
    }
    namespace effect
    {
        class EffectManagerObject;
    }
    namespace ui
    {
        class BattleSequence;
        class TimerUIObject;
        class PlayerHpUIObject;
        class EnemyHpUIObject;
        class LevelUpUIObject;
        class DamagePopPool;
        class PhaseUI;
    }
}


namespace app
{
    namespace battle
    {
        /**
         * バトル管理クラス
         */
        class BattleManager
        {
        public:
            /**
             * 通知処理
             * NOTE: 単体テストをしやすいように分離している
             *       通信などで非同期に処理する場合にも対応しやすい
             */
            struct INotify : Noncopyable
            {
                /** 衝突した物体A */
                app::collision::GhostBody* a = nullptr;
                /** 衝突した物体B */
                app::collision::GhostBody* b = nullptr;
                /** 通知の種別IDを返す */
                virtual uint32_t ID() const = 0;
            };


            /** ダメージ通知処理 */
            struct DamageNotify : public INotify
            {
                /** 被ダメージ対象の敵種別 */
                enum class EnemyType
                {
                    Stone,
                    Mushroom,
                };

                /** 攻撃者 */
                app::actor::BattleCharacter* attacker = nullptr;
                /** 被攻撃者 */
                app::actor::Character* defender = nullptr;
                /** ノックバック方向 */
                Vector3 knockBackDirection;
                /** 溜め攻撃による吹き飛ばしか */
                bool isBlowBack = false;
                /** 溜め攻撃レベル (0=通常, 1/2/3=チャージレベル) */
                int chargeLevel = 0;
                /** コンボの何発目か (0=1発目, 1=2発目, 2=3発目) */
                int comboIndex = 0;
                /** 被ダメージ対象の敵種別 */
                EnemyType enemyType = EnemyType::Stone;

                /** 比較用（クラス名から呼ぶ用） */
                static uint32_t StaticID() { return 1; }
                /** 仮想関数版（インスタンス経由で呼ぶ用） */
                virtual uint32_t ID() const override { return StaticID(); }
            };

            /** プレイヤーへのダメージ通知 */
            struct PlayerDamageNotify : public INotify
            {
                /** 攻撃してきた敵 */
                app::actor::Character* attacker = nullptr;

                /** 比較用（クラス名から呼ぶ用） */
                static uint32_t StaticID() { return 2; }
                /** 仮想関数版（インスタンス経由で呼ぶ用） */
                virtual uint32_t ID() const override { return StaticID(); }
            };

            /** スポーンエフェクト遅延再生エントリ */
            struct PendingSpawnEffect
            {
                /** エフェクトの種類 */
                int effectKind = 0;
                /** エフェクトのスケール */
                Vector3 scale = Vector3::Zero;
                /** 連続再生用のタイマー */
                float timer = 0.0f;
            };

            /** TBDR（タイルベース遅延レンダリング）用スポーン時動的ライトエントリ */
            struct TBDRSpawnLightEntry
            {
                /** ライトの配置座標 */
                Vector3 position;
                /** ライトの最大輝度時のカラー */
                Vector3 peakColor;
                /** ライトの影響半径 */
                float   range = 0.f;
                /** 経過時間タイマー */
                float   timer = 0.0f;
                /** ライトの持続時間 */
                float   duration = 1.0f;
            };


        private:
            /** バトルキャラクターのインスタンス */
            app::actor::BattleCharacter* battleCharacter_ = nullptr;
            /** イベントキャラクターのスポーン管理オブジェクト */
            app::actor::EventCharacterSpawnManagerObject* eventCharacterSpawnManagerObject_ = nullptr;
            /** 月のオブジェクト */
            app::actor::MoonGimmick* moon_ = nullptr;
            /** カメラの操舵制御オブジェクト */
            app::camera::RefCameraController gameCameraController_ = nullptr;
            /** ポーズ画面管理オブジェクト */
            app::core::PauseManagerObject* pauseManagerObject_ = nullptr;
            /** バトル演出シーケンスオブジェクト */
            app::ui::BattleSequence* battleSequenceObject_ = nullptr;
            /** 制限時間UIオブジェクト */
            app::ui::TimerUIObject* timerUIObject_ = nullptr;
            /** フェーズ表示UIオブジェクト（仮置き）*/
            app::ui::PhaseUI* phaseUI_ = nullptr;
            /** プレイヤーHPバーUIオブジェクト */
            app::ui::PlayerHpUIObject* playerHpUIObject_ = nullptr;
            /** レベルアップ演出UIオブジェクト */
            app::ui::LevelUpUIObject* levelUpObject_ = nullptr;
            /** ダメージ数字のポップアップ表示用プール */
            app::ui::DamagePopPool* damagePopPool_ = nullptr;

            /** 3Dエフェクト管理オブジェクト */
            EffectManagerObject* effectManagerObject_ = nullptr;
            /** 2Dエフェクト管理オブジェクト */
            EffectManager2DObject* effectManager2DObject_ = nullptr;
            /** ダメージポップ通知先 */
            IDamagePopListener* damagePopListener_ = nullptr;
            /** スカイキューブのオブジェクト */
            nsK2EngineLow::SkyCube* skyCube_ = nullptr;

            /** スポーンしているストーン敵のリスト */
            std::vector<app::actor::StoneEventCharacter*> stoneEventCharacters_;
            /** スポーンしているキノコ敵のリスト */
            std::vector<app::actor::MushroomEventCharacter*> mushroomEventCharacters_;
            /** 配置されているテスト用静的ギミックのリスト */
            /** TODO: testという表記を変更したい。 */
            std::vector<app::actor::StaticGimmick*> testGimmickList_;
            /** 発生した通知（ダメージ等）を保持するリスト */
            std::vector<std::unique_ptr<INotify>> notifyList_;
            /** 遅延再生を待機しているスポーンエフェクトのリスト */
            std::vector<PendingSpawnEffect> pendingSpawnEffects_;
            /** TBDR（タイルベース遅延レンダリング）用のスポーン時動的ライトのリスト */
            std::vector<TBDRSpawnLightEntry> tbdrSpawnLights_;

            /** キャラクターの操舵制御オブジェクト */
            std::unique_ptr<app::actor::CharacterSteering> characterSteering_ = nullptr;
            /** カメラの操舵制御オブジェクト */
            std::unique_ptr<app::camera::CameraSteering> cameraSteering_ = nullptr;

            /** ケース2用：非同期テクスチャ一括アップロード */
            std::future<void> texturePreloadFuture_;
            /** ケース2用：非同期シェーダー一括コンパイル */
            std::future<void> shaderPrecompileFuture_;

            /** 再生予定の位置を保持 */
            Vector3 reservedEffectPos_;
            /** 攻撃開始時の向きを固定保持 */
            Vector3 reservedEffectDir_;
            /** プレイヤー出現時のエフェクト座標 */
            Vector3 playerSpawnEffectPos_ = Vector3::Zero;
            /** チュートリアル用のストーン敵のスポーン位置 */
            Vector3 tutorialStoneSpawnPos_ = Vector3::Zero;
            /** チュートリアル用のきのこ敵のスポーン位置 */
            Vector3 tutorialMushroomSpawnPos_ = Vector3::Zero;

            /** 再生予定のエフェクト回転 */
            Quaternion reservedEffectRot_;
            /** チュートリアル用の敵のスポーン時回転 */
            Quaternion tutorialEnemySpawnRot_;

            /** ガード成功エフェクトなどの連続発生を防ぐクールダウンタイマー */
            float guardSuccessCooldown_ = 0.0f;
            /** アニメーション初期化を待ってからシーケンスを起動するための遅延タイマー */
            float battleSequenceStartTimer_ = 0.1f;
            /** エフェクトを遅延再生するためのカウントタイマー */
            float effectDelayTimer_ = 0.0f;
            /** バトルの残り時間(秒) */
            float remainTime_ = 120.0f;
            /** プレイヤーの被弾時などの無敵時間カウントタイマー */
            float invincibleTimer_ = 0.0f;
            /** プレイヤー出現時のライト演出用タイマー */
            float pendingPlayerSpawnLightTimer_ = -1.0f;

            /** ストーン敵を倒した累計数 */
            int stoneKillCount_ = 0;
            /** きのこ敵を倒した累計数 */
            int mushroomKillCount_ = 0;
            /** 前フレームに表示していたカウントダウン数値（重複再生防止用） */
            int lastCountShown_ = -1;
            /** タイムアップ時に画面を静止させるフレーム数の経過カウント */
            int timeUpFreezeFrames_ = 0;
            /** 分割ロード（LoadStep）の現在の進行ステップ */
            int loadStep_ = 0;

            /** ゲームが一時停止（ポーズ）状態であるか */
            bool isPause_                  = false;
            /** 現在チュートリアルモードとして実行中か */
            bool isTutorialMode_           = false;
            /** バトルシーケンス完了を検知するためのエッジ検出フラグ */
            bool wasOpeningSequenceDone_   = false;
            /** ゲームオーバー時にゲーム処理を凍結させるか */
            bool gameOverFreeze_           = false;
            /** チュートリアル中に敵の移動・AIを許可するか */
            bool tutorialEnemyMoveEnabled_ = false;
            /** チュートリアル用の敵の追加スポーンが必要か */
            bool tutorialNeedsSpawn_        = false;
            /** チュートリアル中の敵全滅時に自動リスポーンを行うか */
            bool tutorialRespawnEnabled_    = false;
            /** プレイヤーの操作入力を受け付けるか */
            bool playerInputEnabled_          = true;
            /** 溜め攻撃（チャージ攻撃）入力を一時的に禁止するか */
            bool suppressChargeAttackInput_   = false;
            /** コンボ（3連撃）がちょうど完了した瞬間の通知フラグ */
            bool comboAttackJustCompleted_    = false;
            /** 溜め攻撃がちょうど完了した瞬間の通知フラグ */
            bool chargeAttackJustCompleted_   = false;
            /** ガードがちょうど成功した瞬間の通知フラグ */
            bool guardJustSucceeded_          = false;
            /** 回避がちょうど成功した瞬間の通知フラグ */
            bool avoidJustSucceeded_          = false;
            /** チュートリアルのメッセージ表示等でゲームを一時停止するか */
            bool tutorialFreeze_              = false;
            /** チュートリアル用の無敵状態か（HP減少と死亡判定をスキップ） */
            bool tutorialNoDamage_            = false;
            /** チュートリアル用のレベルアップが通知済みか */
            bool tutorialLevelUpNotified_     = false;
            /** 制限時間切れ（タイムアップ）が既に発生したか */
            bool timeUpTriggered_             = false;
            /** プレイヤーが現在無敵状態であるか */
            bool isInvincible_                = false;
            /** エフェクトの再生遅延を待機中であるか */
            bool isWaitEffectPlay_            = false;
            /** ステージの松明（トーチ）用のライトが有効化されているか */
            bool torchLightsEnabled_          = false;


        private:
            /** コンストラクタ（シングルトンのためprivate） */
            BattleManager();
            /** デストラクタ */
            ~BattleManager();

            /** キャラクター1体分の各種アセットの非同期プリロード要求を発行。 */
            void PreloadCharacterAssets(app::actor::CharacterInitializeParameter& param);


        public:
            /** 初期化 */
            void Start();
            /** 分割ロード。falseを返す間は毎フレーム呼ぶ。trueで完了。 */
            bool LoadStep();
            /** 更新処理 */
            void Update();
            /** 描画処理 */
            void Render(RenderContext& rc);

            /** 通知オブジェクトを登録する。所有権はBattleManagerに移る */
            void AddNotify(INotify* notify)
            {
                notifyList_.push_back(std::move(std::unique_ptr<INotify>(notify)));
            }
            /** ptr がプールされた有効な石インスタンスか確認（ダングリングポインタ防止） */
            bool IsValidStoneEventCharacter(const app::actor::StoneEventCharacter* ptr) const
            {
                for (auto* stone : stoneEventCharacters_) {
                    if (stone == ptr) return true;
                }
                return false;
            }
            /** ptr がプールされた有効なキノコインスタンスか確認（ダングリングポインタ防止） */
            bool IsValidMushroomEventCharacter(const app::actor::MushroomEventCharacter* ptr) const
            {
                for (auto* mushroom : mushroomEventCharacters_) {
                    if (mushroom == ptr) return true;
                }
                return false;
            }
            /**
             * ダメージポップリスナーを登録
             * BattleManager はリスナーの所有権を持たない
             */
            void SetDamagePopListener(IDamagePopListener* listener)
            {
                damagePopListener_ = listener;
            }

            IDamagePopListener* GetDamagePopListener() const
            {
                return damagePopListener_;
            }

            /** プレイヤーが死亡しているか */
            bool IsPlayerDead() const;
            /** ゲームオーバー時の処理凍結を設定する */
            void SetGameOverFreeze(bool v);
            /** HPバーのブラー前描画を設定する */
            void SetHpBarPreBlurRender(bool v);

            /** ステージの松明ライトを有効/無効にする */
            void SetTorchLightsEnabled(bool enabled) { torchLightsEnabled_ = enabled; }
            bool IsTorchLightsEnabled() const { return torchLightsEnabled_; }
            /** ポーズ状態を設定する */
            void SetPause(bool isPause);
            /** レベルアップUIオブジェクトを設定する */
            void SetLevelUpUIObject(app::ui::LevelUpUIObject* obj) { levelUpObject_ = obj; }
            /** 制限時間終了後の演出が完了したか */
            bool IsTimeUpFinished() const;
            /** ジャスト回避などでプレイヤーの経験値ゲージを加算する */
            void AddPlayerGauge(int amount);
            /** チュートリアルモードを有効にする（Start()の前に呼ぶこと） */
            void SetTutorialMode(bool isTutorial) { isTutorialMode_ = isTutorial; }
            bool IsTutorialMode() const { return isTutorialMode_; }
            /** オープニングシーケンス（3/2/1/START）が完了したか */
            bool IsOpeningSequenceDone() const;
            /** チュートリアル中の敵の動きを許可/停止する */
            void SetTutorialEnemyMoveEnabled(bool enabled);
            /** プレイヤーの操作入力を許可/停止する */
            void SetPlayerInputEnabled(bool enabled) { playerInputEnabled_ = enabled; }
            bool IsPlayerInputEnabled() const { return playerInputEnabled_; }
            /** チュートリアルメッセージ進行中に溜め攻撃入力を抑制する */
            void SetSuppressChargeAttackInput(bool v) { suppressChargeAttackInput_ = v; }
            bool IsSuppressChargeAttackInput() const { return suppressChargeAttackInput_; }
            /** チュートリアル用: コンボ攻撃（3連撃完了）通知 */
            void NotifyComboAttackCompleted()  { comboAttackJustCompleted_  = true; }
            /** チュートリアル用: コンボ攻撃完了フラグを確認して消費する */
            bool CheckAndConsumeComboAttackCompleted()
            {
                if (!comboAttackJustCompleted_) return false;
                comboAttackJustCompleted_ = false;
                return true;
            }
            /** チュートリアル用: 溜め攻撃完了通知 */
            void NotifyChargeAttackCompleted() { chargeAttackJustCompleted_ = true; }
            /** チュートリアル用: 溜め攻撃完了フラグを確認して消費する */
            bool CheckAndConsumeChargeAttackCompleted()
            {
                if (!chargeAttackJustCompleted_) return false;
                chargeAttackJustCompleted_ = false;
                return true;
            }
            /** チュートリアル用: ガード成功通知 */
            void NotifyGuardSucceeded() { guardJustSucceeded_ = true; }
            /** チュートリアル用: ガード成功フラグを確認して消費する */
            bool CheckAndConsumeGuardSucceeded()
            {
                if (!guardJustSucceeded_) return false;
                guardJustSucceeded_ = false;
                return true;
            }
            /** チュートリアル用: 回避実行通知 */
            void NotifyAvoidSucceeded() { avoidJustSucceeded_ = true; }
            /** チュートリアル用: 回避成功フラグを確認して消費する */
            bool CheckAndConsumeAvoidSucceeded()
            {
                if (!avoidJustSucceeded_) return false;
                avoidJustSucceeded_ = false;
                return true;
            }
            /** チュートリアル用: レベルアップ通知 */
            void NotifyTutorialLevelUp() { tutorialLevelUpNotified_ = true; }
            /** チュートリアル用: レベルアップフラグを確認して消費する */
            bool CheckAndConsumeTutorialLevelUp()
            {
                if (!tutorialLevelUpNotified_) return false;
                tutorialLevelUpNotified_ = false;
                return true;
            }
            /** チュートリアル用: 特定メッセージ表示中にゲームを一時停止する */
            void SetTutorialFreeze(bool freeze) { tutorialFreeze_ = freeze; }
            /** チュートリアル用: trueの間はHP減算とOnDeadをスキップする（アニメーション・ダメージポップは出る） */
            void SetTutorialNoDamage(bool v) { tutorialNoDamage_ = v; }
            bool IsTutorialNoDamage() const { return tutorialNoDamage_; }
            /** チュートリアル用: 敵全滅時のリスポーンを有効/無効にする */
            void SetTutorialRespawnEnabled(bool enabled) { tutorialRespawnEnabled_ = enabled; }
            bool IsTutorialRespawnEnabled() const { return tutorialRespawnEnabled_; }
            /** チュートリアルの全敵が倒されたか（スポーン後にカウントが0になった時 true） */
            bool IsTutorialAllEnemiesDefeated() const;
            /** チュートリアル中の現在の生存敵数を返す */
            int GetTutorialActiveEnemyCount() const;

        private:
            /**
             * 毒雲エントリ
             * unique_ptr<Vector3> で座標を安定させ PlayEffectFollow に渡す
             */
            struct PoisonCloud
            {
                Vector3 position;
                float remainingTime    = 10.0f;
                float tickTimer        = 0.0f;
                float sporeElapsedTime = 0.0f;                     // spore2 開始からの経過時間
                EffectHandle sporeHandle = INVALID_EFFECT_HANDLE;  // 胞子エフェクトのハンドル
                EffectHandle smokeHandle = INVALID_EFFECT_HANDLE;  // 持続煙エフェクトのハンドル
                bool smokeStarted = false;                         // smoke2 を開始済みか
                app::actor::MushroomEventCharacter* mushroomOwner = nullptr;  // 詠唱したキノコ

                static constexpr float RADIUS               = 67.0f;
                static constexpr float EFFECT_Y_OFFSET      = 50.0f;   // エフェクト表示をキノコの頭付近に上げるオフセット
                static constexpr float TICK_INTERVAL        = 1.0f;    // ダメージ間隔(秒)
                static constexpr float DURATION             = 10.0f;   // 持続時間(秒)
                static constexpr float DAMAGE_PER_TICK_RATE = 0.03f;   // 最大HP の 3%
                static constexpr int   MAX_COUNT            = 2;       // 同時最大設置数
                // spore2 開始から smoke2 を再生するまでの秒数
                // poison_spore2.efk の1回目パーティクル消失タイミングに合わせて調整する
                static constexpr float SMOKE_START_DELAY    = 0.8f;
            };
            std::deque<PoisonCloud> activePoisonClouds_;

            void PlacePoisonCloud(app::actor::MushroomEventCharacter* mushroom, const Vector3& position);
            void UpdatePoisonClouds();

        private:
            /** 攻撃者・防御者・チャージレベルからダメージ値を計算する */
            int CalcDamage(const app::actor::BattleCharacter* attacker,
                const app::actor::Character* defender, int chargeLevel = 0,
                bool* outIsCritical = nullptr) const;

        private:
            /** パラメータをファイルからロードする */
            void LoadParameter();
            /** TBDR用のスポーン時動的ライトを登録する */
            void TriggerTBDRSpawnLight(const Vector3& pos, const Vector3& color, float range, float duration);
            /** TBDR動的ライトの毎フレーム更新 */
            void UpdateTBDRSpawnLights();


            /**
             * シングルトン用
             */
        public:
            /**
             * インスタンスを作る
             */
            static void Initialize()
            {
                if (instance_ == nullptr)
                {
                    instance_ = new BattleManager();
                }
            }


            /**
             * インスタンスを取得
             */
            static BattleManager& Get()
            {
                return *instance_;
            }


            /**
             * インスタンスが有効か
             */
            static bool IsAvailable()
            {
                return instance_ != nullptr;
            }


            /**
             * インスタンスを破棄
             */
            static void Finalize()
            {
                if (instance_ != nullptr)
                {
                    delete instance_;
                    instance_ = nullptr;
                }
            }

        private:
            /** シングルトンインスタンス */
            static BattleManager* instance_;
        };
    }
}
