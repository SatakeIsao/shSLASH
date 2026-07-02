#pragma once
#include "ui/Layout.h"

namespace app
{
    namespace ui
    {
        class TutorialConfirmObject : public IGameObject
        {
        public:
            TutorialConfirmObject() = default;
            ~TutorialConfirmObject() override = default;

            void Render(RenderContext& rc) override;

            void Activate(app::ui::Layout* layout);
            void Deactivate();

        private:
            app::ui::Layout* layout_ = nullptr;
            bool isActive_ = false;
        };
    }
}
