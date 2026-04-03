#include "stdafx.h"
#include "HPBar.h"
#include "ui/Layout.h"

namespace
{
	//static app::ui::Layout* hpLayout;
	static const int MAX_HP = 8;
}

HPBarObject::HPBarObject()
{
	layout_ = std::make_unique<app::ui::Layout>();
	layout_->Initialize <app::ui::MenuBase>("Assets/ui/layout/hpLayout.json");
}

HPBarObject::~HPBarObject()
{
}

void HPBarObject::Update()
{
	/** インデックス数に応じて表示/非表示切り替え */
	if (layout_) {
		auto* menu = layout_->GetMenu();
		if (menu) {
			/** 青色 */
			Vector3 targetColor = Vector3(0.0f, 0.569f, 1.0f);
			if (currentHP_ < 3) {
				/** 緑色 */
				targetColor = Vector3(1.0f, 0.0f, 0.0f);
			}
			else if (currentHP_ < 5) {
				/** 黄色 */
				targetColor = Vector3(1.0f, 1.0f, 0.0f);
			}
			else if (currentHP_ < 7) {
				/** 赤色 */
				targetColor = Vector3(0.0f, 1.0f, 0.0f);
			}

			for (int i = 1; i <= MAX_HP; i++) {
				/** 文字列を作成 */
				std::string name = "HPBar_" + std::to_string(i);

				/** Hash32でキー化してUIを取得 */
				uint32_t key = Hash32(name.c_str());
				/** UIIconとして取得 */
				auto* barIcon = menu->GetUI<app::ui::UIIcon>(key);

				if (barIcon) {

					/** 現在のHP以下なら表示、それより大きければ非表示 */
					bool isVisible = (i <= currentHP_);

					/** 色をイージングで変える場合は以下のように */
					float targetAlpha = isVisible ? 1.0f : 0.15f;
					float currentAlpha = barIcon->color.w;
					float newAlpha = currentAlpha + (targetAlpha - currentAlpha) * 0.1f;
					barIcon->color.w = newAlpha;

					barIcon->color.x = targetColor.x;
					barIcon->color.y = targetColor.y;
					barIcon->color.z = targetColor.z;
				}
			}
		}
		layout_->Update();
	}
}

void HPBarObject::Render(RenderContext& rc)
{
	if (layout_) {
		layout_->Render(rc);
	}
}