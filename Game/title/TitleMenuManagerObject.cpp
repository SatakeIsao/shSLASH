#include "stdafx.h"
#include "title/TitleMenuManagerObject.h"
#include "title/TitleMenuManager.h"

namespace app
{
	namespace title
	{
		TitleMenuManagerObject::TitleMenuManagerObject()
		{
		}

		TitleMenuManagerObject::~TitleMenuManagerObject()
		{
			TitleMenuManager::Finalize();
		}

		bool TitleMenuManagerObject::Start()
		{
			TitleMenuManager::Initialize();

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