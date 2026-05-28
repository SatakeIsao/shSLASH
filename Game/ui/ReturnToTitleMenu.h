#pragma once
#include "ui/Menu.h"

namespace app
{
	namespace ui
	{
		class ReturnToTitleMenu : public MenuBase
		{
		private:

			UIIcon* cursol_ = nullptr;
			int currentIndex_ = 0;
			const int maxIndex_ = 1;

			bool isReturnTitleDecided_ = false;
			bool isRetryDecided_ = false;

		public:
			ReturnToTitleMenu();
			virtual ~ReturnToTitleMenu();

			void InitializeLogic() override;
			void Update() override;
			
			void SetDraw(bool isDraw);

			// マネージャーからフラグを確認するための関数
			bool IsReturnTitleDecided() const { return isReturnTitleDecided_; }
			bool IsRetryDecided() const { return isRetryDecided_; }
			void ResetFlags() { isReturnTitleDecided_ = false; isRetryDecided_ = false; }
		};
	}
}


