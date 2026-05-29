/**
 * SoundOptionMenu.h
 * 音量調整をするクラス
 */
#pragma once
#include "Menu.h"

namespace app
{
	namespace ui
	{
		class SoundOptionMenu : public MenuBase
		{
		private:
			int volumeCursolIndex_ = 0;


		public:
			SoundOptionMenu();
			virtual ~SoundOptionMenu();
			void Update() override;
			void SetDraw(bool isDraw);


		public:
			virtual void InitializeLogic();
		};
	}
}

