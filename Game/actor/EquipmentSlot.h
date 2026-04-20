#pragma once
#include "Equipment.h"
#include <memory>

namespace app
{
    namespace actor
    {
        // 装備スロットの種別
        enum class EquipmentSlotType
        {
            Weapon,
            //HeadArmor,
            //BodyArmor,
        };


        /**************************************************************/


        class EquipmentSlot
        {
        private:
            EquipmentSlotType          slotType_;
            std::unique_ptr<Equipment> equipment_; // nullptr = 未装備

        public:
            explicit EquipmentSlot(EquipmentSlotType type);

            // コピー禁止（unique_ptr を持つため）
            EquipmentSlot(const EquipmentSlot&) = delete;
            EquipmentSlot& operator=(const EquipmentSlot&) = delete;

            // ムーブは許可
            EquipmentSlot(EquipmentSlot&&) = default;
            EquipmentSlot& operator=(EquipmentSlot&&) = default;

            // 装備する（前の装備は破棄される）
            void Equip(std::unique_ptr<Equipment> equip);

            // 外す（所有権を呼び出し元へ返す）
            std::unique_ptr<Equipment> Unequip();

            // 装備しているか
            bool HasEquipment() const;

            // スロット種別を取得
            EquipmentSlotType GetSlotType() const;

            // 装備品への参照を取得（HasEquipment() を確認してから呼ぶこと）
            Equipment& GetEquipment();
            const Equipment& GetEquipment() const;

            // ステータス取得（未装備なら 0.0f）
            float GetAttackPower()  const;
            float GetDefensePower() const;
        };
    }
}
