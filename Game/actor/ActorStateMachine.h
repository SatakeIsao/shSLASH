/**
 * ActorStateMachineファイル
 */
#pragma once
#include <util/util.h>
#include "ActorState.h"
#include "sound/SoundManager.h"
#include "effect/EffectManager.h"
#include "effect/Types.h"
#include "actor/EnemyAttackPointManager.h"


namespace app
{
	namespace actor
	{
		class CharacterStatus;
		class Character;


#ifdef K2_DEBUG
		/**
		 * デバッグ用：Stoneのとびかかり攻撃が視野角外で本当に発動していないかを
		 * 確認するための統計情報（画面外ブロック回数 / 実発動回数）
		 * BattleManager の画面表示から参照される
		 */
		struct StonePounceDebugStats
		{
			static int triggeredCount;
			static int blockedByCameraCount;
		};

		/**
		 * デバッグ用：溜め攻撃の地響き床デカール／衝撃波LightPillerエフェクトが
		 * 実際にどこで止まっているか（振り下ろし到達回数 / 床レイキャスト成功回数）を
		 * 確認するための統計情報。BattleManager の画面表示から参照される
		 */
		struct ChargeShockwaveDebugStats
		{
			static int attemptCount;
			static int raycastHitCount;
		};
#endif


		class IStateMachine : public Noncopyable
		{
			/** 例外としてpublic */
		public:
			app::math::Transform transform;

			/** 触らせない */
		private:
			std::map<uint32_t, std::function<ICharacterState* ()>> stateFuncList_;
			std::unique_ptr<ICharacterState> currentState_ = nullptr;
			/** ステート関連 */
			uint32_t currentStateId_ = INVALID_STATE_ID;
			uint32_t nextStateId_ = INVALID_STATE_ID;


		public:
			IStateMachine() {}
			virtual ~IStateMachine() {}

			virtual void Initialize() = 0;
			virtual void Update() = 0;

			/**
			 * 現在のステートを強制的にExitさせる。
			 * リトライ等でアクターを破棄する直前に呼ぶことで、
			 * Exit()を経由しない放置エフェクト・攻撃ゴーストのリークを防ぐ。
			 */
			void ForceExitCurrentState()
			{
				if (currentState_) {
					currentState_->Exit();
				}
			}


		public:
			template <typename T>
			bool Is() const
			{
				auto* ptr = dynamic_cast<T*>(this);
				return ptr != nullptr;
			}
			template <typename T>
			T* As()
			{
				return dynamic_cast<T*>(this);
			}



			/** ステート関連 */
		protected:
			void UpdateStateCore();

			bool CanChangeState() const;

			ICharacterState* GetCurrentState() const
			{
				return currentState_.get();
			}

			void SetCurrentState(const uint32_t stateId)
			{
				currentStateId_ = stateId;
			}

			bool IsEqualCurrentState(const uint32_t stateId) const
			{
				return currentStateId_ == stateId;
			}

			void RequestChangeState(const uint32_t stateId)
			{
				if (nextStateId_ == stateId) return;
				if (stateId == INVALID_STATE_ID) return;
				nextStateId_ = stateId;
			}


		public:
			template <typename T>
			void AddState()
			{
				stateFuncList_.emplace(T::ID(), [&]() { return new T(this); });
			}

		private:
			ICharacterState* CreateState(const uint32_t stateId)
			{
				auto it = stateFuncList_.find(stateId);
				if (it != stateFuncList_.end())
				{
					return it->second();
				}
				K2_ASSERT(false, "指定された状態がありません");
				return nullptr;
			}
		};




		class CharacterStateMachine : public IStateMachine
		{
		protected:
			/** ステートマシンを持っているきゃらくたーの情報 */
			Character* character_ = nullptr;

			/** 移動関連 */
			Vector3 moveDirection_ = Vector3::Front;
			Vector3 moveSpeedVector_ = Vector3::Zero;
			float inputPower_ = 1.0f;

			/** 攻撃中、噛みつき等の演出に合わせてゴーストボディを前進させるオフセット（AttackCharacterState が更新） */
			float currentAttackLungeOffset_ = 0.0f;

			Vector3 warpStartPosition_ = Vector3::Zero;
			Vector3 warpEndPosition_ = Vector3::Zero;
			bool isRequestWarp_ = false;

			/** 移動にカメラの向きを考慮するか */
			bool isUseCameraDirection_ = true;

			/** ガード中か */
			bool isGuarding_ = false;
			/** 回避中か */
			bool isAvoiding_ = false;
			/** ジャスト回避判定ウィンドウ中か */
			bool isJustDodgeWindow_ = false;
			/** ローカルタイムスケール（ジャスト回避スロー用） */
			float localTimeScale_ = 1.0f;

			/** ボタンを押したか */
			bool isActionA_ = false;
			bool isPressA_ = false;
			bool isActionB_ = false;
			bool isActionDown_ = false;
			bool isActionRB1_ = false;
			bool isActionLB1_ = false;
			bool isTriggerY_ = false;


		public:
			CharacterStateMachine();
			virtual ~CharacterStateMachine();

			virtual void Initialize() override {}
			virtual void Update() override;

