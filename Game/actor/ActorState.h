/**
 * ActorState.h
 * キャラクターステート関連
 */
#pragma once

#include <random>
#include "sound/SoundManager.h" 
#include "actor/Types.h"

/**
 * 当初はenumで管理しようと思ったが、
 * ハッシュ値に変更
 * 理由：単純に文字列で検索をかけるより、
 * 数値という名のIDで検索かけた方が早いから。
 */
#define appState(name)\
public:\
	static constexpr uint32_t ID() { return Hash32(#name); }



namespace app
{
	namespace collision
	{
		class GhostBody;
	}


	namespace actor
	{
		class IStateMachine;


		class ICharacterState : public Noncopyable
		{
		protected:
			IStateMachine* owner_ = nullptr;


		public:
			ICharacterState(IStateMachine* owner) : owner_(owner) {}
			virtual ~ICharacterState() {}

			virtual void Enter() = 0;
			virtual void Update() = 0;
			virtual void Exit() = 0;

			virtual bool CanChangeState() const { return false; }
		};




		class IdleCharacterState : public ICharacterState
		{
			appState(IdleCharacterState);


		public:
			IdleCharacterState(IStateMachine* owner);
			~IdleCharacterState();

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		class RunCharacterState : public ICharacterState
		{
			appState(RunCharacterState);


		private:
			float footStepTimer_ = 0.0f;  // 足音タイマー
			app::SoundHandle footStepHandle_ = app::INVALID_SOUND_HANDLE;

		public:
			RunCharacterState(IStateMachine* owner);
			~RunCharacterState();

			void Enter() override;
			void Update() override;
			void Exit() override;
		};




		class AttackCharacterState :public ICharacterState
		{
			appState(AttackCharacterState);


		private:
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;
			float stateTimer_ = 0.0f;
			bool isAttackBody_ = false;

		public:
			AttackCharacterState(IStateMachine* owner);
			~AttackCharacterState();

			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;

			bool IsAttackBody() const
			{
				return isAttackBody_;
			}
		};




		class JumpCharacterState : public ICharacterState
		{
			appState(JumpCharacterState);


		private:
			enum class JumpPhase
			{
				Ascend,		// 上昇
				Falling,	// 落下
				Land		// 着地
			};


		private:
			JumpPhase jumpPhase_ = JumpPhase::Ascend;


		public:
			JumpCharacterState(IStateMachine* owner);
			~JumpCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class FallingCharacterState : public ICharacterState
		{
			appState(FallingCharacterState);


		public:
			FallingCharacterState(IStateMachine* owner);
			~FallingCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;
		};



		class ComboAttackCharacterState : public ICharacterState
		{
		protected:
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;
			float stateTimer_ = 0.0f;
			bool isComboInput_ = false;
			bool isFirstFrame_ = true;

			// 派生クラスで設定するパラメータ
			struct ComboParam
			{
				PlayerAnimationKind animKind;		// アニメーション種別
				float animSpeed = 1.5f;				// アニメーション速度
				float comboWindowTime = 0.8f;		// コンボ受付開始時間
				float attackBodyDelay = 0.3f;		// 攻撃判定発生までの時間
				float attackBodyDuration = 0.3f;	// 攻撃判定の持続時間
				float attackBodyRadius = 45.0f;		// 攻撃判定の大きさ
			};

		public:
			ComboAttackCharacterState(IStateMachine* owner) : ICharacterState(owner) {}
			virtual ~ComboAttackCharacterState() {}

			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const override;
			bool IsComboInput() const { return isComboInput_; }

		protected:
			// 派生クラスでパラメータを返す
			virtual ComboParam GetComboParam() const = 0;
		};




		class SlashFirstCharacterState : public ComboAttackCharacterState
		{
			appState(SlashFirstCharacterState);

		public:
			SlashFirstCharacterState(IStateMachine* owner) : ComboAttackCharacterState(owner) {}
			~SlashFirstCharacterState() {}

		protected:
			ComboParam GetComboParam() const override
			{
				ComboParam param;
				param.animKind = PlayerAnimationKind::SlashFirst;
				param.animSpeed = 1.5f;
				param.comboWindowTime = 0.6f;
				param.attackBodyDelay = 0.3f;
				param.attackBodyDuration = 0.3f;
				param.attackBodyRadius = 45.0f;
				return param;
			}
		};




		class SlashSecondCharacterState : public ComboAttackCharacterState
		{
			appState(SlashSecondCharacterState);

		public:
			SlashSecondCharacterState(IStateMachine* owner) : ComboAttackCharacterState(owner) {}
			~SlashSecondCharacterState() {}

		protected:
			ComboParam GetComboParam() const override
			{
				ComboParam param;
				param.animKind = PlayerAnimationKind::SlashSecond;
				param.animSpeed = 1.5f;
				param.comboWindowTime = 0.8f;
				param.attackBodyDelay = 0.3f;
				param.attackBodyDuration = 0.3f;
				param.attackBodyRadius = 45.0f;
				return param;
			}
		};




		class SlashThirdCharacterState : public ComboAttackCharacterState
		{
			appState(SlashThirdCharacterState);

		public:
			SlashThirdCharacterState(IStateMachine* owner) : ComboAttackCharacterState(owner) {}
			~SlashThirdCharacterState() {}

		protected:
			ComboParam GetComboParam() const override
			{
				ComboParam param;
				param.animKind = PlayerAnimationKind::SlashThird;
				param.animSpeed = 1.5f;
				param.comboWindowTime = 0.8f;
				param.attackBodyDelay = 0.3f;
				param.attackBodyDuration = 0.3f;
				param.attackBodyRadius = 45.0f;
				return param;
			}
		};





		class ChargeAttackCharacterState : public ICharacterState
		{
			appState(ChargeAttackCharacterState);


		private:
			enum class ChargeAttackPhase
			{
				Start,		// 開始
				Looping,	// チャージ中
				End			// 終了
			};

		private:
			ChargeAttackPhase chargeAttackPhase_ = ChargeAttackPhase::Start;
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;


		public:
			ChargeAttackCharacterState(IStateMachine* owner);
			~ChargeAttackCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class WarpInCharacterState : public ICharacterState
		{
			appState(WarpCharacterState);


		private:
			app::util::Vector3Curve translateCurve_;
			app::util::FloatCurve scaleCurve_;


		public:
			WarpInCharacterState(IStateMachine* owner);
			~WarpInCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class WarpOutCharacterState : public ICharacterState
		{
			appState(WarpOutCharacterState);


		private:
			app::util::FloatCurve scaleCurve_;


		public:
			WarpOutCharacterState(IStateMachine* owner);
			~WarpOutCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class DeadCharacterState : public ICharacterState
		{
			appState(DeadCharacterState);


		private:
			float timer_ = 0.0f;

		public:
			DeadCharacterState(IStateMachine* owner);
			~DeadCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class KnockBackCharacterState : public ICharacterState
		{
			appState(KnockBackCharacterState);


		private:
			float timer_ = 0.0f;

		public:
			KnockBackCharacterState(IStateMachine* owner);
			~KnockBackCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class GuardCharacterState : public ICharacterState
		{
			appState(GuardCharacterState);


		private:
			float timer_ = 0.0f;

		public:
			GuardCharacterState(IStateMachine* owner);
			~GuardCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class AvoidanceCharacterState : public ICharacterState
		{
			appState(AvoidanceCharacterState);


		private:
			float timer_ = 0.0f;
			Vector3 avoidanceDirection_; // 回避方向

		public:
			AvoidanceCharacterState(IStateMachine* owner);
			~AvoidanceCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class InjuredIdleCharacterState : public ICharacterState
		{
			appState(InjuredIdleCharacterState);


		private:
			float timer_ = 0.0f;
			bool isInjuredIdleSEPlayed_ = false;

		public:
			InjuredIdleCharacterState(IStateMachine* owner);
			~InjuredIdleCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class InjuredRunCharacterState : public ICharacterState
		{
			appState(InjuredRunCharacterState);


		private:
			float timer_ = 0.0f;

		public:
			InjuredRunCharacterState(IStateMachine* owner);
			~InjuredRunCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class KipUpCharacterState : public ICharacterState
		{
			appState(KipUpCharacterState);


		private:
			float timer_ = 0.0f;
			float seTimer_ = 0.0f;
			bool sePlayed_ = false;

		public:
			KipUpCharacterState(IStateMachine* owner);
			~KipUpCharacterState();
			void Enter() override;
			void Update() override;
			void Exit() override;

			virtual bool CanChangeState() const;
		};




		class PatrolCharacterState : public ICharacterState
		{
			appState(PatrolCharacterState);


		private:
			float timer_ = 0.0f;
			float patrolTimer_ = 0.0f;
			Vector3 patrolDirection_;

			std::mt19937 randomEngine_;


		public:
			PatrolCharacterState(IStateMachine* owner);
			~PatrolCharacterState();


			void Enter() override;
			void Update() override;
			void Exit() override;


			virtual bool CanChangeState() const;
		};
	}
}