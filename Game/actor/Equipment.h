#pragma once

namespace app
{
    namespace actor
    {
        class Equipment
        {
        public:
            virtual ~Equipment() = default;

            // 全装備品が共通で持つもの
            virtual float GetAttackPower()  const { return 0.0f; }
            virtual float GetDefensePower() const { return 0.0f; }
        };




        /**************************************************************/


        class Weapon : public Equipment
        {
        private:
            float attackPower_ = 0.0f;
            int level_ = 0;

        public:
            Weapon();

            // 武器は攻撃力を上書きする
            float GetAttackPower() const { return attackPower_; }
            // レベル上昇
            void LevelUp();
        };
    }
}
