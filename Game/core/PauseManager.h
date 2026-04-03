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
    }
}

namespace app
{
    namespace core
    {
        class PauseManager
        {
        private:
            // ここにレイアウトをもって
            app::ui::Layout* layout_ = nullptr;

            bool isPause_ = false;
            /** Pauseフラグが切り替わった１フレームだけtrueになる */
            bool isPauseTrigger = false;
            /** ポーズメニューを開くことができるか */
            bool canPause_ = true;

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
            bool IsPauseTrigger() const { return isPauseTrigger; }

            /** 外部からポーズの許可/禁止を設定 */
            void SetCanPause(bool canPause) { canPause_ = canPause; }
            bool CanPause() const { return canPause_; }

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