			/** 攻撃ステートに入った時の固有処理 */
			virtual void OnEnterAttack() {}
			/** 攻撃ステートから抜ける時の固有処理 */
			virtual void OnExitAttack() {}
			/** ノックバックステートに入った時の固有処理 */
			virtual void OnEnterKnockBack() {}
			/** ノックバックステートから抜ける時の固有処理 */
			virtual void OnExitKnockBack() {}
			/** 死んだステートに入った時の固有処理 */
			virtual void OnEnterDead() {}
			/** 死んだステートから抜ける時の固有処理 */
			virtual void OnExitDead() {}
			/** 防御ステートに入った時の固有処理 */
			virtual void OnEnterGuard() {}
			/** 防御ステートから抜ける時の固有処理 */
			virtual void OnExitGuard() {}
			/** 回避ステートに入った時の固有処理 */
			virtual void OnEnterAvoidance() {}
			/** 回避ステートから抜ける時の固有処理 */
			virtual void OnExitAvoidance() {}
			/** 走りステートに入った時の固有処理 */
			virtual void OnEnterRun() {}
			/** 走りステートから抜ける時の固有処理 */
			virtual void OnExitRun() {}
			/** 疲労走りステートに入った時の固有処理 */
			virtual void OnEnterInjuredRun() {}
			/** 疲労走りステートから抜ける時の固有処理 */
			virtual void OnExitInjuredRun() {}
			/** パンチステートに入った時の固有処理 */
			virtual void OnEnterPunch() {}
			/** パンチステートに抜ける時の固有処理 */
			virtual void OnExitPunch() {}
			/** パンチ2回目ステートに入った時の固有処理 */
			virtual void OnEnterPunchSecond() {}
			/** パンチ2回目ステートに抜ける時の固有処理 */
			virtual void OnExitPunchSecond() {}
			/** パンチ3回目ステートに入った時の固有処理 */
			virtual void OnEnterPunchThird() {}
			/** パンチ3回目ステートに抜ける時の固有処理 */
			virtual void OnExitPunchThird() {}
			/** とびかかり攻撃ステートに入った時の固有処理 */
			virtual void OnEnterPounceAttack() {}
			/** とびかかり攻撃ステートから抜ける時の固有処理 */
			virtual void OnExitPounceAttack() {}

			void Move(const float deltaTime, const float moveSpeed);
			void Jump(const float jumoPower);

			void Setup(Character* character)
			{
				character_ = character;
			}

			virtual uint32_t GetCharacterID() const { return 0; }

			/** 攻撃アニメーション開始からゴーストボディを生成するまでの遅延秒数 */
			virtual float GetGhostBodyDelay() const { return 0.1f; }

			/** 攻撃中の経過時間(秒)に応じたゴーストボディの前進オフセットを返す（噛みつき等、演出に合わせてゴーストボディを連動させる用。既定は連動なし） */
			virtual float GetAttackLungeDistance(float stateTimer) const { return 0.0f; }
			/** AttackCharacterState が毎フレーム計算した現在の前進オフセットを保持する（キャラクター側のゴーストボディ追従に使用） */
			float GetCurrentAttackLungeOffset() const { return currentAttackLungeOffset_; }
			void SetCurrentAttackLungeOffset(float offset) { currentAttackLungeOffset_ = offset; }

			/** 攻撃中、これ以上前進するとまずいか（例：ガード中のプレイヤーの防御球へめり込む）を判定する。既定はfalse（制限なし） */
			virtual bool IsApproachBlocked() { return false; }

			/**
			 * 移動状態の各ステートから、目標方向への向き適用を委譲するためのフック。
			 * 既定は即座に向く（従来通り）。エネミーが密集して目標方向が毎フレーム
			 * 入れ替わるようなケースでは、派生クラス側で最大回転速度を設けた
			 * なめらかな追従に差し替えることができる
			 */
			virtual void ApplyFacingDirection(const Vector3& direction)
			{
				if (direction.LengthSq() < 0.0001f) { return; }
				transform.rotation.SetRotationYFromDirectionXZ(direction);
			}

			Character* GetCharacter();
			app::actor::CharacterStatus* GetStatus();
			CharacterController* GetCharacterController();
			ModelRender* GetModelRender();



			/** 移動関連 */
		public:
			void SetMoveDirection(const Vector3& direction) { moveDirection_ = direction; }
			const Vector3& GetMoveDirection() const { return moveDirection_; }
			const Vector3& GetMoveSpeedVector() const { return moveSpeedVector_; }
			void ClearMomveSpeedVector() { moveSpeedVector_ = Vector3::Zero; }

			void SetInputPower(const float power) { inputPower_ = power; }
			float GetInputPower() const { return inputPower_; }


			void SetWarpPosition(const Vector3& start, const Vector3& end)
			{
				warpStartPosition_ = start;
				warpEndPosition_ = end;
				isRequestWarp_ = true;
			}
			const Vector3& GetWarpStartPosition() const { return warpStartPosition_; }
			const Vector3& GetWarpEndPosition() const { return warpEndPosition_; }
			const bool IsRequestWarp() const { return isRequestWarp_; }
			void ClearRequestWarp() { isRequestWarp_ = false; }
			/** ガード状態を設定 */
			void SetGuarding(const bool isGuarding) {isGuarding_ = isGuarding; }
			bool IsGuarding() const { return isGuarding_; }
			/** 回避状態を設定 */
			void SetAvoiding(const bool isAvoiding) { isAvoiding_ = isAvoiding; }
			bool IsAvoiding() const { return isAvoiding_; }
			/** ジャスト回避ウィンドウを設定 */
			void SetJustDodgeWindow(const bool active) { isJustDodgeWindow_ = active; }
			bool IsJustDodgeWindow() const { return isJustDodgeWindow_; }
			/** ジャスト回避発動（派生クラスでオーバーライドして効果を実装） */
			virtual void OnJustDodge() {}
			/** ローカルタイムスケール操作（回避ステートの移動・アニメだけをスローにする用） */
			void SetLocalTimeScale(float s) { localTimeScale_ = s; }
			float GetLocalTimeScale() const { return localTimeScale_; }
			float GetLocalDeltaTime() const { return g_gameTime->GetFrameDeltaTime() * localTimeScale_; }


			/** 入力周り */
			// TODO: PressとTriggerを分かるように分けたい。
		public:
			void SetActionA(const bool isAction) { isActionA_ = isAction; }
			bool IsActionA() const { return isActionA_; }
			void SetPressA(const bool isAction) { isPressA_ = isAction; }
			bool IsPressA() const { return isPressA_; }
			void SetActionB(const bool isAction) { isActionB_ = isAction; }
			bool IsActionB() const { return isActionB_; }
			void SetActionDown(const bool isAction) { isActionDown_ = isAction; }
			bool IsActionDown() const { return isActionDown_; }
			void SetActionRB1(const bool isAction) { isActionRB1_ = isAction; }
			bool IsActionRB1() const { return isActionRB1_; }
			void SetActionLB1(const bool isAction) { isActionLB1_ = isAction; }
			bool IsActionLB1() const { return isActionLB1_; }
			void SetTriggerY(const bool isAction) { isTriggerY_ = isAction; }
			bool IsTriggerY() const { return isTriggerY_; }

