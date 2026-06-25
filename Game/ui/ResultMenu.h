#pragma once
#include "ui/Menu.h"
#include "ui/Layout.h"
namespace app {
    namespace ui {
        class UIIcon;
        class UINumberSprite;

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

            /** デバッグ用: シーケンスとUIを先頭から再生しなおす */
            void Reset(int newRank);
            /** デバッグ用: 数値を直接セットして表示 */
            void DebugSetNumber(int value);

            bool IsReturnTitleDecided() const;
            bool IsRetryDecided() const;
            bool IsExitDecided() const;

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
            UINumberSprite* levelDigit_ = nullptr;
            UINumberSprite* enemyDigit1_ = nullptr;
            UINumberSprite* enemyDigit2_ = nullptr;
            UINumberSprite* enemyDigit3_ = nullptr;
            UINumberSprite* scoreDigit_ = nullptr;
            UIIcon* rankMaster_ = nullptr;
            UIIcon* rankMasterFog_ = nullptr;
            UIIcon* rankElite_ = nullptr;
            UIIcon* rankBeginner_ = nullptr;
            UIIcon* resultIcon_ = nullptr;
            UIIcon* skipIcon_ = nullptr;
            UIIcon* nextIcon_ = nullptr;

            float masterFogTimer_ = 0.0f;
            int currentRank_ = -1;   // 0=MASTER, 1=ELITE, 2=BEGINNER
            float beginnerRankTimer_ = 0.0f;
            bool isBeginnerRankTilted_ = false;

            float shakeTimer_ = 0.0f;           // 揺れている時間を計る
            int shakingDigitIndex_ = -1;        // 今どの数字が揺れているか
            Vector3 initialDigitPositions_[5];  // 5つの数字の「元の座標」を記憶
            Vector3 initialBeginnerRankPos_;    // Beginner ランクアイコンの初期座標
            bool rankSEPlayed_ = false;         // ランクSEを再生済みかどうか

        };
    }
}