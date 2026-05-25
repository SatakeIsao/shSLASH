#include "stdafx.h"
#include "ResultSubMenu.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace app {
    namespace ui {
        ResultSubMenu::ResultSubMenu() {}
        ResultSubMenu::~ResultSubMenu() {}

        void ResultSubMenu::InitializeLogic() {
            // UIパーツの取得
            backFilter_ = GetUI<UIIcon>(Hash32("BackFilter"));
            menuLine1_ = GetUI<UIIcon>(Hash32("MenuLine"));
            menuLine2_ = GetUI<UIIcon>(Hash32("MenuLine_2"));
            menuBase_ = GetUI<UIIcon>(Hash32("ResultMenuBase"));
            retry_ = GetUI<UIIcon>(Hash32("Retry"));
            title_ = GetUI<UIIcon>(Hash32("TITLE"));
            exit_ = GetUI<UIIcon>(Hash32("Exit"));
            highlight_ = GetUI<UIIcon>(Hash32("Highlight"));

            // アニメーションをアタッチ（BackFilterとHighlight以外）
            UIIcon* targets[] = { menuLine1_, menuLine2_, menuBase_, retry_, title_, exit_ };
            for (auto* ui : targets) {
                if (ui) {
                    app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(ui, Hash32("FadeIn_ResultSubMenu"));
                }
            }
        }

        void ResultSubMenu::OnOpen() {
            // フェードインアニメーションの再生
            UIIcon* targets[] = { menuLine1_, menuLine2_, menuBase_, retry_, title_, exit_ };
            for (auto* ui : targets) {
                if (ui) {
                    ui->isDraw = true;

                    auto* anim = ui->FindAnimation(Hash32("FadeIn_ResultSubMenu"));
                    if (anim) anim->Play();
                }
            }
        }

        void ResultSubMenu::Update() {
            MenuBase::Update();
            // 今後、ここでカーソル移動の処理（上下キー入力など）を書いていきます
        }
    }
}