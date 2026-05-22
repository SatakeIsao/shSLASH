/**
 * BattleCharacterファイル
 */
#pragma once
#include "Actor.h"
#include "ActorStateMachine.h"
#include "EventCharacterSpawnManager.h"
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
			app::actor::EventCharacterSpawnManager* spawnManager_ = nullptr;

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

				if(spawnManager_)
				{
					spawnManager_->OnPlayerLevelUp(level_);
				}

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
				// ポーズ開始のタイミングだけ足音を止める
				if (isPause && !isPause_)
				{
					stateMachine_->OnPause();
				}
				else if (!isPause && isPause_)
				{
					// ポーズ解除：走り中なら足音を再開する
					stateMachine_->OnResume();
				}
				isPause_ = isPause;
			}


			void SetSpawnManager(app::actor::EventCharacterSpawnManager* spawnManager)
			{
				spawnManager_ = spawnManager;
			}
		};
	}
}