			void ClearInput()
			{
				inputPower_  = 0.0f;
				isActionA_   = false;
				isPressA_    = false;
				isActionB_   = false;
				isActionDown_= false;
				isActionRB1_ = false;
				isActionLB1_ = false;
				isTriggerY_  = false;
			}
		};




		class BattleCharacterStateMachine : public CharacterStateMachine
		{
		private:
			using SuperClass = CharacterStateMachine;
			/** 足音SEのハンドル */
			app::SoundHandle footStepHandle_ = app::INVALID_SOUND_HANDLE;
			/** 疲労足音SEのハンドル */
			app::SoundHandle InjuredFootStepHandle_ = app::INVALID_SOUND_HANDLE;
			
			/** 定数 */
			/** 最初の待機時間 */
			const float WAIT_TIME = 1.0f;

			/** AI用のタイマー */
			float aiTimer_ = 0.0f;
			/** Dead効果音タイマー */
			float deadSETimer_ = 0.0f;

			/** 死んだか */
			bool isDead_ = false;
			/** ノックバックしたか */
			bool isKnockBack_ = false;
			/** 防御したか */
			bool isGuard_ = false;
			/** 防御中エフェクトのハンドル */
			EffectHandle guardEffectHandle_ = INVALID_EFFECT_HANDLE;
			/** 防御中、エネミーを押し返すための球判定 */
			std::unique_ptr<app::collision::GhostBody> guardBlockBody_;
			/** 防御を継続している時間 */
			float guardHoldTimer_ = 0.0f;
			/** 制限時間に達し、ボタンを離すまで再ガード不可の状態か */
			bool isGuardLocked_ = false;
			/** 防御可能な連続時間（秒）。試験的に5秒 */
			static constexpr float kGuardTimeLimit = 5.0f;
			/** 切り込みエフェクト */
			bool isSlashEffect_ = false;
			/** チャージエフェクト */
			bool isChargeEffectRequested_ = false;
			/** チャージ攻撃エフェクト */
			bool isChargeAttackEffectRequested_ = false;
			/** 現在のチャージレベル (0=未チャージ, 1/2/3) */
			int chargeLevel_ = 0;
			/** チャージ溜め中のリアルタイムレベル (0=未到達, 1/2/3) */
			int currentChargingLevel_ = 0;
			/** Deadの効果音再生したか */
			bool isDeadSEPlayed_ = false;
			/** プレイヤー側ヒットストップの残り時間 */
			float hitStopTimer_ = 0.0f;
			/** ジャスト回避スローモーションの残りフレーム数 */
			int justDodgeSlowFrames_ = 0;
			/** ジャスト回避発動時のコールバック */
			std::function<void()> onJustDodgeCallback_;
			/** [前フェーズ] 敵の攻撃ゴーストが当たった後、回避入力を待つウィンドウ（秒） */
			float preJustDodgeWindowTimer_ = 0.0f;
			/** 直前にノックバックを受けたか（次の回避でジャスト回避を封じる用） */
			bool wasRecentlyKnockedBack_ = false;
			/** 無敵時間中か（ジャスト回避を封じる用） */
			bool isInvincible_ = false;
			/** 通常攻撃1段目の再使用クールダウン（秒） */
			float slashFirstCooldownTimer_ = 0.0f;
			/** 攻撃キャンセルからの回避か（ブレンド時間を長くする用） */
			bool isAvoidanceCancelFromAttack_ = false;


		public:
			BattleCharacterStateMachine();
			virtual ~BattleCharacterStateMachine();

			virtual void Initialize() override final;
			virtual void Update() override final;

			virtual uint32_t GetCharacterID() const override;

			virtual void OnEnterKnockBack() override;
			virtual void OnExitKnockBack() override;

			virtual void OnEnterDead() override;
			virtual void OnExitDead() override;

			virtual void OnEnterGuard() override;
			virtual void OnExitGuard() override;

			virtual void OnEnterAvoidance() override;
			virtual void OnExitAvoidance() override;

			virtual void OnEnterRun() override;
			virtual void OnExitRun() override;

			virtual void OnEnterInjuredRun() override;
			virtual void OnExitInjuredRun() override;

			virtual void OnEnterPunch() override;
			virtual void OnExitPunch() override;

			virtual void OnEnterPunchSecond() override;
			virtual void OnExitPunchSecond() override;

			virtual void OnEnterPunchThird() override;
			virtual void OnExitPunchThird() override;

		private:
			void UpdateState();

			/** 防御球（guardBlockBody_）の中心座標を取得する。transform.position は足元基準のため、
			    体の中心付近まで持ち上げてプレイヤーを囲むようにする */
			Vector3 GetGuardBlockCenter();

		public:
			/** 溜め攻撃ヒット時にプレイヤー側のヒットストップを開始する */
			void StartHitStop(float duration) { hitStopTimer_ = duration; }

			/** ジャスト回避発動：スローモーション開始 */
			virtual void OnJustDodge() override;
			/** ジャスト回避時のコールバックを登録（BattleManagerが外からセット） */
			void SetJustDodgeCallback(std::function<void()> cb) { onJustDodgeCallback_ = std::move(cb); }

