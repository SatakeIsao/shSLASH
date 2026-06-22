/**
 * PauseManager.h
 * ポーズマネージャー
 */
#pragma once
namespace app
{
    namespace ui
    {
        class Layout;
        class ReturnToTitleMenu;
    }
}

namespace app
{
    namespace core
    {
        enum class PauseState
        {
            enClosed,
            enPause,
            enOption,
        };

        class PauseManager
        {
        private:
            // ここにレイアウトをもって
            app::ui::Layout* layout_ = nullptr;
            app::ui::Layout* pauseLayout_ = nullptr;

            PauseState currentState_ = PauseState::enClosed;
            /** 遷移先のステートを予約 */
            PauseState nextState_ = PauseState::enClosed;

            /** アニメーション再生中か */
            bool isPlayingAnimation_ = false;
            /** タイトル */
            bool isTest_ = false;

            /** 下記三つのフラグは消去 */
            bool isPause_ = false;
            bool isTitle_ = false;
            bool isVolume_ = false;
            /** ポーズメニューを開くことができるか */
            bool canPause_ = true;
            /** オプション画面を開くことができるか */
            bool canOpenOption_ = true;
            /** タイトルに戻るリクエストが確定したか（ラッチ式） */
            bool returnToTitlePending_ = false;
            /** リトライリクエストが確定したか（ラッチ式） */
            bool retryPending_ = false;

        private:
            PauseManager();
            ~PauseManager();

        public:
            /** 更新処理 */
            void Update();
            void Render(RenderContext& rc);


        private:
            void CreateMenu();

        public:
            bool IsPause() const { return isPause_; }
            bool IsTitle() const { return isTitle_; }
            bool IsValume() const { return isVolume_; }
            //bool IsPauseTrigger() const { return isPauseTrigger; }

            /** 外部からポーズの許可/禁止を設定 */
            void SetCanPause(bool canPause) { canPause_ = canPause; }
            bool CanPause() const { return canPause_; }

            /** 外部からオプション画面への遷移の許可/禁止を設定 */
            void SetCanOpenOption(bool can) { canOpenOption_ = can; }

            bool IsReturnToTitleRequested() const;
            bool IsRetryRequested() const;

            /** ポーズメニュー画面（設定画面ではない）が開いているか */
            bool IsInPauseMenu() const;

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
                    instance_ = new PauseManager();
                }
            }


            /**
             * インスタンスを取得
             */
            static PauseManager& Get()
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
            static PauseManager* instance_;
        };
    }
}