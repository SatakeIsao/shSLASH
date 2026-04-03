/**
 * BattleManager.h
 * バトル管理
 */
#pragma once
#include "camera/CameraCommon.h"
#include "camera/CameraSteering.h"
#include "ui/Layout.h"
#include "effect/EffectManager.h"

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


            struct DamageNotify : public INotify
            {
				virtual uint32_t ID() const override { return 1; }
            };




        private:
            // @todo for test
            app::actor::BattleCharacter* battleCharacter_ = nullptr;
            app::actor::EventCharacter* eventCharacter_ = nullptr;
			std::vector<app::actor::StaticGimmick*> testGimmickList_;
            std::vector<app::actor::PipeGimmick*> pipeGimmickList_;
            
            std::unique_ptr<app::actor::CharacterSteering> characterSteering_ = nullptr;
			std::unique_ptr<app::camera::CameraSteering> cameraSteering_ = nullptr;
			app::camera::RefCameraController gameCameraController_ = nullptr;

            EffectManagerObject* effectManagerObject_ = nullptr;
            app::core::PauseManagerObject* pauseManagerObject_ = nullptr;
            app::ui::BattleSequence* battleSequenceObject_ = nullptr;
            nsK2EngineLow::SkyCube* skyCube_ = nullptr;									//スカイキューブのオブジェクト
            /** 通知リスト */
			std::vector<std::unique_ptr<INotify>> notifyList_;

            bool hasPlayedPunchEffect_ = false;
            bool deadTest_ = false;
            bool isPause_ = false;

            /** あとで書き換える */
            //Test currentDown = Test::Compleate;
            float countDownTimer_ = 3.0f;
            std::unique_ptr<app::ui::Layout> layout_;


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

            /** DEBUG:あとで書き換える */
            bool GetDeadTest()
            {
                return deadTest_;
            }


            void SetPause(bool isPause);

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


        //class IPauseMenu : Noncopyable
        //{
        //public:
        //    IPauseMenu() {}
        //    virtual ~IPauseMenu() {}
        //
        //    virtual bool Start() = 0;
        //    virtual void Update() = 0;
        //    virtual void Render(RenderContext& rc) = 0;
        //    virtual void CanChange(int& request) = 0;
        //};
        //
        //
        //
        ///** ポーズメニュー表示 */
        //class BattlePauseMenu : IPauseMenu
        //{
        //public:
        //    enum EnPauseMenuType
        //    {
        //        enPauseMenuType_ReGame,
        //        enPauseMenuType_Volume,
        //
        //    };
        //};
    }
}