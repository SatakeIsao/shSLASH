#include "stdafx.h"
#include "PauseMenu.h"
#include "battle/BattleManager.h"
#include "core/ParameterManager.h"
#include "sound/SoundManager.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h"

namespace app
{
	namespace ui
	{
		PauseMenu::PauseMenu()
		{
			app::core::ParameterManager::Get().LoadParameter<app::core::MasterPauseMenuParameter>("Assets/master/PauseMenuParameter.json", [](const nlohmann::json& j, app::core::MasterPauseMenuParameter& p)
				{
					p.cursolPositionX[0] = j["cursolPositionXA"];
					p.cursolPositionX[1] = j["cursolPositionXB"];
					p.cursolPositionY[0] = j["cursolPositionYA"];
					p.cursolPositionY[1] = j["cursolPositionYB"];
				});
		}

		PauseMenu::~PauseMenu()
		{
			app::core::ParameterManager::Get().UnloadParameter<app::core::MasterPauseMenuParameter>();
		}

		void PauseMenu::Update()
		{
			MenuBase::Update();
		}

		void PauseMenu::OnOpen()
		{
			auto* canvas = GetCanvas();
			if (canvas) {
				canvas->isDraw = true;
			}
		}

		void PauseMenu::OnClose()
		{
			auto* canvas = GetCanvas();
			if (canvas) {
				canvas->isDraw = false;
			}
		}

		bool PauseMenu::IsPause()
		{
			auto* canvas = GetCanvas();
			if (canvas) {
				auto* anim = canvas->FindAnimation(Hash32("SlideOut_Pause_Exit"));
				if (anim && anim->IsPlay()) {
					return true;
				}
			}
			return false;
		}

		void PauseMenu::InitializeLogic()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				canvas->transform.localScale = Vector3::One;
			}
		}
	}
}