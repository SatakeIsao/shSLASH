#pragma once
#include <memory>
#include "ui/Layout.h"

namespace app
{
	namespace ui
	{
		class PhaseUI : public IGameObject
		{
		private:
			std::unique_ptr<app::ui::Layout> layout_;
			int currentPhase_ = 1;

		public:
			PhaseUI();

			void SetPhaseCount(int phaseCount);
			void Update();
			void Render(RenderContext& rc);
		};
	}
}