			/** [前フェーズ] 敵の攻撃が当たった直後に呼ぶ。duration 秒以内に回避するとノックバックをキャンセルできる */
			void StartPreJustDodgeWindow(float duration = 0.3f) { preJustDodgeWindowTimer_ = duration; }
			bool IsPreJustDodgeWindowActive() const { return preJustDodgeWindowTimer_ > 0.0f; }
			/** 被ダメージフラグを取得して同時にクリア（回避ステートが呼ぶ） */
			bool ConsumeWasRecentlyKnockedBack() { bool v = wasRecentlyKnockedBack_; wasRecentlyKnockedBack_ = false; return v; }
			/** 無敵状態を設定（BattleManagerが呼ぶ） */
			void SetInvincible(bool b) { isInvincible_ = b; }
			bool IsInvincible() const { return isInvincible_; }

			/** 死んだことを教える */
			void OnDead()
			{
				isDead_ = true;
			}
			bool IsDeadTriggered() const { return isDead_; }

			// 参照と同時にフラグをリセット
			bool CheckAndConsumeKnockBack()
			{
				if (isKnockBack_) {
					isKnockBack_ = false;
					return true;
				}
				return false;
			}

			/** ノックバックしたことを教える */
			void OnKnockBack()
			{
				if (IsEqualCurrentState(KnockBackCharacterState::ID())) {
					return;
				}
				isKnockBack_ = true;
			}

			/** ノックバックしたことを取得 */
			bool GetKnockBack()
			{
				return isKnockBack_;
			}
			/** チャージエフェクトの再生リクエストを出す */
			void RequestGuardEffect()
			{
				isGuard_ = true;
			}

			/** リクエストが来ているか確認し、確認したら自動でフラグを下ろす（1回だけ再生するため） */
			bool OnGuaed()
			{
				if (isGuard_) {
					isGuard_ = false;
					return true;
				}
				return false;
			}

			/** 防御中の押し返し球判定を取得（防御していない間はnullptr） */
			app::collision::GhostBody* GetGuardBlockBody() const { return guardBlockBody_.get(); }

			/** 防御の残り時間割合を取得（1.0=満タン、0.0=制限時間切れ）。
			 *  防御を解いた後も回復し切るまでは1.0未満のままになる */
			float GetGuardRemainingRatio() const
			{
				return max(0.0f, 1.0f - guardHoldTimer_ / kGuardTimeLimit);
			}

			/** 防御ゲージを画面に表示すべきか（防御中、または回復しきっていない間） */
			bool IsGuardGaugeVisible() const
			{
				return IsGuarding() || guardHoldTimer_ > 0.0f;
			}

			/** 防御エフェクト（guard.efk）の球サイズに合わせた半径。TODO: 実機で見た目に合わせて要調整 */
			static constexpr float kGuardBlockRadius = 38.0f;
			/** 切り込みエフェクトしたことを取得 */
			bool IsSlashEffect() const 
			{
				return isSlashEffect_; 
			}
			/** 切り込みエフェクトしたことを設定 */
			void SetSlashEffect(bool flag) 
			{
				isSlashEffect_ = flag; 
			}
			/** チャージエフェクトの再生リクエストを出す */
			void RequestChargeEffect()
			{
				isChargeEffectRequested_ = true;
			}
			/** チャージレベルエフェクトを直接再生する（level: 0=Level1, 1=Level2, 2=Level3） */
			EffectEmitter* RequestChargeLevelEffect(int level)
			{
				if (level < 0 || level >= 3) { return nullptr; }
				const int kinds[3] = {
					enEffectKind_PlayerChargeLevel1,
					enEffectKind_PlayerChargeLevel2,
					enEffectKind_PlayerChargeLevel3
				};

				// ボーン座標を取得してエフェクト位置を決定
				Vector3 effectPos = transform.position;
				effectPos.y += 30.0f; // ボーンが取得できない場合のフォールバック

				auto* model = GetModelRender();
				if (model)
				{
					auto& skeleton = model->GetSkeleton();
					int boneId = skeleton.FindBoneID(L"mixamorig:Spine1");
					if (boneId != -1)
					{
						Quaternion boneRot;
						Vector3 boneScale;
						skeleton.GetBone(boneId)->CalcWorldTRS(effectPos, boneRot, boneScale);
					}
				}

				EffectEmitter* emitter = NewGO<EffectEmitter>(0);
				emitter->Init(kinds[level]);
				emitter->SetPosition(effectPos);
				emitter->SetScale(Vector3(0.5f, 0.5f, 0.5f));
				emitter->Play();
				return emitter;
			}
			/** 溜め攻撃中か */
			bool IsChargeAttacking() const
			{
				return IsEqualCurrentState(ChargeAttackCharacterState::ID());
			}
			/** チャージレベルを設定 */
			void SetChargeLevel(int level) { chargeLevel_ = level; }
			/** チャージレベルを取得 */
			int GetChargeLevel() const { return chargeLevel_; }
			/** 溜め中のリアルタイムレベルを設定 */
			void SetCurrentChargingLevel(int level) { currentChargingLevel_ = level; }
			/** 溜め中のリアルタイムレベルを取得 */
			int GetCurrentChargingLevel() const { return currentChargingLevel_; }

			/** リクエストが来ているか確認し、確認したら自動でフラグを下ろす（1回だけ再生するため） */
			bool CheckAndConsumeChargeEffectRequest()
			{
				if (isChargeEffectRequested_) {
					isChargeEffectRequested_ = false;
					return true;
				}
				return false;
			}
			/** チャージ攻撃エフェクトの再生リクエストを出す */
			void RequestChargeAttackEffect()
			{
				isChargeAttackEffectRequested_ = true;
			}
			/** リクエストが来ているか確認し、確認したら自動でフラグを下ろす（1回だけ再生するため） */
			bool CheckAndConsumeChargeAttackEffectRequest()
			{
				if (isChargeAttackEffectRequested_) {
					isChargeAttackEffectRequested_ = false;
					return true;
				}
				return false;
			}
			/** ポーズ開始時に足音SEを止める */
			void OnPause()
			{
				if (footStepHandle_ != app::INVALID_SOUND_HANDLE)
				{
					app::SoundManager::Get().StopSE(footStepHandle_);
					footStepHandle_ = app::INVALID_SOUND_HANDLE;
				}
				if (InjuredFootStepHandle_ != app::INVALID_SOUND_HANDLE)
				{
					app::SoundManager::Get().StopSE(InjuredFootStepHandle_);
					InjuredFootStepHandle_ = app::INVALID_SOUND_HANDLE;
				}
			}

