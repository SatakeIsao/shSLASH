#pragma once
#include "Menu.h"
#include "Layout.h"
#include "json/json.hpp"

namespace app
{
    namespace ui
    {
        class UIText;
        class UIIcon;

        class TutorialMessageMenu : public MenuBase
        {
        private:
            UIIcon* messageWindowIcon_         = nullptr;
            UIIcon* messageWindowPracticeIcon_ = nullptr;
            UIText* titleText_                  = nullptr;
            UIIcon* titleLineIcon_              = nullptr;
            UIIcon* titleLinePracticeIcon_      = nullptr;
            UIText* messageText_       = nullptr;
            UIText* timerText_         = nullptr;
            UIIcon* nextButtonIcon_    = nullptr;
            UIIcon* nextButtonAltIcon_ = nullptr;
            UIText* nextHintText_      = nullptr;

            bool skipInput_ = true;

            // CLEAR アニメーション
            UIIcon* clearBannerIcon_      = nullptr;
            UIIcon* clearTextIcon_        = nullptr;
            Vector3 clearBannerBaseScale_ = Vector3(1.0f, 1.0f, 1.0f);
            Vector3 clearTextBaseScale_   = Vector3(1.0f, 1.0f, 1.0f);

            enum class ClearAnimState { None, ZoomIn, Hold, ZoomOut, FinalSlideOut };
            ClearAnimState clearAnimState_  = ClearAnimState::None;
            float          clearAnimTimer_  = 0.0f;
            bool           clearPendingAdv_ = false;
            bool           finalClearDone_  = false;

            static constexpr float kClearZoomInDur  = 0.175f;
            static constexpr float kClearHoldDur    = 0.8f;
            static constexpr float kClearZoomOutDur = 0.3f;
            static constexpr float kClearFadeOutDur = 0.1f;
            static constexpr float kClearScaleMax   = 2.0f;

            struct MessageEntry
            {
                std::wstring title;
                bool         hasTitlePosition = false;
                Vector3      titlePosition;
                bool         hasTitleScale    = false;
                Vector3      titleScale;

                std::wstring body;
                Vector3      bodyPosition;
                Vector3      bodyScale;
                Vector4      bodyColor;

                bool         hasTimer         = false;
                float        timerRequired    = 0.0f;
                bool         hasTimerPosition = false;
                Vector3      timerPosition;
                bool         hasTimerScale    = false;
                Vector3      timerScale;

                bool         hasCounter          = false;
                int          counterRequired     = 0;
                std::string  counterButton       = "B";
                std::string  counterUnit         = "";
                bool         hasCounterPosition  = false;
                Vector3      counterPosition;
                bool         hasCounterScale     = false;
                Vector3      counterScale;

                bool         hasEnemyCounter        = false;
                int          enemyCounterTotal       = 2;
                std::string  enemyCounterUnit        = "";
                bool         hasEnemyCounterPosition = false;
                Vector3      enemyCounterPosition;
                bool         hasEnemyCounterScale    = false;
                Vector3      enemyCounterScale;

                bool         enableEnemyMove     = false;
                bool         startPracticePhase  = false;
                bool         freezeGame          = false;
                bool         nextButtonHint         = false;
                std::string  nextIconName;
                bool         hasNextIconPosition    = false;
                Vector3      nextIconPosition;
                bool         hasNextIconScale       = false;
                Vector3      nextIconScale;
                bool         hasNextTextPosition    = false;
                Vector3      nextTextPosition;
                bool         hasNextTextScale       = false;
                Vector3      nextTextScale;
                bool         hideNextText           = false;

                struct ImageEntry
                {
                    std::string name;
                    bool        hasPosition = false;
                    Vector3     position;
                    bool        hasScale    = false;
                    Vector3     scale;
                };
                std::vector<ImageEntry> images;

                bool    hasWindowPosition      = false;
                Vector3 windowPosition;

                bool    hideWindow          = false;
                bool    showLvPopupAfter   = false;
                bool    blurBackground     = false;

                struct LvPopupImageEntry {
                    std::string name;
                    bool        hasPosition = false;
                    Vector3     position;
                    bool        hasScale    = false;
                    Vector3     scale;

                    struct OverlayEntry {
                        std::string name;
                        bool        hasPosition = false;
                        Vector3     position;
                        bool        hasScale    = false;
                        Vector3     scale;
                    };
                    std::vector<OverlayEntry> overlays;
                };
                std::vector<LvPopupImageEntry> lvPopupImages;

                bool    hasLvPopupAnim       = false;
                float   lvPopupAnimScale     = 1.625f;
                float   lvPopupAnimBaseScale = 1.3f;
                float   lvPopupAnimInDur     = 0.2f;
                bool    hasLvPopupAnimCenter = false;
                Vector3 lvPopupAnimCenter;

                bool    hasWindowAnim       = false;
                float   windowAnimScale     = 1.5f;
                float   windowAnimBaseScale = 1.0f;
                float   windowAnimInDur     = 0.4f;
                bool    hasWindowAnimCenter = false;
                Vector3 windowAnimCenter;
            };
            std::vector<MessageEntry> messages_;
            int  currentIndex_         = 0;
            bool isAllShown_           = false;
            bool practicePhaseReached_ = false;

            static int s_retryIndex_;

