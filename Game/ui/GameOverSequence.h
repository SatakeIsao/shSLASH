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
                Finished    
            };

        private:
            std::unique_ptr<app::ui::Layout> layout_;
            SequenceState currentState_ = SequenceState::Standby;

            float delayTimer_ = 1.0f; 
            bool isReturnTitleDecided_ = false;

        public:
            GameOverSequence();
            ~GameOverSequence();

            void StartSequence(); // 倒れた瞬間に呼ぶ
            void Update() override;
            void Render(RenderContext& rc);

            bool IsReturnTitleDecided() const { return isReturnTitleDecided_; }
        };
    }
}