/**
 * EventCharacterファイル
 */
#pragma once
#include "Actor.h"
#include "ActorStatus.h"
#include "ActorStateMachine.h"
#include "BattleCharacter.h"
#include "actor/Types.h"
#include "actor/EnemyPool.h"
#include "EnemyAttackPointManager.h"

namespace app
{
	namespace actor
	{
		class EventCharacter : public Character
		{
			appActor(EventCharacter);

		private:
			using SuperClass = Character;
			using DeadCallback = std::function<void()>;

		private:
			DeadCallback onDead_ = nullptr;
			std::unique_ptr<EventCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;
			Vector3 forward_ = g_vec3Front;
			bool isPause_ = false;

		public:
			EventCharacter();
			~EventCharacter();

			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;

		public:
			void Initialize(CharacterInitializeParameter& param) override final;
			/** 当たり判定を作り直す */
			void ResizeCollision();
			Vector3 GetForward()
			{
				return forward_;
			}

			template <typename TState>
			void AddState()
			{
				stateMachine_->AddState<TState>();
			}

			EventCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}

			app::collision::GhostBody* GetGhostBody() const override
			{
				return ghostBody_.get();
			}

			void SetPause(bool isPause)
			{
				isPause_ = isPause;
			}
			void AddOnDead(DeadCallback callback)
			{
				onDead_ = std::move(callback);
			}

		};




		/****************************************************/


		class StoneEventCharacter : public Character , public IPoolable
		{
			appActor(StoneEventCharacter);


		private:
			using SuperClass = Character;
			using DeadCallback = std::function<void()>;

		private:
			DeadCallback onDead_ = nullptr;
			std::unique_ptr<StoneEventCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;
			std::vector<std::function<void()>> onDeadCallbacks_;
			app::actor::BattleCharacter* battleCharacter_ = nullptr;
			static int instanceCount_;
			Vector3 forward_ = g_vec3Front;
			bool isPause_ = true;
			/** 表示フラグ */
			bool isVisible_ = false;
			bool justSpawned_ = false;

			EnemyAttackPointManager* attackPointManager_;
			EnemyAttackPoint::AttackPoint* currentAttackPoint_ = nullptr;

		public:
			StoneEventCharacter();
			~StoneEventCharacter();

			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			/** 初期化処理 */
			void Initialize(CharacterInitializeParameter& param) override final;


			/** 当たり判定を作り直す */
			void ResizeCollision();


			Vector3 GetForward()
			{
				return forward_;
			}

			void TakeDamage(int damegeHP)
			{
				currentHP_ -= damegeHP;
				if (currentHP_ < 0)
				{
					currentHP_ = 0;
				}
			}

			void SyncCurrentHPFromStatus()
			{
				currentHP_ = static_cast<int>(GetStatus()->GetCurrentHp());
			}

			static int GetNum() {return instanceCount_;}


			template <typename TState>
			void AddState()
			{
				stateMachine_->AddState<TState>();
			}


			StoneEventCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}

			app::collision::GhostBody* GetGhostBody() const override
			{
				return ghostBody_.get();
			}

			void SetPause(bool isPause)
			{
				isPause_ = isPause;
			}

			void SetBattleCharacter(app::actor::BattleCharacter* battleCharacter)
			{
				battleCharacter_ = battleCharacter;
			}

			void SetAttckPointManager(EnemyAttackPointManager* manager)
			{
				attackPointManager_ = manager;
			}

			void SetCurrentAttackPoint(EnemyAttackPoint::AttackPoint* attackPoint)
			{
				currentAttackPoint_ = attackPoint;
			}

			EnemyAttackPointManager* GetAttackPointManager() const
			{
				return attackPointManager_;
			}

			EnemyAttackPoint::AttackPoint* GetCurrentAttackPoint() const
			{
				return currentAttackPoint_;
			}

			void AddOnDead(std::function<void()> callback)
			{
				onDeadCallbacks_.push_back(std::move(callback));
			}

			void NotifyDead()
			{
				for (auto& cb : onDeadCallbacks_)
				{
					if (cb)cb();
				}
				onDeadCallbacks_.clear();
			}

			void OnSpawn() override
			{
				StoneEventCharacterStatus* s = status_ -> As<StoneEventCharacterStatus>();

				if (s && battleCharacter_)
				{
					s->Setup();
					s->ResetPhase();
					s->ApplyPhase(battleCharacter_->GetLevel());
				}

				// 表示ON・コリジョンON・ステートマシン初期化
				GetStatus()->SetCurrentHp(GetStatus()->GetMaxHp());
				currentHP_ = static_cast<int>(GetStatus()->GetMaxHp());
				ghostBody_->SetActive(true);
				onDeadCallbacks_.clear(); // 前回のコールバックをクリア
				instanceCount_++;

				stateMachine_->Initialize();

				justSpawned_ = true;
				isPause_ = false;
				isVisible_ = true;

				currentAttackPoint_ = nullptr;
			}

