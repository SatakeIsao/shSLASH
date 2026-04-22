/**
 * BattleCharacterファイル
 */
#pragma once
#include "Actor.h"
#include "ActorStateMachine.h"
#include "actor/Types.h"
#include "actor/Equipment.h"


namespace app
{
	namespace actor
	{
		class BattleCharacter : public Character
		{
			appActor(BattleCharacter);


		private:
			using SuperClass = Character;


		private:
			std::unique_ptr<BattleCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;

			/** TODO: jsonで管理 */
			int level_ = 0;
			bool isPause_ = false;


		public:
			BattleCharacter();
			~BattleCharacter();

			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			void Initialize(CharacterInitializeParameter& param) override final;
			float GetTotalAttack() const;
			int GetLevel() const { return level_; }
			void LevelUp() 
			{ 
				if (level_ >= 10) return;
				level_++; 
				// EquipmentSlot 経由で武器のレベルを上げる
				auto* slot = equipmentSlots_.GetSlot(EquipmentSlotType::Weapon);
				if (slot && slot->HasEquipment())
				{
					// Weapon にキャストしてLevelUp呼び出し
					auto* weapon = dynamic_cast<Weapon*>(&slot->GetEquipment());
					if (weapon)
					{
						weapon->LevelUp();
					}
				}
			}

		public:
			template <typename TState>
			void AddState()
			{
				stateMachine_->AddState<TState>();
			}


			BattleCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}


			app::collision::GhostBody* GetGhostBody() const
			{
				return ghostBody_.get();
			}


			int GetCurrentHP() const
			{
				return currentHP_;
			}



			void SetPouse(bool isPause)
			{
				isPause_ = isPause;
			}
		};
	}
}