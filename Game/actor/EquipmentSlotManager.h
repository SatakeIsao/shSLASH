#pragma once
#include "EquipmentSlot.h"
#include <unordered_map>
#include <memory>

namespace app
{
    namespace actor
    {
        class EquipmentSlotManager
        {
        private:
            std::unordered_map<EquipmentSlotType, EquipmentSlot> slots_;

        public:
            EquipmentSlotManager();

            // 指定スロットを取得（存在しない種別なら nullptr）
            EquipmentSlot* GetSlot(EquipmentSlotType type);
            const EquipmentSlot* GetSlot(EquipmentSlotType type) const;

            // 指定スロットに装備する
            void Equip(EquipmentSlotType type, std::unique_ptr<Equipment> equip);

            // 指定スロットの装備を外す
            std::unique_ptr<Equipment> Unequip(EquipmentSlotType type);

            // 全スロット合計のステータス
            float GetTotalAttackPower()  const;
            float GetTotalDefensePower() const;
        };
    }
}