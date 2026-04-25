#include "stdafx.h"
#include "EventCharacterSpawnManagerObject.h"

namespace app
{
	namespace actor
	{
		EventCharacterSpawnManagerObject::EventCharacterSpawnManagerObject()
		{}

		EventCharacterSpawnManagerObject::~EventCharacterSpawnManagerObject()
		{}

		bool EventCharacterSpawnManagerObject::Start()
		{
			return true;
		}

		void EventCharacterSpawnManagerObject::Update()
		{
			spawnManager_.Update();
		}
	}
}
