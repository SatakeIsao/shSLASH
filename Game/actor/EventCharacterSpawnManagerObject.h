#pragma once
#include "actor/EventCharacterSpawnManager.h"

namespace app
{
	namespace actor
	{
		class EventCharacterSpawnManagerObject : public IGameObject
		{
		public:
			EventCharacterSpawnManagerObject();
			~EventCharacterSpawnManagerObject();

			bool Start();
			void Update();

		public:
			void SetPause(bool isPause)
			{
				spawnManager_.SetPause(isPause);
			}

			EventCharacterSpawnManager& GetManager() { return spawnManager_; }

		private:
			EventCharacterSpawnManager spawnManager_;
		};
	}
}
