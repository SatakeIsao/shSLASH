#pragma once
#include "Layout.h"

namespace app
{
    namespace ui
    {
        class TutorialPauseHintObject : public IGameObject
        {
        public:
            TutorialPauseHintObject() = default;
            ~TutorialPauseHintObject() override = default;

            bool Start() override;
            void Update() override;
            void Render(RenderContext& rc) override;

        private:
            std::unique_ptr<Layout> layout_;
        };
    }
}