			void OnRecycle() override
			{
				if (attackPointManager_)
				{
					attackPointManager_ ->RemoveFromToken(this);
					if (currentAttackPoint_)
					{
						attackPointManager_->ReleaseAttackPoint(currentAttackPoint_, this);
						currentAttackPoint_ = nullptr;
					}
				}

				// リサイクル時に攻撃ポイントを解放
				if (attackPointManager_ != nullptr && currentAttackPoint_ != nullptr)
				{
					attackPointManager_->ReleaseAttackPoint(currentAttackPoint_, this);
					currentAttackPoint_ = nullptr;
				}

				// 表示OFF・コリジョンOFF（DeleteGOは呼ばない！）
				ghostBody_->SetActive(false);
				instanceCount_--;
				if (instanceCount_ < 0) { instanceCount_ = 0; }
				
				isPause_ = true;
				isVisible_ = false;
			}

			static void ResetInstanceCount()
			{
				instanceCount_ = 0;
			}
		};




		/****************************************************/


		class MushroomEventCharacter : public Character, public IPoolable
		{
			appActor(MushroomEventCharacter);


		private:
			using SuperClass = Character;
			using DeadCallback = std::function<void()>;

		private:
			DeadCallback onDead_ = nullptr;
			std::unique_ptr<MushroomEventCharacterStateMachine> stateMachine_ = nullptr;
			std::unique_ptr<app::collision::GhostBody> ghostBody_ = nullptr;
			std::vector<std::function<void()>> onDeadCallbacks_;
			app::actor::BattleCharacter* battleCharacter_ = nullptr;
			EnemyAttackPointManager* attackPointManager_ = nullptr;
			EnemyAttackPoint::AttackPoint* currentAttackPoint_ = nullptr;
			static int instanceCount_;
			Vector3 forward_ = g_vec3Front;
			bool isPause_ = true;
			bool justSpawned_ = false;
			/** 表示フラグ */
			bool isVisible_ = false;

		public:
			MushroomEventCharacter();
			~MushroomEventCharacter();


			bool Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			/** 初期化処理 */
			void Initialize(CharacterInitializeParameter& param) override final;


			/** 当たり判定を作り直す */
			void ResizeCollision();


			Vector3 GetForward()
			{
				return forward_;
			}

			void TakeDamage(int damegeHP)
			{
				currentHP_ -= damegeHP;
				if (currentHP_ < 0)
				{
					currentHP_ = 0;
				}
			}

			void SyncCurrentHPFromStatus()
			{
				currentHP_ = static_cast<int>(GetStatus()->GetCurrentHp());
			}

			static int GetNum() { return instanceCount_; }



			template <typename TState>
			void AddState()
			{
				stateMachine_->AddState<TState>();
			}


			MushroomEventCharacterStateMachine* GetStateMachine()
			{
				return stateMachine_.get();
			}

			app::collision::GhostBody* GetGhostBody() const override
			{
				return ghostBody_.get();
			}

			void SetPause(bool isPause)
			{
				isPause_ = isPause;
			}

			void SetBattleCharacter(app::actor::BattleCharacter* battleCharacter)
			{
				battleCharacter_ = battleCharacter;
			}

			void SetAttckPointManager(EnemyAttackPointManager* manager)
			{
				attackPointManager_ = manager;
			}

			void SetCurrentAttackPoint(EnemyAttackPoint::AttackPoint* attackPoint)
			{
				currentAttackPoint_ = attackPoint;
			}

			EnemyAttackPointManager* GetAttackPointManager() const
			{
				return attackPointManager_;
			}

			EnemyAttackPoint::AttackPoint* GetCurrentAttackPoint() const
			{
				return currentAttackPoint_;
			}

			void AddOnDead(std::function<void()> callback)
			{
				onDeadCallbacks_.push_back(std::move(callback));
			}

			void NotifyDead()
			{
				for (auto& cb : onDeadCallbacks_)
				{
					if (cb)cb();
				}
				onDeadCallbacks_.clear();
			}

			void OnSpawn() override
			{
				MushroomEventCharacterStatus* s = status_->As<MushroomEventCharacterStatus>();

				if (s && battleCharacter_)
				{
					s->Setup();
					s->ResetPhase();
					s->ApplyPhase(battleCharacter_->GetLevel());
				}

				// 表示ON・コリジョンON・ステートマシン初期化
				GetStatus()->SetCurrentHp(GetStatus()->GetMaxHp());
				currentHP_ = static_cast<int>(GetStatus()->GetMaxHp());
				ghostBody_->SetActive(true);
				// 前回のコールバックをクリア
				onDeadCallbacks_.clear();
				instanceCount_++;
				stateMachine_->Initialize();

				justSpawned_ = true;
				isPause_ = false;
				isVisible_ = true;

				currentAttackPoint_ = nullptr;
			}

			void OnRecycle() override
			{
				if (attackPointManager_)
				{
					attackPointManager_->RemoveFromToken(this);
					if (currentAttackPoint_)
					{
						attackPointManager_->ReleaseAttackPoint(currentAttackPoint_, this);
						currentAttackPoint_ = nullptr;
					}
				}

				// リサイクル時に攻撃ポイントを解放
				if(attackPointManager_ && currentAttackPoint_)
				{
					attackPointManager_->ReleaseAttackPoint(currentAttackPoint_, this);
					currentAttackPoint_ = nullptr;
				}

				// 表示OFF・コリジョンOFF
				ghostBody_->SetActive(false);
				instanceCount_--;
				if (instanceCount_ < 0) { instanceCount_ = 0; }
				
				isPause_ = true;
				isVisible_ = false;
			}

			static void ResetInstanceCount()
			{
				instanceCount_ = 0;
			}
		};
	}
}