			/** ポーズ解除時に、走り中なら足音を再開 */
			void OnResume() 
			{
				if (IsEqualCurrentState(RunCharacterState::ID()))
				{
					footStepHandle_ = app::SoundManager::Get().PlaySE(
						static_cast<int>(app::SoundKind::FootStep), true);
				}
				else if (IsEqualCurrentState(InjuredRunCharacterState::ID()))
				{
					InjuredFootStepHandle_ = app::SoundManager::Get().PlaySE(
						static_cast<int>(app::SoundKind::InjuredFootStep), true);
				}
			}
			/** 斬撃1回目か */
			bool IsSlashFirst() const
			{
				return IsEqualCurrentState(SlashFirstCharacterState::ID());
			}
			/** 斬撃2回目か */
			bool isSlashSecond() const
			{
				return IsEqualCurrentState(SlashSecondCharacterState::ID());
			}
		};




		class EventCharacterStateMachine : public CharacterStateMachine
		{
		private:
			using SuperClass = CharacterStateMachine;
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;

			Vector3 targetPosition_ = Vector3::Zero;
			Vector3 chaseDirection_ = Vector3::Zero;
			Vector3 knockBackDirection_ = Vector3::Zero;
			/** AI用のタイマー */
			float aiTimer_ = 0.0f;
			/** 最初の待機時間 */
			const float WAIT_TIME = 1.0f;
			/** 踏まれたか */
			bool isSquashed_ = false;
			/** 視野角に入ったか */
			bool isViewAngle_ = false;
			/** 追いかけているか */
			bool isChasing_ = false;
			/** パンチされたか */
			bool isKnockBack_ = false;

			bool isAttackGhostCreated_ = false;
			/** 攻撃ゴーストが当たった位置（ガード成功時のエフェクト再生などに使用） */
			Vector3 lastAttackGhostPosition_ = Vector3::Zero;

		public:
			EventCharacterStateMachine();
			virtual ~EventCharacterStateMachine();

			virtual void Initialize() override final;
			virtual void Update() override final;

			virtual uint32_t GetCharacterID() const override;

			virtual void OnEnterAttack() override;
			virtual void OnExitAttack() override;
			virtual void OnEnterDead() override;
			virtual void OnExitDead() override;
			virtual void OnEnterKnockBack() override;
			virtual void OnExitKnockBack() override;

		private:
			void UpdateState();

		public:
			/** 外から踏まれたことを教える */
			void OnSquashed()
			{
				isSquashed_ = true;
				aiTimer_ = 0.0f;
			}
			bool IsSquashed() const { return isSquashed_; }

			/** 視野角に入ったことを教える */
			void OnViewAngle(const Vector3& targetPos) {
				isViewAngle_ = true;
				targetPosition_ = targetPos;
			}

			bool IsViewAngle() const { return isViewAngle_; }

			/** 追跡開始を教える */
			void OnChase(const Vector3& direction, const Vector3& targetPos) {
				isChasing_ = true;
				chaseDirection_ = direction;
				targetPosition_ = targetPos;
			}

			/**
			 * DEBUG: 書く場所変更予定だが一旦ここで実装
			 * パンチ食らったことを教える⇒ノックバックに変更したい
			 */
			void OnKnockBack(const Vector3& direction)
			{
				isKnockBack_ = true;
				knockBackDirection_ = direction;
			}
			bool IsKnockBack()
			{
				return isKnockBack_;
			}

			//ゴーストが生成されたことを通知
			void NontifyAttackGhostCreated(const Vector3& hitPosition = Vector3::Zero)
			{
				isAttackGhostCreated_ = true;
				lastAttackGhostPosition_ = hitPosition;
			}


			bool CheckAndConsumeAttackGhostCreated()
			{
				if (isAttackGhostCreated_) {
					isAttackGhostCreated_ = false;
					return true;
				}
				return false;
			}

			/** 直近で攻撃ゴーストが当たった位置を取得（ガード成功エフェクトなどに使用） */
			const Vector3& GetLastAttackGhostPosition() const { return lastAttackGhostPosition_; }
		};



		/******************************************/


		class StoneEventCharacterStateMachine : public CharacterStateMachine
		{
		private:
			using SuperClass = CharacterStateMachine;
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackEffectScheduler_;
			std::mt19937 randomEngine_;

			Vector3 targetPosition_ = Vector3::Zero;
			Vector3 chaseDirection_ = Vector3::Zero;
			Vector3 knockBackDirection_ = Vector3::Zero;
			/** AI用のタイマー */
			float aiTimer_ = 0.0f;
			/** 最初の待機時間 */
			const float WAIT_TIME = 1.0f;
			/** 攻撃クールタイム */
			float attackCoolTimer_ = 0.0f;
			static constexpr float kAttackCoolTime = 1.5f;
			/** 死んだか */
			bool isDead_ = false;
			/** 踏まれたか */
			bool isSquashed_ = false;
			/** 視野角に入ったか */
			bool isViewAngle_ = false;
			/** 追いかけているか */
			bool isChasing_ = false;
			/** パンチされたか */
			bool isKnockBack_ = false;
			/** 溜め攻撃による吹き飛ばしたか */
			bool isBlowBack_ = false;
			/** ノックバック時のチャージレベル */
			int knockBackChargeLevel_ = 0;
			/** ガードで防がれたことによるノックバックか（被弾エフェクトの抑制に使用） */
			bool isGuardBlockedKnockBack_ = false;
			/** ガード反射時の吹き飛ばし速度（0以下ならチャージレベル基準の通常テーブルを使う） */
			float customBlowBackSpeed_ = 0.0f;
			/** 攻撃用のゴーストボディ生成したか */
			bool isAttackGhostCreated_ = false;
			/** 攻撃ゴーストが当たった位置（ガード成功時のエフェクト再生などに使用） */
			Vector3 lastAttackGhostPosition_ = Vector3::Zero;
			/** スポーン直後の待機フラグ */
			bool isJustSpawned_ = true;
			/** 攻撃後の待機ポイントへ戻るフラグ */
			bool isReturningToWait_ = false;
			/** AIの自律移動（パトロール等）を有効にするか */
			bool isAIEnabled_ = true;
			/** とびかかり攻撃中フラグ（EventCharacter側の描画オフセット戻し制御に使用） */
			bool isInPounceAttack_ = false;
			/** とびかかり攻撃の空中（Leap中）攻撃判定がアクティブか。ガードで防いだ時に反射させるかどうかの判定に使用 */
			bool isInAerialPounceAttack_ = false;
			/** とびかかりエフェクトの追従先座標（StonePounceAttackState が毎フレーム更新） */
			Vector3 pounceEffectFollowPos_ = Vector3::Zero;

