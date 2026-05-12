#include "stdafx.h"
#include "TitleMenuManagerObject.h"
#include "TitleMenuManager.h"

namespace app
{
	namespace core
	{
		TitleMenuManagerObject::TitleMenuManagerObject()
		{
		}

		TitleMenuManagerObject::~TitleMenuManagerObject()
		{
			TitleMenuManager::Get().Finalize();
		}

		bool TitleMenuManagerObject::Start()
		{
			TitleMenuManager::Get().Initialize();

			return true;
		}

		void TitleMenuManagerObject::Update()
		{
			TitleMenuManager::Get().Update();
		}

		void TitleMenuManagerObject::Render(RenderContext& rc)
		{
			TitleMenuManager::Get().Render(rc);
		}
	}
}