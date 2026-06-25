#pragma once
#include "ui/Layout.h"
#include "ui/UIParts.h"

namespace app {
    namespace ui {
        class GameOverSequence : public IGameObject {
            enum class SequenceState {
                Standby,
                Wait,
                FadeIn,
                Finished,
                FadeOut,
                ShowMenu
            };

        private:
            std::unique_ptr<app::ui::Layout> layout_;
            std::unique_ptr<app::ui::Layout> subMenuLayout_;
            SequenceState currentState_ = SequenceState::Standby;

            float delayTimer_ = 1.0f;
            float menuWaitTimer_ = 0.0f;
            float fadeOutTimer_ = 0.0f;
            static constexpr float kFadeOutDuration = 0.8f;

        public:
            GameOverSequence();
            ~GameOverSequence();

            void StartSequence();
            void Update() override;
            void Render(RenderContext& rc) override;

            bool IsReturnTitleDecided() const;
            bool IsRetryDecided() const;
            bool IsExitDecided() const;
        };
    }
}
