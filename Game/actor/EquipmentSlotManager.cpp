#include "stdafx.h"
#include "EquipmentSlotManager.h"

namespace app
{
    namespace actor
    {
        EquipmentSlotManager::EquipmentSlotManager()
        {
            // 全スロットを空で初期化しておく
            auto addSlot = [&](EquipmentSlotType type)
                {
                    slots_.emplace(type, EquipmentSlot(type));
                };

            addSlot(EquipmentSlotType::Weapon);
            //addSlot(EquipmentSlotType::HeadArmor);
            //addSlot(EquipmentSlotType::BodyArmor);
        }

        EquipmentSlot* EquipmentSlotManager::GetSlot(EquipmentSlotType type)
        {
            auto it = slots_.find(type);
            return (it != slots_.end()) ? &it->second : nullptr;
        }

        const EquipmentSlot* EquipmentSlotManager::GetSlot(EquipmentSlotType type) const
        {
            auto it = slots_.find(type);
            return (it != slots_.end()) ? &it->second : nullptr;
        }

        void EquipmentSlotManager::Equip(EquipmentSlotType type, std::unique_ptr<Equipment> equip)
        {
            auto* slot = GetSlot(type);
            if (slot)
            {
                slot->Equip(std::move(equip));
            }
        }

        std::unique_ptr<Equipment> EquipmentSlotManager::Unequip(EquipmentSlotType type)
        {
            auto* slot = GetSlot(type);
            if (slot)
            {
                return slot->Unequip();
            }
            return nullptr;
        }

        float EquipmentSlotManager::GetTotalAttackPower() const
        {
            float total = 0.0f;
            for (const auto& pair : slots_)
            {
                total += pair.second.GetAttackPower();
            }
            return total;
        }

        float EquipmentSlotManager::GetTotalDefensePower() const
        {
            float total = 0.0f;
            for (const auto& pair : slots_)
            {
                total += pair.second.GetDefensePower();
            }
            return total;
        }
    }
}