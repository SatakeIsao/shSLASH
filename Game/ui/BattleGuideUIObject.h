#pragma once
#include <memory>
#include "ui/Layout.h"

namespace app
{
	namespace ui
	{
		/** 画面右下に常時表示する操作ガイド */
		class BattleGuideUIObject : public IGameObject
		{
		private:
			std::unique_ptr<app::ui::Layout> layout_;

		public:
			BattleGuideUIObject();

			void Update();
			void Render(RenderContext& rc);
		};
	}
}