            float   timerElapsed_      = 0.0f;
            float   timerResetGrace_  = 0.0f;
            int     counterElapsed_   = 0;

            // layout-level defaults (updated by messageWindowTitle/Timer in JSON)
            Vector3 defaultTitlePosition_ = Vector3(-200.0f, -110.0f, 0.0f);
            Vector3 defaultTitleScale_    = Vector3(1.0f, 1.0f, 1.0f);
            Vector3 defaultTimerPosition_ = Vector3(-60.0f, -370.0f, 0.0f);
            Vector3 defaultTimerScale_    = Vector3(0.65f, 0.65f, 1.0f);

            std::vector<UIIcon*> currentTutorialImages_;

            // ウィンドウポップアニメーション
            enum class WinAnim { None, In, Hold, Out };
            WinAnim winAnim_          = WinAnim::None;
            float   winAnimTimer_     = 0.0f;
            float   winAnimScale_     = 1.5f;
            float   winAnimBaseScale_ = 1.0f;
            float   winAnimInDur_     = 0.4f;
            Vector3 winAnimCenter_;

            struct WinElemSnap {
                UIIcon* icon      = nullptr;
                UIText* text      = nullptr;
                Vector3 origPos;
                Vector3 targetPos;
                Vector3 baseScale;
            };
            std::vector<WinElemSnap> winElems_;

            // ホットリロード時の累積スケール・位置防止用: 各要素のレイアウト由来スケールと位置を保持
            struct WinOrigScale {
                UIIcon* icon  = nullptr;
                UIText* text  = nullptr;
                Vector3 scale;
                Vector3 pos;
            };
            std::vector<WinOrigScale> winOrigScales_;

            // スライド+フェードアニメーション
            struct SlideElemSnap {
                UIIcon* icon      = nullptr;
                UIText* text      = nullptr;
                Vector3 basePos;
                float   baseAlpha = 1.0f;
            };
            std::vector<SlideElemSnap> slideElems_;

            enum class SlideAnim { None, In, Out };
            SlideAnim slideAnim_      = SlideAnim::None;
            float     slideAnimTimer_ = 0.0f;

            static constexpr float kTimerResetGraceDur  = 0.3f;
            static constexpr float kSlideDur            = 0.1f;
            static constexpr float kSlideOffset         = 15.0f;
            static constexpr float kWinAnimInitialRatio  = 0.7f;

            // Level-up explanation popup (same animation as action_explain)
            UIIcon* levelExplainIcon_     = nullptr;
            UIIcon* phaseExplainIcon_     = nullptr;
            bool    lvPopupPendingAdvance_ = false;

            enum class LvPopupState { None, In, Hold };
            LvPopupState lvPopupState_        = LvPopupState::None;
            float        lvPopupTimer_        = 0.0f;
            int          lvPopupStep_         = 0;
            UIIcon*      lvPopupCurrentIcon_   = nullptr;
            float        lvPopupCurScale_       = 1.625f;
            float        lvPopupCurBaseScale_   = 1.3f;
            float        lvPopupCurInDur_       = 0.2f;
            Vector3      lvPopupCurCenter_      = Vector3(0.0f, 0.0f, 0.0f);

            struct LvPopupElemSnap {
                UIIcon* icon      = nullptr;
                Vector3 baseScale;
                Vector3 targetPos;
            };
            std::vector<LvPopupElemSnap> lvPopupElems_;

            static constexpr float kLvPopupInDur    = 0.2f;
            static constexpr float kLvPopupScaleMax  = 1.625f;
            static constexpr float kLvPopupBaseScale = 1.3f;

#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
            time_t messagesFileTime_ = 0;
#endif

        public:
            TutorialMessageMenu() = default;
            ~TutorialMessageMenu() = default;

            void InitializeLogic() override;
            void Update() override;

            bool IsAllMessagesShown()       const { return isAllShown_; }
            bool IsPracticePhaseReached()   const { return practicePhaseReached_; }
            bool IsPlayerInputAllowed() const
            {
                if (isAllShown_) return true;
                if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(messages_.size())) return false;
                const auto& msg = messages_[currentIndex_];
                return msg.hasTimer || msg.hasCounter || !msg.nextButtonHint;
            }

            void SetTimerProgress(float elapsed);
            void TriggerFinalClear();
            bool IsFinalClearDone()    const { return finalClearDone_; }
            int  GetCurrentMessageIndex() const { return currentIndex_; }

            static void SetRetryIndex(int index) { s_retryIndex_ = index; }

        private:
            void UpdateLogic();
            void ShowCurrentMessage();
            void AdvanceMessage();
            void ApplyLayoutOverrides(const nlohmann::json& layout);
            void UpdateTimerDisplay(float elapsed);
            void UpdateCounterDisplay(int current);
            void UpdateEnemyCounterDisplay(int defeated, int total, const std::string& unit);
            void StartClearAnimation(bool pendingAdvance);
            void UpdateClearAnimation(float dt);
            void ApplyClearScale(float factor, float alpha);
            void StartWindowAnim(const MessageEntry& entry);
            void ApplyWindowAnimFactor(float t);
            void RestoreWindowElems();
            void StartSlideAnim(bool isIn);
            void ApplySlideAnim(float t, bool isIn);
            void StartLvPopup();
            void UpdateLvPopup(float dt);
            void ApplyLvPopupFactor(float t);
        };
    }
}
