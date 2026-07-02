#include "stdafx.h"
#include "actor/EnemyPhase.h"

bool app::actor::EnemyPhase::Update(int playerLevel, EnemyPhaseData& outData)
{
    for(int i = static_cast<int>(phases_.size()) - 1; i >= 0; --i)
    {
        if(playerLevel >= phases_[i].requiredPlayerLevel)
        {
            if (currentIndex_ == i) return false;

			currentIndex_ = i;
			outData = phases_[i];
            phaseCount_++;
            return true;
        }
	}
    return false;
}
