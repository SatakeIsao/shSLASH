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

            bool hasPlayedPunchEffect_ = false;
            bool deadTest_ = false;
            bool isPause_ = false;

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


        private:
            BattleManager();
            ~BattleManager();


        public:
            /** 初期化 */
            void Start();
            /** 更新処理 */
            void Update();


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


            void SetPause(bool isPause);
            void SetLevelUpUIObject(app::ui::LevelUpUIObject* obj) { levelUpObject_ = obj; }
            bool IsTimeUpFinished() const;

        private:
            int CalcDamage(const app::actor::BattleCharacter* attacker,
                const app::actor::Character* defender, int chargeLevel = 0) const;

        private:
            void LoadParameter();




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