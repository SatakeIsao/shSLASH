#pragma once
#include "ui/Menu.h"

namespace app
{
	namespace ui
	{
		class ReturnToTitleMenu : public MenuBase
		{
		private:
			int cursolIndex_ = 0;

			bool isPause_ = false;
			bool isDecidedYes_ = false;
			bool isDecidedNo_ = false;

		public:
			ReturnToTitleMenu();
			virtual ~ReturnToTitleMenu();
			void Update() override;

			void OnOpen();
			void OnClose();
			void PlaySelectedAnimation();

		public:
			virtual void InitializeLogic();


		public:
			bool IsPause()
			{
				return isPause_;
			}
			bool IsDecidedYes()
			{
				return isDecidedYes_;
			}
			bool IsDecidedNo()
			{
				return isDecidedNo_;
			}
		};
	}
}


