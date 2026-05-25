#pragma once
#include "ui/Menu.h"
#include "ui/Layout.h"
namespace app {
    namespace ui {
        class UIIcon;
        class UIDigit;

        class ResultMenu : public MenuBase {
        public:
            enum class SequenceState {
                Init,
                ShowResult,
                SlideInScoreBoardAndResult,
                ShowDigits,
                ShowRank,
                Finished,
                OpenMenu
            };

            ResultMenu();
            virtual ~ResultMenu();

            void InitializeLogic() override;
            void Update() override;
            void Render(RenderContext& rc) override;

            // アニメーション用に追加しておく
            void OnOpen();
            void OnClose();

        private:
            SequenceState currentState_ = SequenceState::Init;
            float stateTimer_ = 0.0f;   // 待ち時間を計るタイマー
            int currentDigitStep_ = 0;  // 数字を出す順番を数える変数
			std::unique_ptr<app::ui::Layout> subMenuLayout_; 

            // UIのポインタを保持しておく変数
            UIIcon* scoreBoard_ = nullptr;
            UIIcon* enemyIcon1_ = nullptr;
            UIIcon* enemyIcon2_ = nullptr;
            UIIcon* enemyIcon3_ = nullptr;
            UIDigit* levelDigit_ = nullptr;
            UIDigit* enemyDigit1_ = nullptr;
            UIDigit* enemyDigit2_ = nullptr;
            UIDigit* enemyDigit3_ = nullptr;
            UIDigit* scoreDigit_ = nullptr;
            UIIcon* rankS_ = nullptr;
            UIIcon* rankSFog_ = nullptr;
            UIIcon* rankA_ = nullptr;
            UIIcon* rankB_ = nullptr;
            UIIcon* skipIcon_ = nullptr;
            UIIcon* nextIcon_ = nullptr;

            float sFogTimer_ = 0.0f;
            int debugCurrentRank_ = -1;
            float bRankTimer_ = 0.0f;
            bool isBRankTilted_ = false;
        };
    }
}