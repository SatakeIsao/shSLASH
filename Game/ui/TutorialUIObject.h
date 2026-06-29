#pragma once
#include "Layout.h"
#include "TutorialMessageMenu.h"

namespace app
{
    namespace ui
    {
        class TutorialUIObject : public IGameObject
        {
        public:
            TutorialUIObject();
            ~TutorialUIObject() override = default;

            void Update() override;
            void Render(RenderContext& rc) override;

            bool IsAllMessagesShown()     const;
            bool IsPlayerInputAllowed()   const;
            bool IsPracticePhaseReached() const;
            int  GetCurrentMessageIndex() const;
            void SetTimerProgress(float elapsed);
            void TriggerFinalClear();
            bool IsFinalClearDone()       const;

        private:
            std::unique_ptr<Layout> layout_;
            bool isVisible_ = false;
        };
    }
}
