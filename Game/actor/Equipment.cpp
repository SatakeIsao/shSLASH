#include "stdafx.h"
#include "Equipment.h"
#include "core/ParameterManager.h"

namespace
{
    constexpr int MAX_LEVEL = 6;
}

namespace app
{
    namespace actor
    {
        Weapon::Weapon()
        {
            // BattleManager::LoadParameter() で読み込んだデータを取得する
            const auto* param = app::core::ParameterManager::Get().GetParameter<app::core::MasterWeaponParameter>(level_);

            if (param){
                attackPower_ = param->attackPower;
            }
        }


        void Weapon::LevelUp()
        {
            if (level_ < MAX_LEVEL){
                level_++;
            }

            // パラメータを再取得
            const auto* param = app::core::ParameterManager::Get().GetParameter<app::core::MasterWeaponParameter>(level_);

            if (param){
                attackPower_ = param->attackPower;
            }
        }
    }
}