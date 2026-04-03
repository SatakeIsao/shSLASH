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

			bool isPause_ = false;
			bool isVolume_ = false;
			

		public:
			SoundOptionMenu();
			virtual ~SoundOptionMenu();
			void Update() override;

			void OnOpen();
			void OnClose();


		public:
			virtual void InitializeLogic();


		public:
			bool IsPause()
			{
				return isPause_;
			}
		};
	}
}