			/**
			 * kUseWaitPointReservation が false の場合のフォールバック用キャッシュ。
			 * 待機ポイントは毎フレーム、プレイヤーを中心とした円状に再配置されるため、
			 * 自機が2点の境界付近に留まる（ガード等で足止めされている等）と、
			 * 毎フレーム GetNearWaitPoint() を呼ぶたびに最近傍の点が入れ替わり、
			 * 向く方向が高速に反転してしまうことがある。それを避けるための簡易策
			 */
			app::actor::EnemyAttackPoint::AttackPoint* cachedWaitPoint_ = nullptr;

#ifdef K2_DEBUG
			/** デバッグ用：とびかかり攻撃の視野角判定の直近結果（-1=未評価 0=画面外でブロック 1=発動可）。変化した時だけログを出すためのエッジ検出に使う */
			int debugLastPounceDecision_ = -1;
#endif

		public:
			StoneEventCharacterStateMachine();
			virtual ~StoneEventCharacterStateMachine();

			virtual void Initialize() override final;
			virtual void Update() override final;

			virtual uint32_t GetCharacterID() const override;

			virtual void OnEnterAttack() override;
			virtual void OnExitAttack() override;
			virtual void OnEnterDead() override;
			virtual void OnExitDead() override;
			virtual void OnEnterKnockBack() override;
			virtual void OnExitKnockBack() override;
			virtual void OnEnterPounceAttack() override;
			virtual void OnExitPounceAttack() override;

			/** 顔を突き出すフレームに合わせた遅延秒数（要アニメーション確認で調整） */
			virtual float GetGhostBodyDelay() const override { return 0.35f; }
			/** 噛みつき攻撃：顔を突き出す動きに合わせてゴーストボディを前進させるオフセットを返す（要アニメーション確認で調整） */
			virtual float GetAttackLungeDistance(float stateTimer) const override;
			/** ガード中のプレイヤーに防御球（kGuardBlockRadius）より近づく移動かどうかを判定する（true の間は前進を止める） */
			virtual bool IsApproachBlocked() override;

			/**
			 * 敵同士の密集による押し合いで目標方向が毎フレーム入れ替わっても、
			 * 見た目が瞬時に反転しないよう、最大回転速度を設けてなめらかに追従する
			 */
			virtual void ApplyFacingDirection(const Vector3& direction) override;
			/** ApplyFacingDirection の最大回転速度（度/秒） */
			static constexpr float kMaxTurnDegPerSec = 540.0f;

			/**
			 * コリジョン用の半径(15)よりモデルの見た目の方が大きく、噛みつきモーションで
			 * 頭部がさらに前へ出るため、ガード時の停止距離にモデル分の余裕を追加する。
			 * StoneMonster.FBX の頂点バウンディングボックスを実測したところ、
			 * ピボット(transform.position)から頭部側へ最大 約60 ユニット離れていたが、
			 * 実機確認の結果 30 に調整（要実機確認でさらに調整可）
			 */
			static constexpr float kGuardModelMargin = 30.0f;

			/**
			 * 待機ポイントの「予約」システム（1点=1体を保証）を使うかどうか。
			 * 試験導入のためのスイッチ：false にすると、予約なしの単純キャッシュ方式
			 * （このスイッチを追加する前の動作）に即座に戻る
			 */
			static constexpr bool kUseWaitPointReservation = true;

		private:
			void UpdateState();

			/**
			 * 待機ポイントを取得する。kUseWaitPointReservation が true の場合は
			 * EnemyAttackPointManager::AcquireWaitPoint() で予約し、StoneEventCharacter に
			 * 保持させる（1点=1体を保証）。false の場合は cachedWaitPoint_ に単純キャッシュする
			 * （ReleaseCachedWaitPoint() で解放するまで使い回す）
			 */
			app::actor::EnemyAttackPoint::AttackPoint* GetOrPickWaitPoint(app::actor::EnemyAttackPointManager* manager);
			/** 保持中の待機ポイントを解放する（攻撃/とびかかり開始時、待機ポイント検索失敗時に呼ぶ） */
			void ReleaseCachedWaitPoint();

		public:
			/** 死んだことを教える */
			void OnDead()
			{
				isDead_ = true;
			}
			bool IsDeadState() const { return isDead_; }
			/** 外から踏まれたことを教える */
			void OnSquashed()
			{
				isSquashed_ = true;
				aiTimer_ = 0.0f;
			}
			bool IsSquashed() const { return isSquashed_; }

			/** とびかかり攻撃のターゲット位置を取得（StonePounceAttackState が使用） */
			const Vector3& GetTargetPosition() const { return targetPosition_; }
			bool IsInPounceAttack() const { return isInPounceAttack_; }
			/** Bounce/Landing フェーズ中のみ true にセットする（Charge/Leap 中は false） */
			void SetBouncingActive(bool active) { isInPounceAttack_ = active; }

