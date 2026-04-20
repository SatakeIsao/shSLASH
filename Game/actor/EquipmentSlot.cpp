#include "stdafx.h"
#include "EquipmentSlot.h"

namespace app
{
    namespace actor
    {
        EquipmentSlot::EquipmentSlot(EquipmentSlotType type)
            : slotType_(type)
            , equipment_(nullptr)
        {}

        void EquipmentSlot::Equip(std::unique_ptr<Equipment> equip)
        {
            // 前の装備は unique_ptr が自動破棄する
            equipment_ = std::move(equip);
        }

        std::unique_ptr<Equipment> EquipmentSlot::Unequip()
        {
            return std::move(equipment_); // nullptr になる
        }

        bool EquipmentSlot::HasEquipment() const
        {
            return equipment_ != nullptr;
        }

        EquipmentSlotType EquipmentSlot::GetSlotType() const
        {
            return slotType_;
        }

        Equipment& EquipmentSlot::GetEquipment()
        {
            return *equipment_;
        }

        const Equipment& EquipmentSlot::GetEquipment() const
        {
            return *equipment_;
        }

        float EquipmentSlot::GetAttackPower() const
        {
            return HasEquipment() ? equipment_->GetAttackPower() : 0.0f;
        }

        float EquipmentSlot::GetDefensePower() const
        {
            return HasEquipment() ? equipment_->GetDefensePower() : 0.0f;
        }
    }
}