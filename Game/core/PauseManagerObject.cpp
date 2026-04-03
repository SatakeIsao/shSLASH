#include "stdafx.h"
#include "PauseManagerObject.h"
#include "PauseManager.h"

namespace app
{
	namespace core
	{
		PauseManagerObject::PauseManagerObject()
		{
		}

		PauseManagerObject::~PauseManagerObject()
		{
			PauseManager::Get().Finalize();
		}

		bool PauseManagerObject::Start()
		{
			PauseManager::Get().Initialize();

			return true;
		}

		void PauseManagerObject::Update()
		{
			PauseManager::Get().Update();
		}

		void PauseManagerObject::Render(RenderContext& rc)
		{
			PauseManager::Get().Render(rc);
		}
	}
}