			/** Leap（空中飛行）中の攻撃判定がアクティブか（ガードで防ぐと反射する方の判定） */
			bool IsInAerialPounceAttack() const { return isInAerialPounceAttack_; }
			/** Leapフェーズ中のみ true にセットする（着地したら false） */
			void SetAerialPounceAttackActive(bool active) { isInAerialPounceAttack_ = active; }

			/** とびかかりエフェクトの追従先座標を更新する（StonePounceAttackState が使用） */
			void SetPounceEffectFollowPos(const Vector3& pos) { pounceEffectFollowPos_ = pos; }
			/** とびかかりエフェクトの追従先座標へのポインタを取得する（PlayEffectFollow に渡す） */
			const Vector3* GetPounceEffectFollowPosPtr() const { return &pounceEffectFollowPos_; }

			/** 視野角に入ったことを教える */
			void OnViewAngle(const Vector3& targetPos) {
				/** スポーン直後は視野角に反応しない */
				if (isJustSpawned_) return;

				isViewAngle_ = true;
				targetPosition_ = targetPos;
			}

			bool IsViewAngle() const { return isViewAngle_; }

			/** 追跡開始を教える */
			void OnChase(const Vector3& direction, const Vector3& targetPos) {
				/** スポーン直後は視野角に反応しない */
				if (isJustSpawned_) return;

				isChasing_ = true;
				chaseDirection_ = direction;
				targetPosition_ = targetPos;
			}

			/** AIの自律移動（パトロール等）を有効/無効にする */
			void SetAIEnabled(bool enabled) { isAIEnabled_ = enabled; }

			/**
			 * DEBUG: 書く場所変更予定だが一旦ここで実装
			 * パンチ食らったことを教える⇒ノックバックに変更したい
			 */
			void OnKnockBack(const Vector3& direction, bool isBlowBack = false, int chargeLevel = 0, bool isGuardBlocked = false, float customBlowBackSpeed = 0.0f)
			{
				isKnockBack_ = true;
				knockBackDirection_ = direction;
				isBlowBack_ = isBlowBack;
				knockBackChargeLevel_ = chargeLevel;
				isGuardBlockedKnockBack_ = isGuardBlocked;
				customBlowBackSpeed_ = customBlowBackSpeed;
			}
			bool IsKnockBack()
			{
				return isKnockBack_;
			}
			bool IsBlowBack() const { return isBlowBack_; }
			int GetKnockBackChargeLevel() const { return knockBackChargeLevel_; }
			/** ガードで防がれたことによるノックバックか（被弾エフェクトの抑制に使用） */
			bool IsGuardBlockedKnockBack() const { return isGuardBlockedKnockBack_; }
			/** ガード反射時の吹き飛ばし速度（0以下なら未設定＝通常テーブルを使う） */
			float GetCustomBlowBackSpeed() const { return customBlowBackSpeed_; }

			//ゴーストが生成されたことを通知
			void NontifyAttackGhostCreated(const Vector3& hitPosition = Vector3::Zero)
			{
				isAttackGhostCreated_ = true;
				lastAttackGhostPosition_ = hitPosition;
			}


			bool CheckAndConsumeAttackGhostCreated()
			{
				if (isAttackGhostCreated_) {
					isAttackGhostCreated_ = false;
					return true;
				}
				return false;
			}

			/** 直近で攻撃ゴーストが当たった位置を取得（ガード成功エフェクトなどに使用） */
			const Vector3& GetLastAttackGhostPosition() const { return lastAttackGhostPosition_; }
		};




		/******************************************/


		class MushroomEventCharacterStateMachine : public CharacterStateMachine
		{
		private:
			using SuperClass = CharacterStateMachine;
			app::collision::GhostBody* attackBody_ = nullptr;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackScheduler_;
			std::unique_ptr<app::core::TaskSchedulerSystem> attackEffectScheduler_;
			std::mt19937 randomEngine_;

			Vector3 targetPosition_ = Vector3::Zero;
			Vector3 chaseDirection_ = Vector3::Zero;
			Vector3 knockBackDirection_ = Vector3::Zero;
			/** AI用のタイマー */
			float aiTimer_ = 0.0f;
			/** 最初の待機時間 */
			const float WAIT_TIME = 1.0f;
			float attackCoolTimer_ = 0.0f;
			static constexpr float kAttackCoolTime = 1.5f;
			bool isDead_ = false;
			/** 踏まれたか */
			bool isSquashed_ = false;
			/** 視野角に入ったか */
			bool isViewAngle_ = false;
			/** 追いかけているか */
			bool isChasing_ = false;
			/** パンチされたか */
			bool isKnockBack_ = false;
			/** 溜め攻撃による吹き飛ばしか */
			bool isBlowBack_ = false;
			/** ノックバック時のチャージレベル */
			int knockBackChargeLevel_ = 0;
			/** ガードで防がれたことによるノックバックか（被弾エフェクトの抑制に使用） */
			bool isGuardBlockedKnockBack_ = false;
			/** 攻撃用のゴーストボディ生成したか */
			bool isAttackGhostCreated_ = false;
			/** 攻撃ゴーストが当たった位置（ガード成功時のエフェクト再生などに使用） */
			Vector3 lastAttackGhostPosition_ = Vector3::Zero;
			/** スポーン直後の待機フラグ */
			bool isJustSpawned_ = true;
			/** 攻撃後の待機ポイントへ戻るフラグ */
			bool isReturningToWait_ = false;
			/** AIの自律移動（パトロール等）を有効にするか */
			bool isAIEnabled_ = true;
			/** 毒雲詠唱完了フラグ（BattleManagerが読み取る） */
			bool isPoisonCastComplete_ = false;
			/** 胞子エフェクト再生中フラグ（trueの間はキャスト状態を維持） */
			bool isPoisonSporeActive_ = false;
			/** 毒雲詠唱通知済みフラグ（BattleManagerに通知を1回だけ送るための制御） */
			bool hasPoisonNotified_ = false;
			/** 毒雲詠唱クールタイム */
			float poisonCoolTimer_ = 0.0f;
			/** 詠唱時のターゲット位置（プレイヤー座標） */
			Vector3 poisonCastTargetPos_ = Vector3::Zero;

