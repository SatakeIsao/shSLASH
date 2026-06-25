/**
 * BattleManager.h
 * バトル管理
 */
#pragma once
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
				app::collision::GhostBody* a = nullptr;
				app::collision::GhostBody* b = nullptr;
                //
                virtual uint32_t ID() const = 0;
            };


            /** ダメージ通知処理 */
            struct DamageNotify : public INotify
            {
                enum class EnemyType
                {
                    Stone,
                    Mushroom,
                };

                // 攻撃者
                app::actor::BattleCharacter* attacker = nullptr;
                // 被攻撃者
                app::actor::Character* defender = nullptr;
                // ノックバック方向
                Vector3 knockBackDirection;
                // 溜め攻撃による吹き飛ばしか
                bool isBlowBack = false;
                // 溜め攻撃レベル (0=通常, 1/2/3=チャージレベル)
                int chargeLevel = 0;
                // コンボの何発目か (0=1発目, 1=2発目, 2=3発目)
                int comboIndex = 0;
                
                EnemyType enemyType = EnemyType::Stone;

                // 比較用（クラス名から呼ぶ用）
                static uint32_t StaticID() { return 1; }
                // 仮想関数版（インスタンス経由で呼ぶ用）
                virtual uint32_t ID() const override { return StaticID(); }
            };




        private:
            // @todo for test
            app::actor::BattleCharacter* battleCharacter_ = nullptr;
            app::actor::EventCharacter* eventCharacter_ = nullptr;
            std::vector<app::actor::StoneEventCharacter*> stoneEventCharacters_;
            std::vector<app::actor::MushroomEventCharacter*> mushroomEventCharacters_;
			std::vector<app::actor::StaticGimmick*> testGimmickList_;
            std::vector<app::actor::PipeGimmick*> pipeGimmickList_;
            
            std::unique_ptr<app::actor::CharacterSteering> characterSteering_ = nullptr;
			std::unique_ptr<app::camera::CameraSteering> cameraSteering_ = nullptr;
            app::actor::EventCharacterSpawnManagerObject* eventCharacterSpawnManagerObject_ = nullptr;
			app::camera::RefCameraController gameCameraController_ = nullptr;

            EffectManagerObject* effectManagerObject_ = nullptr;
            EffectManager2DObject* effectManager2DObject_ = nullptr;
            app::core::PauseManagerObject* pauseManagerObject_ = nullptr;
            app::ui::BattleSequence* battleSequenceObject_ = nullptr;
            app::ui::TimerUIObject* timerUIObject_ = nullptr;
            app::ui::PlayerHpUIObject* playerHpUIObject_ = nullptr;
            app::ui::EnemyHpUIObject* enemyHpUIObject_ = nullptr;
            app::ui::LevelUpUIObject* levelUpObject_ = nullptr;
            app::ui::DamagePopPool* damagePopPool_ = nullptr;
            /** ダメージポップ通知先 */
            IDamagePopListener* damagePopListener_ = nullptr;
            /** フェーズを表示できる様に仮置き */
			app::actor::PhaseUI* phaseUI_ = nullptr;
            /** スカイキューブのオブジェクト */
            nsK2EngineLow::SkyCube* skyCube_ = nullptr;
            /** 月のオブジェクト */
            app::actor::MoonGimmick* moon_ = nullptr;
            /** 通知リスト */
			std::vector<std::unique_ptr<INotify>> notifyList_;

            int stoneKillCount_ = 0;
            int mushroomKillCount_ = 0;

            bool hasPlayedPunchEffect_ = false;
            bool deadTest_ = false;
            bool isPause_ = false;
            bool isTutorialMode_ = false;
            bool gameOverFreeze_ = false;
            bool tutorialEnemyMoveEnabled_ = false;
            bool tutorialNeedsSpawn_        = false;
            bool tutorialRespawnEnabled_    = false;
            Vector3 tutorialStoneSpawnPos_    = Vector3::Zero;
            Vector3 tutorialMushroomSpawnPos_ = Vector3::Zero;
            Quaternion tutorialEnemySpawnRot_;
            bool playerInputEnabled_          = true;
            bool suppressChargeAttackInput_   = false;
            bool comboAttackJustCompleted_    = false;
            bool chargeAttackJustCompleted_   = false;
            bool guardJustSucceeded_          = false;
            float guardSuccessCooldown_       = 0.0f;
            bool avoidJustSucceeded_          = false;
            bool tutorialFreeze_              = false;
            bool tutorialNoDamage_            = false;
            bool tutorialLevelUpNotified_     = false;
            bool timeUpTriggered_ = false;
            int  lastCountShown_  = -1;

            int timeUpFreezeFrames_ = 0;
            static constexpr int kTimeUpFreezeFrameCount = 10;
            int loadStep_ = 0;

            /** アニメーション初期化を待ってからシーケンスを起動するための遅延 */
            float battleSequenceStartTimer_ = 0.1f;

            float countDownTimer_ = 3.0f;
            /** 遅延時間をカウント */
            float effectDelayTimer_ = 0.0f;
            /** 残り時間 */
            float remainTime_ = 120.0f;
            /** 無敵時間をカウント */
            float invincibleTimer_ = 0.0f;
            /** 無敵になったか */
            bool isInvincible_ = false;

            bool isWaitEffectPlay_ = false;
            /** 再生予定の位置を保持 */
            Vector3 reservedEffectPos_;
            /** 攻撃開始時の向きを固定保持 */
            Vector3 reservedEffectDir_;
            Quaternion reservedEffectRot_;
            std::unique_ptr<app::ui::Layout> layout_;

            // スポーンエフェクト遅延再生エントリ
            struct PendingSpawnEffect
            {
                int effectKind = 0;
                Vector3 scale = Vector3::Zero;
                float timer = 0.0f;
            };
            std::vector<PendingSpawnEffect> pendingSpawnEffects_;
            Vector3 playerSpawnEffectPos_ = Vector3::Zero;
            float pendingPlayerSpawnLightTimer_ = -1.0f;
            /** 松明用のライト有無 */
            bool torchLightsEnabled_ = false;


        private:
            BattleManager();
            ~BattleManager();


        public:
            /** 初期化（同期版。DebugScene用） */
            void Start();
            /** 分割ロード。falseを返す間は毎フレーム呼ぶ。trueで完了。 */
            bool LoadStep();
            /** 更新処理 */
            void Update();
            /** 描画処理 */
            void Render(RenderContext& rc);


            void AddNotify(INotify* notify)
            {
                notifyList_.push_back(std::move(std::unique_ptr<INotify>(notify)));
			}
            /**
            * ダメージポップリスナーを登録
            * BattleManager はリスナーの所有権を持たない
            */
            void SetDamagePopListener(IDamagePopListener* listener)
            {
                damagePopListener_ = listener;
            }
            /** プレイヤーへのダメージ通知 */
            struct PlayerDamageNotify : public INotify
            {
                // 攻撃してきた敵
                app::actor::Character* attacker = nullptr;

                static uint32_t StaticID() { return 2; }
                virtual uint32_t ID() const override { return StaticID(); }
            };

            /** DEBUG:あとで書き換える */
            bool GetDeadTest()
            {
                return deadTest_;
            }

            bool IsPlayerDead() const;
            void SetGameOverFreeze(bool v);
            void SetHpBarPreBlurRender(bool v);


            void SetTorchLightsEnabled(bool enabled) { torchLightsEnabled_ = enabled; }
            bool IsTorchLightsEnabled() const { return torchLightsEnabled_; }
            void SetPause(bool isPause);
            void SetLevelUpUIObject(app::ui::LevelUpUIObject* obj) { levelUpObject_ = obj; }
            bool IsTimeUpFinished() const;
            /** ジャスト回避などでプレイヤーの経験値ゲージを加算する */
            void AddPlayerGauge(int amount);

            /** チュートリアルモードを有効にする（Start()の前に呼ぶこと） */
            void SetTutorialMode(bool isTutorial) { isTutorialMode_ = isTutorial; }
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
            bool CheckAndConsumeComboAttackCompleted()
            {
                if (!comboAttackJustCompleted_) return false;
                comboAttackJustCompleted_ = false;
                return true;
            }
            /** チュートリアル用: 溜め攻撃完了通知 */
            void NotifyChargeAttackCompleted() { chargeAttackJustCompleted_ = true; }
            bool CheckAndConsumeChargeAttackCompleted()
            {
                if (!chargeAttackJustCompleted_) return false;
                chargeAttackJustCompleted_ = false;
                return true;
            }
            /** チュートリアル用: ガード成功通知 */
            void NotifyGuardSucceeded() { guardJustSucceeded_ = true; }
            bool CheckAndConsumeGuardSucceeded()
            {
                if (!guardJustSucceeded_) return false;
                guardJustSucceeded_ = false;
                return true;
            }
            /** チュートリアル用: 回避実行通知 */
            void NotifyAvoidSucceeded() { avoidJustSucceeded_ = true; }
            bool CheckAndConsumeAvoidSucceeded()
            {
                if (!avoidJustSucceeded_) return false;
                avoidJustSucceeded_ = false;
                return true;
            }
            /** チュートリアル用: レベルアップ通知 */
            void NotifyTutorialLevelUp() { tutorialLevelUpNotified_ = true; }
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
            int CalcDamage(const app::actor::BattleCharacter* attacker,
                const app::actor::Character* defender, int chargeLevel = 0,
                bool* outIsCritical = nullptr) const;

        private:
            void LoadParameter();
            void TriggerTBDRSpawnLight(const Vector3& pos, const Vector3& color, float range, float duration);
            void UpdateTBDRSpawnLights();

            struct TBDRSpawnLightEntry
            {
                Vector3 position;
                Vector3 peakColor;
                float   range    = 0.f;
                float   timer    = 0.f;
                float   duration = 1.f;
            };
            std::vector<TBDRSpawnLightEntry> tbdrSpawnLights_;




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