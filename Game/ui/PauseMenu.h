#pragma once
#include "Menu.h"
#include "Layout.h"

namespace app
{
	namespace ui
	{
		class PauseMenu : public MenuBase
		{
		private:
			std::unique_ptr <app::ui::Layout> layout_;
			int cursolIndex_ = 0;

		public:
			PauseMenu();
			virtual ~PauseMenu();
			void Update() override;

			void OnOpen();
			void OnClose();

			int GerCurrentIndex() const
			{
				return cursolIndex_;
			}
		public:
			virtual void InitializeLogic();

		public:
			bool IsPause();
		};
	}
}