			/** 毒雲詠唱発動距離（下限：近接攻撃範囲外、上限：中距離） */
			static constexpr float kPoisonMinDist  = 60.0f;
			static constexpr float kPoisonMaxDist  = 150.0f;
			/** 毒雲詠唱クールタイム（秒） */
			static constexpr float kPoisonCoolTime = 8.0f;
			/** プレイヤーの攻撃ヒット判定を受け付けるか */
			bool isReceiveDamageEnabled_ = true;

			/** kUseWaitPointReservation が false の場合のフォールバック用キャッシュ（Stone側と同様の仕組み） */
			app::actor::EnemyAttackPoint::AttackPoint* cachedWaitPoint_ = nullptr;

		public:
			MushroomEventCharacterStateMachine();
			virtual ~MushroomEventCharacterStateMachine();

			virtual void Initialize() override final;
			virtual void Update() override final;

			virtual uint32_t GetCharacterID() const override;

			virtual void OnEnterAttack() override;
			virtual void OnExitAttack() override;
			virtual void OnEnterDead() override;
			virtual void OnExitDead() override;
			virtual void OnEnterKnockBack() override;
			virtual void OnExitKnockBack() override;

			/** 攻撃アニメーション開始からゴーストボディを生成するまでの遅延秒数（要アニメーション確認で調整） */
			virtual float GetGhostBodyDelay() const override { return 0.35f; }

			/**
			 * 待機ポイントの「予約」システム（1点=1体を保証）を使うかどうか。
			 * Stone側と同じ仕組み。Stoneとは独立して on/off できる
			 */
			static constexpr bool kUseWaitPointReservation = true;

		private:
			void UpdateState();

			/** 待機ポイントを取得する（詳細は StoneEventCharacterStateMachine::GetOrPickWaitPoint と同様） */
			app::actor::EnemyAttackPoint::AttackPoint* GetOrPickWaitPoint(app::actor::EnemyAttackPointManager* manager);
			/** 保持中の待機ポイントを解放する */
			void ReleaseCachedWaitPoint();

		public:
			/** 死んだことを教える */
			void OnDead()
			{
				isDead_ = true;
			}
			bool IsDeadState() const { return isDead_; }
			/** 外から踏まれたことを教える */
			void OnSquashed()
			{
				isSquashed_ = true;
				aiTimer_ = 0.0f;
			}
			bool IsSquashed() const { return isSquashed_; }

			/** 視野角に入ったことを教える */
			void OnViewAngle(const Vector3& targetPos) {
				/** スポーン直後は視野角に反応しない */
				if (isJustSpawned_) return;

				isViewAngle_ = true;
				targetPosition_ = targetPos;
			}

			bool IsViewAngle() const { return isViewAngle_; }

			/** 追跡開始を教える */
			void OnChase(const Vector3& direction, const Vector3& targetPos) {
				/** スポーン直後は視野角に反応しない */
				if (isJustSpawned_) return;

				isChasing_ = true;
				chaseDirection_ = direction;
				targetPosition_ = targetPos;
			}

			/** AIの自律移動（パトロール等）を有効/無効にする */
			void SetAIEnabled(bool enabled) { isAIEnabled_ = enabled; }
			/** プレイヤーの攻撃ヒット判定を有効/無効にする */
			void SetReceiveDamageEnabled(bool enabled) { isReceiveDamageEnabled_ = enabled; }
			bool IsReceiveDamageEnabled() const { return isReceiveDamageEnabled_; }

			/**
			 * DEBUG: 書く場所変更予定だが一旦ここで実装
			 * パンチ食らったことを教える⇒ノックバックに変更したい
			 */
			void OnKnockBack(const Vector3& direction, bool isBlowBack = false, int chargeLevel = 0, bool isGuardBlocked = false)
			{
				isKnockBack_ = true;
				knockBackDirection_ = direction;
				isBlowBack_ = isBlowBack;
				knockBackChargeLevel_ = chargeLevel;
				isGuardBlockedKnockBack_ = isGuardBlocked;
			}
			bool IsKnockBack()
			{
				return isKnockBack_;
			}
			bool IsBlowBack() const { return isBlowBack_; }
			int GetKnockBackChargeLevel() const { return knockBackChargeLevel_; }
			/** ガードで防がれたことによるノックバックか（被弾エフェクトの抑制に使用） */
			bool IsGuardBlockedKnockBack() const { return isGuardBlockedKnockBack_; }

			//ゴーストが生成されたことを通知
			void NontifyAttackGhostCreated(const Vector3& hitPosition = Vector3::Zero)
			{
				isAttackGhostCreated_ = true;
				lastAttackGhostPosition_ = hitPosition;
			}


			bool CheckAndConsumeAttackGhostCreated()
			{
				if (isAttackGhostCreated_) {
					isAttackGhostCreated_ = false;
					return true;
				}
				return false;
			}

			/** 直近で攻撃ゴーストが当たった位置を取得（ガード成功エフェクトなどに使用） */
			const Vector3& GetLastAttackGhostPosition() const { return lastAttackGhostPosition_; }

			/** 毒雲詠唱完了を取得（取得と同時にフラグクリア） */
			bool CheckAndConsumePoisonCastComplete()
			{
				if (isPoisonCastComplete_) {
					isPoisonCastComplete_ = false;
					return true;
				}
				return false;
			}

			/** 毒雲詠唱時のターゲット座標を取得 */
			const Vector3& GetPoisonCastTargetPosition() const { return poisonCastTargetPos_; }
			/** 胞子エフェクト終了を BattleManager から通知する */
			void SetPoisonSporeActive(bool v) { isPoisonSporeActive_ = v; }
			/** 詠唱開始時に通知済みフラグをリセットする（MushroomPoisonCastState::Enter が呼ぶ） */
			void ResetPoisonNotified() { hasPoisonNotified_ = false; }
		};
	}
}