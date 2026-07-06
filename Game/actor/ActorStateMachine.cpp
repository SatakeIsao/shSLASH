/**
 * Actorファイル
 */
#include "stdafx.h"
#include "ActorStateMachine.h"
#include "Actor.h"
#include "ActorStatus.h"
#include "BattleCharacter.h"
#include "EventCharacter.h"
#include "effect/EffectManager.h"
#include "sound/SoundManager.h"
#include "battle/BattleManager.h"
#include "camera/CameraManager.h"


namespace
{
	Vector3 ComputeCameraDirection(const Vector3& inputDirection)
	{
		// カメラの前方向と右方向のベクトルを取得
		Vector3 forward = g_camera3D->GetForward();
		Vector3 right = g_camera3D->GetRight();

		// y方向には移動しない
		forward.y = 0.0f;
		right.y = 0.0f;

		// 左スティックの入力量を加算
		right *= inputDirection.x;
		forward *= inputDirection.z;

		Vector3 direction = right + forward;

		direction.Normalize();

		return direction;
	}


	Quaternion ComputeRotation(const Vector3& inputDirection)
	{
		// スティックの方向
		Vector3 direction = ComputeCameraDirection(inputDirection);
		// スティック入力を使ってY軸回転の情報を見る
		Quaternion q;
		q.SetRotationYFromDirectionXZ(direction);

		return q;
	}
}


namespace app
{
	namespace actor
	{
		void IStateMachine::UpdateStateCore()
		{
			//次のステートにしてね。という予約があれば
			if (nextStateId_ != INVALID_STATE_ID && currentStateId_ != nextStateId_)
			{
				// ステート変更
				// 現在のステートを抜ける
				if (currentState_) {
					currentState_->Exit();
				}
				// 次のステートに入る
				currentState_ = std::unique_ptr<ICharacterState>(CreateState(nextStateId_));
				if (currentState_)
				{
					currentState_->Enter();
				}
				currentStateId_ = nextStateId_;
				//次のステートを初期化
				nextStateId_ = INVALID_STATE_ID;
			}
			K2_ASSERT(currentState_ != nullptr, "状態の生成がされません\n");
			currentState_->Update();
		}


		bool IStateMachine::CanChangeState() const
		{
			return currentState_->CanChangeState();
		}




		/************************************/


		CharacterStateMachine::CharacterStateMachine()
		{
			//初期ステート
			//SetCurrentState(RunCharacterState::ID());
			RequestChangeState(IdleCharacterState::ID());
		}

		CharacterStateMachine::~CharacterStateMachine()
		{
		}


		void CharacterStateMachine::Update()
		{
			//ステート更新
			UpdateStateCore();
		}


		void CharacterStateMachine::Move(const float deltaTime, const float moveSpeed)
		{
			// カメラの向きを考慮するか
			Vector3 moveVector = moveDirection_;
			if (isUseCameraDirection_)
			{
				// TODO: 将来的にCharacterControllerを使って衝突判定をする
				moveVector = ComputeCameraDirection(moveDirection_);
			}

			const Vector3 moveSpeedVector = moveVector * moveSpeed;
			if (inputPower_)
			{
				moveSpeedVector_ = moveSpeedVector;
			}
			moveSpeedVector_ *= GetStatus()->GetFriction(); // 摩擦係数的な
			transform.position += moveSpeedVector_ * deltaTime;
		}


		void CharacterStateMachine::Jump(const float jumoPower)
		{
			character_->GetCharacterController()->Jump(jumoPower);
		}


		Character* CharacterStateMachine::GetCharacter()
		{
			return character_;
		}


		app::actor::CharacterStatus* CharacterStateMachine::GetStatus()
		{
			return character_->GetStatus();
		}


		CharacterController* CharacterStateMachine::GetCharacterController()
		{
			return character_->GetCharacterController();
		}


		ModelRender* CharacterStateMachine::GetModelRender()
		{
			return character_->GetModelRender();
		}




		/************************************/


		BattleCharacterStateMachine::BattleCharacterStateMachine()
		{
		}


		BattleCharacterStateMachine::~BattleCharacterStateMachine()
		{
		}


		void BattleCharacterStateMachine::Initialize()
		{
			SuperClass::Initialize();
			isUseCameraDirection_ = true;
			// CharacterSteering が最初に処理するまでは入力ゼロ扱い（Idle を維持するため）
			inputPower_ = 0.0f;
		}


		void BattleCharacterStateMachine::Update()
		{
			// ヒットストップ中は入力・ステート遷移をスキップしアニメーションを停止
			if (hitStopTimer_ > 0.0f)
			{
				GetModelRender()->SetAnimationSpeed(0.0f);
				hitStopTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (hitStopTimer_ <= 0.0f)
				{
					hitStopTimer_ = 0.0f;
					GetModelRender()->SetAnimationSpeed(1.0f);
				}
				SuperClass::Update();
				return;
			}

			UpdateState();
			SuperClass::Update();
		}


		uint32_t BattleCharacterStateMachine::GetCharacterID() const
		{
			return BattleCharacter::ID();
		}


		void BattleCharacterStateMachine::OnEnterKnockBack()
		{
			// エフェクト再生
			EffectManager::Get().PlayEffectFollow(
				enEffectKind_PlayerKnockBack,
				&transform.position,
				Quaternion::Identity,
				Vector3::One
			);
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::KnockBack), 0.1f);
		}


		void BattleCharacterStateMachine::OnExitKnockBack()
		{
		}


		void BattleCharacterStateMachine::OnEnterDead()
		{
			// エフェクト再生
			EffectManager::Get().PlayEffectFollow(
				enEffectKind_PlayerKnockBack,
				&transform.position,
				Quaternion::Identity,
				Vector3::One
			);
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Dead), 0.2f);
			deadSETimer_ = 0.0f;
			isDeadSEPlayed_ = false;
		}


		void BattleCharacterStateMachine::OnExitDead()
		{
		}


		void BattleCharacterStateMachine::OnEnterGuard()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Guard), 0.1f);
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Defence));
		}


		void BattleCharacterStateMachine::OnExitGuard()
		{
		}


		void BattleCharacterStateMachine::OnEnterAvoidance()
		{

			// スティック入力から回避方向を取得（Y成分は不要）
			Vector3 avoidDir = GetMoveDirection();
			avoidDir.y = 0.0f;

			Quaternion effectRot = Quaternion::Identity;

			// 入力がある場合のみ向きを計算
			if (avoidDir.LengthSq() > 0.0001f)
			{
				avoidDir.Normalize();

				// カメラの前方向・右方向を取得（Y成分は除外）
				Vector3 cameraForward = g_camera3D->GetForward();
				Vector3 cameraRight = g_camera3D->GetRight();
				cameraForward.y = 0.0f;
				cameraRight.y = 0.0f;
				cameraForward.Normalize();
				cameraRight.Normalize();

				// スティック入力（カメラ空間）をワールド空間の回避方向に変換
				Vector3 worldDir = cameraRight * avoidDir.x + cameraForward * avoidDir.z;
				worldDir.y = 0.0f;
				worldDir.Normalize();

				// 内積でワールド方向がカメラ前方向（奥）と一致しているか判定
				// 0.7f ≒ cos45度：概ね奥向きなら反転対象とする
				float dot = worldDir.Dot(cameraForward);

				// 奥方向への回避：エフェクトは回避方向と同じ向きに
				if (dot > 0.7f)
				{
					float yaw = atan2f(-worldDir.x, -worldDir.z);
					effectRot.SetRotationY(yaw);
				}
				// 手前・左右への回避：エフェクトは回避方向の逆向きに
				else
				{
					float yaw = atan2f(-worldDir.x, -worldDir.z);
					effectRot.SetRotationY(yaw);
				}
			}
			// 回避エフェクト
			EffectManager::Get().PlayEffectFollow(
				enEffectKind_PlayerAvoidance,
				&transform.position,
				effectRot,
				Vector3::One
			);
			// 砂埃エフェクト
			EffectManager::Get().PlayEffect(
				enEffectKind_PlayerAvoidanceDust,
				transform.position,
				effectRot,
				Vector3::One
			);

			const float blendTime = isAvoidanceCancelFromAttack_ ? 0.2f : 0.1f;
			isAvoidanceCancelFromAttack_ = false;
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::PlayerAnimationKind::Avoidance), blendTime);
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::Avoidance));

			// 前フェーズウィンドウ（ノックバックキャンセル）消費
			preJustDodgeWindowTimer_ = 0.0f;
		}


		void BattleCharacterStateMachine::OnExitAvoidance()
		{
			if (app::battle::BattleManager::IsAvailable())
				app::battle::BattleManager::Get().NotifyAvoidSucceeded();
		}


		void BattleCharacterStateMachine::OnEnterRun()
		{
			footStepHandle_ = app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::FootStep), true);
		}


		void BattleCharacterStateMachine::OnExitRun()
		{
			if (footStepHandle_ == app::INVALID_SOUND_HANDLE)
			{
				return;
			}
			app::SoundManager::Get().StopSE(footStepHandle_);
			footStepHandle_ = app::INVALID_SOUND_HANDLE;
		}


		void BattleCharacterStateMachine::OnEnterInjuredRun()
		{
			InjuredFootStepHandle_ = app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::InjuredFootStep), true);
		}

		void BattleCharacterStateMachine::OnExitInjuredRun()
		{
			if (InjuredFootStepHandle_ == app::INVALID_SOUND_HANDLE)
			{
				return;
			}
			app::SoundManager::Get().StopSE(InjuredFootStepHandle_);
			InjuredFootStepHandle_ = app::INVALID_SOUND_HANDLE;
		}

		void BattleCharacterStateMachine::OnJustDodge()
		{
			// 二重発火防止：ウィンドウを即座に閉じる
			SetJustDodgeWindow(false);

			SetLocalTimeScale(0.2f);
			justDodgeSlowFrames_ = 60;

			app::camera::CameraManager::Get().SetScreenEffectActive(true);
			app::camera::CameraManager::Get().SetScreenEffectFocusWorldPos(transform.position);

			// ジャスト回避エフェクト（OnEnterAvoidanceと同じ回転計算）
			if (EffectManager::IsAvailable())
			{
				Vector3 avoidDir = GetMoveDirection();
				avoidDir.y = 0.0f;

				Quaternion effectRot = Quaternion::Identity;
				if (avoidDir.LengthSq() > 0.0001f)
				{
					avoidDir.Normalize();
					Vector3 cameraForward = g_camera3D->GetForward();
					Vector3 cameraRight   = g_camera3D->GetRight();
					cameraForward.y = 0.0f;
					cameraRight.y   = 0.0f;
					cameraForward.Normalize();
					cameraRight.Normalize();
					Vector3 worldDir = cameraRight * avoidDir.x + cameraForward * avoidDir.z;
					worldDir.y = 0.0f;
					worldDir.Normalize();
					float yaw = atan2f(-worldDir.x, -worldDir.z);
					effectRot.SetRotationY(yaw);
				}

				Vector3 effectPos = transform.position;
				effectPos.y += 30.0f;

				EffectManager::Get().PlayEffect(
					enEffectKind_PlayerAvoidanceJust,
					effectPos,
					effectRot,
					Vector3::One,
					0.2f
				);
			}

			if (onJustDodgeCallback_)
			{
				onJustDodgeCallback_();
			}
		}


		void BattleCharacterStateMachine::OnEnterPunch()
		{
		}

		void BattleCharacterStateMachine::OnExitPunch()
		{
		}

		void BattleCharacterStateMachine::OnEnterPunchSecond()
		{
		}

		void BattleCharacterStateMachine::OnExitPunchSecond()
		{
		}

		void BattleCharacterStateMachine::OnEnterPunchThird()
		{
		}

		void BattleCharacterStateMachine::OnExitPunchThird()
		{
			if (app::battle::BattleManager::IsAvailable())
				app::battle::BattleManager::Get().NotifyComboAttackCompleted();
		}

		void BattleCharacterStateMachine::UpdateState()
		{
			// ジャスト回避前ウィンドウタイマー更新
			if (preJustDodgeWindowTimer_ > 0.0f)
			{
				preJustDodgeWindowTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (preJustDodgeWindowTimer_ < 0.0f) preJustDodgeWindowTimer_ = 0.0f;
			}

			// 通常攻撃1段目クールダウン更新
			if (slashFirstCooldownTimer_ > 0.0f)
			{
				slashFirstCooldownTimer_ -= g_gameTime->GetFrameDeltaTime();
				if (slashFirstCooldownTimer_ < 0.0f) slashFirstCooldownTimer_ = 0.0f;
			}

			// ジャスト回避スローモーションのフレームカウントダウン
			if (justDodgeSlowFrames_ > 0)
			{
				--justDodgeSlowFrames_;
				app::camera::CameraManager::Get().SetScreenEffectFocusWorldPos(transform.position);
				if (justDodgeSlowFrames_ == 0)
				{
					SetLocalTimeScale(1.0f);
					app::camera::CameraManager::Get().SetScreenEffectActive(false);
				}
			}

			// このフレームで既に遷移リクエストを送ったかどうかのフラグ
			bool isStateRequested = false;

			// 死亡・起き上がり判定
			{
#if defined(APP_DEBUG)
				bool isRB2Trigger = g_pad[0]->IsTrigger(enButtonRB2);
				bool isXTrigger = g_pad[0]->IsPress(enButtonX);
#endif // APP_DEBUG

				if (IsEqualCurrentState(DeadCharacterState::ID()))
				{
					// 遅延SE再生
					if (!isDeadSEPlayed_)
					{
						constexpr float SE_DELAY = 1.3f;
						deadSETimer_ += g_gameTime->GetFrameDeltaTime();
						if (deadSETimer_ >= SE_DELAY)
						{
							app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::DeadPlayer));
							isDeadSEPlayed_ = true;
						}
					}
#if defined(APP_DEBUG)
					if (isXTrigger)
					{
						RequestChangeState(KipUpCharacterState::ID());
						isStateRequested = true; // 遷移決定
					}
#endif // APP_DEBUG
					if (!isStateRequested) return; // 遷移しないならDead維持。遷移するなら下へ。
				}
				else if (isDead_)
				{
					RequestChangeState(DeadCharacterState::ID());
					isStateRequested = true;
				}
#if defined(APP_DEBUG)
				else if (isRB2Trigger)
				{
					RequestChangeState(DeadCharacterState::ID());
					isStateRequested = true;
				}
#endif // APP_DEBUG
			}

			// 既に遷移が決まっているなら、以下の他のステート判定（移動など）をすべてスキップする
			if (isStateRequested) return;

#if defined(APP_DEBUG)
			// --- KipUp中の待機 ---
			if (IsEqualCurrentState(KipUpCharacterState::ID()))
			{
				if (!CanChangeState()) return;
			}
#endif // APP_DEBUG
			//ノックバック
			{
				// [前フェーズ] ウィンドウ内にY入力があればノックバックをキャンセルして通常回避へ（被ダメージ中のためジャスト回避なし）
				if (isKnockBack_ && preJustDodgeWindowTimer_ > 0.0f && IsTriggerY())
				{
					isKnockBack_ = false;
					preJustDodgeWindowTimer_ = 0.0f;
					wasRecentlyKnockedBack_ = true;
					RequestChangeState(AvoidanceCharacterState::ID());
					return;
				}

				if (isKnockBack_) {
					RequestChangeState(KnockBackCharacterState::ID());
					isKnockBack_ = false;
					wasRecentlyKnockedBack_ = true;
					return;
				}

				if (IsEqualCurrentState(KnockBackCharacterState::ID()))
				{
					if (CanChangeState())
					{
						if (GetStatus()->IsInjured())
						{
							RequestChangeState(InjuredIdleCharacterState::ID());
						}
						else {
							RequestChangeState(IdleCharacterState::ID());
						}
					}
					return;
				}
			}
			// ワープ
			{
				if (IsRequestWarp()) {
					RequestChangeState(WarpInCharacterState::ID());
					ClearRequestWarp();
					return;
				}
				if (IsEqualCurrentState(WarpInCharacterState::ID())) {
					if (CanChangeState()) {
						RequestChangeState(WarpOutCharacterState::ID());
						return;
					}
					else {
						return;
					}
				}
				if (IsEqualCurrentState(WarpOutCharacterState::ID()))
				{
					if (!CanChangeState()) {
						return;
					}
				}
			}
			// ジャンプ
			{
				if (IsActionA()) {
					RequestChangeState(ChargeAttackCharacterState::ID());
					//isPressA_ = false;
					return;
				}
				// パンチ中は他の状態に遷移しない
				if (IsEqualCurrentState(ChargeAttackCharacterState::ID())) {
					if (!CanChangeState()) {
						if (IsTriggerY()) {
							isAvoidanceCancelFromAttack_ = true;
							RequestChangeState(AvoidanceCharacterState::ID());
						}
						return;
					}
				}
			}
			// 攻撃
			{
				// 攻撃中に回避入力があれば攻撃をキャンセルして回避へ
				if (IsTriggerY()
					&& (IsEqualCurrentState(SlashFirstCharacterState::ID())
						|| IsEqualCurrentState(SlashSecondCharacterState::ID())
						|| IsEqualCurrentState(SlashThirdCharacterState::ID())))
				{
					isAvoidanceCancelFromAttack_ = true;
					RequestChangeState(AvoidanceCharacterState::ID());
					return;
				}

				if (IsActionB() && slashFirstCooldownTimer_ <= 0.0f) {
					//  コンボ判定を通常Punchへの遷移のみここで行う
					if (!IsEqualCurrentState(SlashFirstCharacterState::ID())
						&& !IsEqualCurrentState(SlashSecondCharacterState::ID())
						&& !IsEqualCurrentState(SlashThirdCharacterState::ID()))
					{
						RequestChangeState(SlashFirstCharacterState::ID());
						isSlashEffect_ = true;
						return;
					}
				}

				// PunchState内でコンボ入力済みなら PunchSecond へ
				if (IsEqualCurrentState(SlashFirstCharacterState::ID()))
				{
					auto* state = static_cast<SlashFirstCharacterState*>(GetCurrentState());
					if (state->IsComboInput() && CanChangeState())
					{
						RequestChangeState(SlashSecondCharacterState::ID());
						isSlashEffect_ = true;
						return;
					}
					if (!CanChangeState()) {
						return;
					}
					// コンボなしで1段目終了 → 再使用クールダウン開始
					slashFirstCooldownTimer_ = 0.8f;
				}

				// PunchState内でコンボ入力済みなら PunchSecond へ
				if (IsEqualCurrentState(SlashSecondCharacterState::ID()))
				{
					auto* state = static_cast<SlashSecondCharacterState*>(GetCurrentState());
					if (state->IsComboInput() && CanChangeState())
					{
						RequestChangeState(SlashThirdCharacterState::ID());
						isSlashEffect_ = true;
						return;
					}
					if (!CanChangeState()) {
						return;
					}
				}

				// PunchSecond中はアニメーション終了まで何もさせない
				if (IsEqualCurrentState(SlashSecondCharacterState::ID())
					|| IsEqualCurrentState(SlashThirdCharacterState::ID()))
				{
					if (!CanChangeState()) {
						return;
					}
				}
			}
			// 落下
			{
				if (!GetCharacterController()->IsOnGround()) {
					if (!IsEqualCurrentState(FallingCharacterState::ID())) {
						RequestChangeState(FallingCharacterState::ID());
					}
					return;
				}
			}
			// 防御
			{
				if (IsActionLB1()
					|| IsActionRB1())
				{
					RequestChangeState(GuardCharacterState::ID());
					return;
				}
			}
			// 回避
			{
				if (IsTriggerY())
				{
					RequestChangeState(AvoidanceCharacterState::ID());
					return;
				}
				// パンチ中は他の状態に遷移しない
				if (IsEqualCurrentState(AvoidanceCharacterState::ID())) {
					if (!CanChangeState()) {
						return;
					}
				}
			}
			// 体力ない時
			{
				///** デバッグテスト */
				//if (g_pad[0]->IsPress(enButtonLB2))
				//{
				//	RequestChangeState(InjuredIdleCharacterState::ID());
				//	return;
				//}
			}

#if defined(APP_DEBUG)
			const bool isLB2Pressed = g_pad[0]->IsPress(enButtonLB2);
#else
			const bool isLB2Pressed = false;
#endif // APP_DEBUG
			// 負傷判定
			const bool isInjured = GetStatus()->IsInjured();
			const Vector3 direction = moveSpeedVector_;
			if (direction.LengthSq() >= MOVE_MIN_FLOAT || inputPower_ >= MOVE_MIN_FLOAT)
			{
				// オープニングシーケンス（3/2/1/START）が終わるまで走りに遷移しない
				const bool isOpeningDone =
					!app::battle::BattleManager::IsAvailable() ||
					app::battle::BattleManager::Get().IsOpeningSequenceDone();
				if (!isOpeningDone)
				{
					RequestChangeState(IdleCharacterState::ID());
					return;
				}

				if (isLB2Pressed || isInjured)
				{
					// LB2を押しながら移動入力がある場合：負傷走りへ
					RequestChangeState(InjuredRunCharacterState::ID());
				}
				else
				{
					// 通常の移動入力がある場合：走りへ
					RequestChangeState(RunCharacterState::ID());
				}
				return;
			}
			else
			{
				if (isLB2Pressed || isInjured)
				{
					// 移動入力がなく、LB2だけ押されている場合：負傷待機へ
					RequestChangeState(InjuredIdleCharacterState::ID());
					//app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::HpDanger),false);
					return;
				}
			}

			RequestChangeState(IdleCharacterState::ID());
		}




		/************************************/


		EventCharacterStateMachine::EventCharacterStateMachine()
		{
		}


		EventCharacterStateMachine::~EventCharacterStateMachine()
		{
		}


		void EventCharacterStateMachine::Initialize()
		{
			SuperClass::Initialize();
			SetCurrentState(INVALID_STATE_ID);
			RequestChangeState(IdleCharacterState::ID());

			isUseCameraDirection_ = false;
		}


		void EventCharacterStateMachine::Update()
		{
			UpdateState();
			SuperClass::Update();
		}


		void EventCharacterStateMachine::OnEnterDead()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::SlimeAnimationKind::Dead));
			// ぺっちゃんこ
			transform.scale = Vector3(1.0f, 0.1f, 1.0f);
		}


		void EventCharacterStateMachine::OnExitDead()
		{
			// 元に戻す
			transform.scale = Vector3::One;
		}


		void EventCharacterStateMachine::OnEnterKnockBack()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::SlimeAnimationKind::knockBack));
			Jump(80.0f);
		}


		void EventCharacterStateMachine::OnExitKnockBack()
		{
		}


		uint32_t EventCharacterStateMachine::GetCharacterID() const
		{
			return EventCharacter::ID();
		}


		void EventCharacterStateMachine::OnEnterAttack()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::SlimeAnimationKind::Attack));
		}


		void EventCharacterStateMachine::OnExitAttack()
		{
		}


		void EventCharacterStateMachine::UpdateState()
		{
			/** TODO: スライムの頭上にPlayerの当たり判定が衝突したらぺっちゃんこ */
			if (IsSquashed())
			{
				RequestChangeState(DeadCharacterState::ID());

				// Deadステート側で2秒経過して「遷移可能」になったらIdleに戻す
				if (IsEqualCurrentState(DeadCharacterState::ID())
					&& CanChangeState())
				{
					isSquashed_ = false;
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}

			/** TODO: パンチ食らったらknockBack */
			if (isKnockBack_)
			{
				//ノックバックする方向を設定
				SetMoveDirection(knockBackDirection_);
				RequestChangeState(KnockBackCharacterState::ID());

				if (IsEqualCurrentState(KnockBackCharacterState::ID())
					&& CanChangeState())
				{
					aiTimer_ = 0.0f;
					isKnockBack_ = false;
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}

			if (IsEqualCurrentState(AttackCharacterState::ID()))
			{
				if (CanChangeState()) {
					aiTimer_ = 0.0f;
					RequestChangeState(RunCharacterState::ID());
				}
				return;
			}

			/** TODO: 敵の視野角にPlayerが入ったら追従して */
			if (isChasing_)
			{
				isChasing_ = false;

				if (IsEqualCurrentState(IdleCharacterState::ID())
					&& aiTimer_ > 5.0f)
				{

				}
				else {
					Vector3 toPlayer = targetPosition_ - transform.position;
					toPlayer.y = 0.0f;
					float distance = toPlayer.Length();

					if (distance <= 40.0f) {
						SetMoveDirection(chaseDirection_);
						RequestChangeState(AttackCharacterState::ID());
					}
					else if (distance <= 200.0f) {
						SetMoveDirection(chaseDirection_);
						RequestChangeState(RunCharacterState::ID());
						aiTimer_ = 5.0f;
					}
					/** DEBUG: いらないかも */
					else
					{
						RequestChangeState(RunCharacterState::ID());
						aiTimer_ = 0.0f;
					}
					return;
				}
			}

			app::actor::BattleCharacter* player = nullptr;

			/** デバッグテスト: 待機⇒左に走る⇒右に走る⇒待機のモーション */
			if (IsEqualCurrentState(IdleCharacterState::ID()))
			{
				aiTimer_ += g_gameTime->GetFrameDeltaTime();
				if (aiTimer_ > WAIT_TIME)
				{
					RequestChangeState(RunCharacterState::ID());
					aiTimer_ = 0.0f;
				}
				return;
			}

			if (IsEqualCurrentState(RunCharacterState::ID()))
			{
				aiTimer_ += g_gameTime->GetFrameDeltaTime();

				if (aiTimer_ <= 2.0f)
				{
					SetMoveDirection(Vector3::Left);
				}
				else if (aiTimer_ <= 4.0f)
				{
					SetMoveDirection(Vector3::Right);
				}
				else {
					SetMoveDirection(Vector3::Zero);
					RequestChangeState(IdleCharacterState::ID());
					aiTimer_ = 0.0f;
				}
				return;
			}

			RequestChangeState(IdleCharacterState::ID());
		}




		/************************************/

		StoneEventCharacterStateMachine::StoneEventCharacterStateMachine()
		{
		}


		StoneEventCharacterStateMachine::~StoneEventCharacterStateMachine()
		{
		}


		void StoneEventCharacterStateMachine::Initialize()
		{
			SuperClass::Initialize();
			SetCurrentState(INVALID_STATE_ID);
			RequestChangeState(IdleCharacterState::ID());

			isUseCameraDirection_ = false;
			// フラグを全リセット
			isKnockBack_ = false;
			isDead_ = false;
			isChasing_ = false;
			isBlowBack_ = false;
			knockBackDirection_ = Vector3::Zero;
			aiTimer_ = 0.0f;
			isJustSpawned_ = true;

			// スポーン時の向きをランダムに設定
			randomEngine_.seed(std::random_device{}());
			float randomYaw = (randomEngine_() % 360) * (Math::PI / 180.0f);
			transform.rotation.SetRotationY(randomYaw);

			isReturningToWait_ = false;
		}


		void StoneEventCharacterStateMachine::Update()
		{
			UpdateState();
			// 攻撃エフェクトの遅延再生を更新
			if (attackEffectScheduler_)
			{
				attackEffectScheduler_->Update(g_gameTime->GetFrameDeltaTime());
			}
			SuperClass::Update();
		}


		void StoneEventCharacterStateMachine::OnEnterAttack()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::StoneAnimationKind::Attack));

			// 攻撃方向を取得（ワールド空間）
			Vector3 attackDir = GetMoveDirection();
			attackDir.y = 0.0f;

			// 攻撃方向に合わせた回転を計算
			Quaternion effectRot = Quaternion::Identity;
			if (attackDir.LengthSq() > 0.0001f)
			{
				attackDir.Normalize();
				float yaw = atan2f(attackDir.x, attackDir.z);
				effectRot.SetRotationY(yaw);
			}

			// 攻撃方向にオフセットした座標
			const float offset = 20.0f;
			Vector3 effectPos = transform.position + attackDir * offset;

			// 攻撃予測エフェクト（即時再生）
			if (EffectManager::IsAvailable())
			{
				EffectManager::Get().PlayEffect(
					enEffectKind_StonePredictionAtk,
					effectPos,
					effectRot,
					Vector3::One,
					2.0f
				);
			}

			// エフェクト再生
			attackEffectScheduler_ = std::make_unique<app::core::TaskSchedulerSystem>();
			attackEffectScheduler_->AddTimer(0.35f, [effectPos, effectRot]()
				{
					EffectManager::Get().PlayEffect(
						enEffectKind_StoneAttack,
						effectPos,
						effectRot,
						Vector3::One
					);
				}, false);

			// アタックポイントの確保。確保済みなら二重確保しない
			auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
			if (stone && stone->GetCurrentAttackPoint() == nullptr)
			{
				auto* manager = stone->GetAttackPointManager();
				if (manager)
				{
					auto* point = manager->AcquireAttackPoint(transform.position, stone);
					stone->SetCurrentAttackPoint(point);
				}
			}
		}

		void StoneEventCharacterStateMachine::OnExitAttack()
		{
			// 攻撃ステートを正常に抜ける時はアタックポイントを解放する
			auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
			if (stone)
			{
				auto* manager = stone->GetAttackPointManager();
				if (manager && stone->GetCurrentAttackPoint() != nullptr)
				{
					manager->ReleaseAttackPoint(stone->GetCurrentAttackPoint(), stone);
					stone->SetCurrentAttackPoint(nullptr);
				}

				if (manager)
				{
					auto* waitPoint = manager->GetNearWaitPoint(transform.position);
					if (waitPoint != nullptr)
					{
						Vector3 toWait = waitPoint->position_ - transform.position;
						toWait.Normalize();
						SetMoveDirection(toWait);
					}
				}
			}

			attackCoolTimer_ = kAttackCoolTime;
		}


		void StoneEventCharacterStateMachine::OnEnterDead()
		{
			// エフェクト再生
			EffectManager::Get().PlayEffect(
				enEffectKind_StoneKnockBack,
				transform.position,
				Quaternion::Identity,
				Vector3::One
			);
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::StoneAnimationKind::Dead));
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::DeadStone));

			auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
			if (stone)
			{
				stone->GetGhostBody()->SetActive(false);

				// CharacterControllerの剛体もデバッグ描画から隠す
				btRigidBody* ccBody = stone->GetCharacterController()->GetRigidBody()->GetBody();
				ccBody->setCollisionFlags(ccBody->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

				auto* manager = stone->GetAttackPointManager();
				if (manager && stone->GetCurrentAttackPoint() != nullptr)
				{
					manager->ReleaseAttackPoint(stone->GetCurrentAttackPoint(), stone);
					stone->SetCurrentAttackPoint(nullptr);
				}
			}
		}


		void StoneEventCharacterStateMachine::OnExitDead()
		{
		}


		void StoneEventCharacterStateMachine::OnEnterKnockBack()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::StoneAnimationKind::KnockBack));
			SetMoveDirection(knockBackDirection_);
			// Jump は KnockBackCharacterState のヒットストップ終了後に呼ばれる
			isBlowBack_ = false;
			// エフェクト再生
			EffectManager::Get().PlayEffectFollow(
				enEffectKind_StoneKnockBack,
				&transform.position,
				Quaternion::Identity,
				Vector3::One
			);

			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::KnockbackStone), false);

			auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
			if (stone)
			{
				auto* manager = stone->GetAttackPointManager();
				if (manager && stone->GetCurrentAttackPoint() != nullptr)
				{
					manager->ReleaseAttackPoint(stone->GetCurrentAttackPoint(), stone);
					stone->SetCurrentAttackPoint(nullptr);
				}
			}
		}


		void StoneEventCharacterStateMachine::OnExitKnockBack()
		{
		}


		uint32_t StoneEventCharacterStateMachine::GetCharacterID() const
		{
			return StoneEventCharacter::ID();
		}


		void StoneEventCharacterStateMachine::UpdateState()
		{
			if (attackCoolTimer_ > 0.0f)
			{
				attackCoolTimer_ -= g_gameTime->GetFrameDeltaTime();
			}

			// 死亡は最優先（スポーン猶予中でも即処理）
			if (isDead_)
			{
				RequestChangeState(DeadCharacterState::ID());

				// Dead アニメーション終了後：床デカールと同時にフェードアウト開始、完了後に削除通知
				if (IsEqualCurrentState(DeadCharacterState::ID())
					&& CanChangeState())
				{
					auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
					if (stone)
					{
						stone->StartFadeOut();
						stone->NotifyDeadEffect();
						if (stone->IsFadeOutComplete())
						{
							isDead_ = false;
							stone->NotifyDead();
						}
					}
				}
				return;
			}

			/** スポーン直後、一定時間はAIを停止 */
			if (isJustSpawned_)
			{
				static const float SPAWN_WAIT_TIME = 1.5f;
				aiTimer_ += g_gameTime->GetFrameDeltaTime();
				if (aiTimer_ > SPAWN_WAIT_TIME)
				{
					isJustSpawned_ = false;
					aiTimer_ = 0.0f;
				}
				return;
			}

			if (IsSquashed())
			{
				RequestChangeState(DeadCharacterState::ID());

				// Deadステート側で2秒経過して「遷移可能」になったらIdleに戻す
				if (IsEqualCurrentState(DeadCharacterState::ID())
					&& CanChangeState())
				{
					isSquashed_ = false;
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}


			if (isKnockBack_)
			{
				//ノックバックする方向を設定
				SetMoveDirection(knockBackDirection_);
				RequestChangeState(KnockBackCharacterState::ID());

				if (IsEqualCurrentState(KnockBackCharacterState::ID())
					&& CanChangeState())
				{
					aiTimer_ = 0.0f;
					isKnockBack_ = false;
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}

			if (IsEqualCurrentState(AttackCharacterState::ID()))
			{
				if (CanChangeState()) {
					aiTimer_ = 0.0f;
					isReturningToWait_ = true;
					RequestChangeState(RunCharacterState::ID());
				}
				return;
			}

			if (isReturningToWait_)
			{
				auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
				if (stone && stone->GetAttackPointManager())
				{
					auto* waitPoint = stone->GetAttackPointManager()->GetNearWaitPoint(transform.position);
					if (waitPoint != nullptr)
					{
						Vector3 toWait = waitPoint->position_ - transform.position;
						const float distToWait = toWait.Length();

						if (distToWait > 30.0f)
						{
							// まだ待機ポイントに着いていない → 向かい続ける
							toWait.Normalize();
							SetMoveDirection(toWait);
							RequestChangeState(RunCharacterState::ID());
							return;
						}
						else
						{
							// 待機ポイントに到達 → 通常の追跡状態に戻す
							isReturningToWait_ = false;
							SetMoveDirection(Vector3::Zero);
							RequestChangeState(IdleCharacterState::ID());
							return;
						}
					}
					else
					{
						// 待機ポイントが取れなければそのままフラグを解除
						isReturningToWait_ = false;
					}
				}
				else
				{
					isReturningToWait_ = false;
				}
			}

			/** TODO: 敵の視野角にPlayerが入ったら追従して */
			if (isChasing_)
			{
				isChasing_ = false;

				auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
				if (stone && stone->GetAttackPointManager())
				{
					const bool canAttack = stone->GetAttackPointManager()->IsUseable() && attackCoolTimer_ <= 0.0f;
					if (!canAttack)
					{
						auto* waitPoint = stone->GetAttackPointManager()->GetNearWaitPoint(transform.position);
						if (waitPoint != nullptr)
						{
							Vector3 toWait = waitPoint->position_ - transform.position;
							toWait.Normalize();
							SetMoveDirection(toWait);
						}
					}
				}

				if (IsEqualCurrentState(IdleCharacterState::ID()) && aiTimer_ > 5.0f) {}

				else
				{
					Vector3 toPlayer = targetPosition_ - transform.position;
					toPlayer.y = 0.0f;
					float distance = toPlayer.Length();

					if (distance <= 40.0f)
					{
						auto* manager = stone->GetAttackPointManager();

						if (attackCoolTimer_ <= 0.0f)
						{
							manager->RequestAttackToken(stone);
						}

						const bool isMyTurn = manager->ConsumeAttackToken(stone);
						if (isMyTurn && manager->IsUseable())
						{
							SetMoveDirection(chaseDirection_);
							RequestChangeState(AttackCharacterState::ID());
						}
						else
						{
							auto* waitPoint = manager->GetNearWaitPoint(transform.position);
							if (waitPoint != nullptr)
							{
								Vector3 toWait = waitPoint->position_ - transform.position;
								toWait.Normalize();
								SetMoveDirection(toWait);
							}
							RequestChangeState(RunCharacterState::ID());
						}
					}
					else if (distance <= 400.0f)
					{
						auto* stone = dynamic_cast<app::actor::StoneEventCharacter*>(GetCharacter());
						if (stone && stone->GetAttackPointManager())
						{
							const bool canAttack = stone->GetAttackPointManager()->IsUseable() && attackCoolTimer_ <= 0.0f;

							if (!canAttack)
							{
								auto* waitPoint = stone->GetAttackPointManager()->GetNearWaitPoint(transform.position);
								if (waitPoint != nullptr)
								{
									Vector3 toWait = waitPoint->position_ - transform.position;
									const float distanceToWait = toWait.Length();
									if (distanceToWait < 30.0f)
									{
										SetMoveDirection(toWait);
										RequestChangeState(IdleCharacterState::ID());
										aiTimer_ = 0.0f;
										return;
									}
									toWait.Normalize();
									SetMoveDirection(toWait);
								}
								RequestChangeState(RunCharacterState::ID());
								aiTimer_ = 5.0f;
								return;
							}
						}
						SetMoveDirection(chaseDirection_);
						RequestChangeState(RunCharacterState::ID());
						aiTimer_ = 5.0f;
					}
					/** DEBUG: いらないかも */
					else
					{
						RequestChangeState(RunCharacterState::ID());
						aiTimer_ = 0.0f;
					}
					return;
				}
			}

			app::actor::BattleCharacter* player = nullptr;

			/** デバッグテスト: 待機⇒左に走る⇒右に走る⇒待機のモーション */
			if (IsEqualCurrentState(IdleCharacterState::ID()))
			{
				if (isAIEnabled_)
				{
					aiTimer_ += g_gameTime->GetFrameDeltaTime();

					/** 通常の待機 */
					if (aiTimer_ > WAIT_TIME)
					{
						RequestChangeState(PatrolCharacterState::ID());
						aiTimer_ = 0.0f;
					}
				}
				return;
			}

			if (IsEqualCurrentState(PatrolCharacterState::ID()))
			{
				if (CanChangeState())
				{
					SetMoveDirection(Vector3::Zero);
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}

			if (IsEqualCurrentState(RunCharacterState::ID()))
			{
				aiTimer_ += g_gameTime->GetFrameDeltaTime();

				if (aiTimer_ <= 2.0f)
				{
					SetMoveDirection(Vector3::Left);
				}
				else if (aiTimer_ <= 4.0f)
				{
					SetMoveDirection(Vector3::Right);
				}
				else {
					SetMoveDirection(Vector3::Zero);
					RequestChangeState(IdleCharacterState::ID());
					aiTimer_ = 0.0f;
				}
				return;
			}

			RequestChangeState(IdleCharacterState::ID());
		}




		/************************************/

		MushroomEventCharacterStateMachine::MushroomEventCharacterStateMachine()
		{
		}


		MushroomEventCharacterStateMachine::~MushroomEventCharacterStateMachine()
		{
		}


		void MushroomEventCharacterStateMachine::Initialize()
		{
			SuperClass::Initialize();
			SetCurrentState(INVALID_STATE_ID);
			RequestChangeState(IdleCharacterState::ID());
			isUseCameraDirection_ = false;
			// フラグを全リセット
			isKnockBack_ = false;
			isDead_ = false;
			isChasing_ = false;
			isBlowBack_ = false;
			knockBackDirection_ = Vector3::Zero;
			aiTimer_ = 0.0f;
			isJustSpawned_ = true;
			isReceiveDamageEnabled_ = true;

			// スポーン時の向きをランダムに設定
			randomEngine_.seed(std::random_device{}());
			float randomYaw = (randomEngine_() % 360) * (Math::PI / 180.0f);
			transform.rotation.SetRotationY(randomYaw);

			isReturningToWait_ = false;

			// 毒雲詠唱フィールドをリセット
			isPoisonCastComplete_ = false;
			hasPoisonNotified_    = false;
			poisonCoolTimer_      = 0.0f;
			poisonCastTargetPos_  = Vector3::Zero;
		}


		void MushroomEventCharacterStateMachine::Update()
		{
			UpdateState();

			// 攻撃エフェクトの遅延再生を更新
			if (attackEffectScheduler_)
			{
				attackEffectScheduler_->Update(g_gameTime->GetFrameDeltaTime());
			}

			SuperClass::Update();
		}


		void MushroomEventCharacterStateMachine::OnEnterAttack()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::MushroomAnimationKind::Attack));
			// 攻撃方向を取得（ワールド空間）
			Vector3 attackDir = GetMoveDirection();
			attackDir.y = 0.0f;

			// 攻撃方向に合わせた回転を計算
			Quaternion effectRot = Quaternion::Identity;
			if (attackDir.LengthSq() > 0.0001f)
			{
				attackDir.Normalize();
				float yaw = atan2f(attackDir.x, attackDir.z);
				effectRot.SetRotationY(yaw);
			}

			// 攻撃方向にオフセットした座標にエフェクトを設置
			const float offset = 20.0f;
			Vector3 effectPos = transform.position + attackDir * offset;

			// 攻撃予測エフェクト（即時再生）
			if (EffectManager::IsAvailable())
			{
				EffectManager::Get().PlayEffect(
					enEffectKind_MushroomPredictionAtk,
					effectPos,
					effectRot,
					Vector3::One,
					2.0f
				);
			}

			// 0.5秒後にエフェクト再生
			attackEffectScheduler_ = std::make_unique<app::core::TaskSchedulerSystem>();
			attackEffectScheduler_->AddTimer(0.55f, [effectPos, effectRot]()
				{
					EffectManager::Get().PlayEffect(
						enEffectKind_MushroomAttack,
						effectPos,
						effectRot,
						Vector3::One
					);
				}, false);

			auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
			if (mushroom && mushroom->GetAttackPointManager())
			{
				auto* manager = mushroom->GetAttackPointManager();
				if (manager)
				{
					auto* point = manager->AcquireAttackPoint(transform.position, mushroom);
					mushroom->SetCurrentAttackPoint(point);
				}
			}
		}


		void MushroomEventCharacterStateMachine::OnExitAttack()
		{
			// 攻撃ステートを正常に抜ける時はアタックポイントを解放する
			auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
			if (mushroom)
			{
				auto* manager = mushroom->GetAttackPointManager();
				if (manager && mushroom->GetCurrentAttackPoint() != nullptr)
				{
					manager->ReleaseAttackPoint(mushroom->GetCurrentAttackPoint(), mushroom);
					mushroom->SetCurrentAttackPoint(nullptr);
				}
			}

			attackCoolTimer_ = kAttackCoolTime;
		}


		void MushroomEventCharacterStateMachine::OnEnterDead()
		{
			// エフェクト再生
			EffectManager::Get().PlayEffectFollow(
				enEffectKind_MushroomKnockBack,
				&transform.position,
				Quaternion::Identity,
				Vector3::One
			);
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::MushroomAnimationKind::Dead));
			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::DeadMushroom));

			auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
			if (mushroom)
			{
				mushroom->GetGhostBody()->SetActive(false);

				// CharacterControllerの剛体もデバッグ描画から隠す
				btRigidBody* ccBody = mushroom->GetCharacterController()->GetRigidBody()->GetBody();
				ccBody->setCollisionFlags(ccBody->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

				auto* manager = mushroom->GetAttackPointManager();
				if (manager && mushroom->GetCurrentAttackPoint() != nullptr)
				{
					manager->ReleaseAttackPoint(mushroom->GetCurrentAttackPoint(), mushroom);
					mushroom->SetCurrentAttackPoint(nullptr);
				}
			}
		}


		void MushroomEventCharacterStateMachine::OnExitDead()
		{
		}


		void MushroomEventCharacterStateMachine::OnEnterKnockBack()
		{
			GetModelRender()->PlayAnimation(static_cast<uint8_t>(app::actor::MushroomAnimationKind::KnockBack));
			SetMoveDirection(knockBackDirection_);
			// Jump は KnockBackCharacterState のヒットストップ終了後に呼ばれる
			isBlowBack_ = false;
			// エフェクト再生
			EffectManager::Get().PlayEffectFollow(
				enEffectKind_MushroomKnockBack,
				&transform.position,
				Quaternion::Identity,
				Vector3::One
			);

			app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::KnockbackMushroom), false);

			auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
			if (mushroom)
			{
				auto* manager = mushroom->GetAttackPointManager();
				if (manager && mushroom->GetCurrentAttackPoint() != nullptr)
				{
					manager->ReleaseAttackPoint(mushroom->GetCurrentAttackPoint(), mushroom);
					mushroom->SetCurrentAttackPoint(nullptr);
				}
			}
		}


		void MushroomEventCharacterStateMachine::OnExitKnockBack()
		{
		}


		uint32_t MushroomEventCharacterStateMachine::GetCharacterID() const
		{
			return MushroomEventCharacter::ID();
		}


		void MushroomEventCharacterStateMachine::UpdateState()
		{
			if (attackCoolTimer_ > 0.0f)
			{
				attackCoolTimer_ -= g_gameTime->GetFrameDeltaTime();
			}

			if (poisonCoolTimer_ > 0.0f)
			{
				poisonCoolTimer_ -= g_gameTime->GetFrameDeltaTime();
			}

			// 死亡は最優先（スポーン猶予中でも即処理）
			if (isDead_)
			{
				RequestChangeState(DeadCharacterState::ID());

				// Dead アニメーション終了後：床デカールと同時にフェードアウト開始、完了後に削除通知
				if (IsEqualCurrentState(DeadCharacterState::ID())
					&& CanChangeState())
				{
					auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
					if (mushroom)
					{
						mushroom->StartFadeOut();
						mushroom->NotifyDeadEffect();
						if (mushroom->IsFadeOutComplete())
						{
							isDead_ = false;
							mushroom->NotifyDead();
						}
					}
				}
				return;
			}

			/** スポーン直後、一定時間はAIを停止 */
			if (isJustSpawned_)
			{
				static const float SPAWN_WAIT_TIME = 1.5f;
				aiTimer_ += g_gameTime->GetFrameDeltaTime();
				if (aiTimer_ > SPAWN_WAIT_TIME)
				{
					isJustSpawned_ = false;
					aiTimer_ = 0.0f;
				}
				return;
			}

			if (IsSquashed())
			{
				RequestChangeState(DeadCharacterState::ID());

				// Deadステート側で2秒経過して「遷移可能」になったらIdleに戻す
				if (IsEqualCurrentState(DeadCharacterState::ID())
					&& CanChangeState())
				{
					isSquashed_ = false;
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}


			if (isKnockBack_)
			{
				//ノックバックする方向を設定
				SetMoveDirection(knockBackDirection_);
				RequestChangeState(KnockBackCharacterState::ID());
				//app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::KnockbackMushroom), false);

				if (IsEqualCurrentState(KnockBackCharacterState::ID())
					&& CanChangeState())
				{
					aiTimer_ = 0.0f;
					isKnockBack_ = false;
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}


			if (IsEqualCurrentState(AttackCharacterState::ID()))
			{
				if (CanChangeState()) {
					aiTimer_ = 0.0f;
					isReturningToWait_ = true; // 攻撃後の待機ポイントへ戻るフラグ
					RequestChangeState(RunCharacterState::ID());
				}
				return;
			}

			// 毒雲詠唱ステートのハンドリング（詠唱中はノックバック以外で中断しない）
			if (IsEqualCurrentState(MushroomPoisonCastState::ID()))
			{
				if (CanChangeState())
				{
					// 詠唱タイマー完了時に1回だけ BattleManager へ通知（通知済みフラグで多重送信を防ぐ）
					if (!hasPoisonNotified_)
					{
						hasPoisonNotified_    = true;
						isPoisonCastComplete_ = true;
						poisonCoolTimer_      = kPoisonCoolTime;
						isPoisonSporeActive_  = true;
					}
					// 胞子エフェクトが終わるまでキャスト状態を維持（移動しない）
					if (!isPoisonSporeActive_)
					{
						isReturningToWait_ = true;
						RequestChangeState(RunCharacterState::ID());
					}
				}
				return;
			}

			if (isReturningToWait_)
			{


				auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
				if (mushroom && mushroom->GetAttackPointManager())
				{
					auto* waitPoint = mushroom->GetAttackPointManager()->GetNearWaitPoint(transform.position);
					if (waitPoint != nullptr)
					{
						Vector3 toWait = waitPoint->position_ - transform.position;
						const float distToWait = toWait.Length();

						if (distToWait > 30.0f)
						{
							// まだ待機ポイントに着いていない → 向かい続ける
							toWait.Normalize();
							SetMoveDirection(toWait);
							RequestChangeState(RunCharacterState::ID());
							return;
						}
						else
						{
							// 待機ポイントに到達 → 通常の追跡状態に戻す
							isReturningToWait_ = false;
							SetMoveDirection(Vector3::Zero);
							RequestChangeState(IdleCharacterState::ID());
							return;
						}
					}
					else
					{
						// 待機ポイントが取れなければそのままフラグを解除
						isReturningToWait_ = false;
					}
				}
				else
				{
					isReturningToWait_ = false;
				}
			}

			/** TODO: 敵の視野角にPlayerが入ったら追従して */
			if (isChasing_)
			{
				isChasing_ = false;

				auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
				if (mushroom && mushroom->GetAttackPointManager())
				{
					const bool canAttack = mushroom->GetAttackPointManager()->IsUseable() && attackCoolTimer_ <= 0.0f;
					if (!canAttack)
					{
						auto* waitPoint = mushroom->GetAttackPointManager()->GetNearWaitPoint(transform.position);
						if (waitPoint != nullptr)
						{
							Vector3 toWait = waitPoint->position_ - transform.position;
							toWait.Normalize();
							SetMoveDirection(toWait);
						}
					}
				}

				if (IsEqualCurrentState(IdleCharacterState::ID()) && aiTimer_ > 5.0f) {}

				else
				{
					Vector3 toPlayer = targetPosition_ - transform.position;
					toPlayer.y = 0.0f;
					float distance = toPlayer.Length();

					if (distance <= 40.0f)
					{
						auto* manager = mushroom->GetAttackPointManager();

						if (attackCoolTimer_ <= 0.0f)
						{
							manager->RequestAttackToken(mushroom);
						}

						const bool isMyTurn = manager->ConsumeAttackToken(mushroom);
						if (isMyTurn && manager->IsUseable())
						{
							SetMoveDirection(chaseDirection_);
							RequestChangeState(AttackCharacterState::ID());
							app::SoundManager::Get().PlaySE(static_cast<int>(app::SoundKind::AtkMushroom), false);
						}
						else
						{
							// 自分の番じゃない → 待機ポイントで待つ
							auto* waitPoint = manager->GetNearWaitPoint(transform.position);
							if (waitPoint != nullptr)
							{
								Vector3 toWait = waitPoint->position_ - transform.position;
								toWait.Normalize();
								SetMoveDirection(toWait);
							}
							RequestChangeState(RunCharacterState::ID());
						}
						return;
					}
					else if (distance <= kPoisonMaxDist && poisonCoolTimer_ <= 0.0f)
					{
						// 中距離かつクールダウン完了 → 毒雲詠唱を開始
						poisonCastTargetPos_ = targetPosition_;
						RequestChangeState(MushroomPoisonCastState::ID());
						return;
					}
					else if (distance <= 400.0f)
					{
						// 待機距離内に入ったら周回待機
						auto* mushroom = dynamic_cast<app::actor::MushroomEventCharacter*>(GetCharacter());
						if (mushroom && mushroom->GetAttackPointManager())
						{
							const bool canAttack = mushroom->GetAttackPointManager()->IsUseable()
								&& attackCoolTimer_ <= 0.0f;

							if (!canAttack)
							{
								// 攻撃できない → 待機ポイントへ向かいながら周回
								auto* waitPoint = mushroom->GetAttackPointManager()->GetNearWaitPoint(transform.position);

								if (waitPoint != nullptr)
								{
									Vector3 toWait = waitPoint->position_ - transform.position;
									const float distanceToWait = toWait.Length();

									if (distanceToWait < 30.0f)
									{
										SetMoveDirection(Vector3::Zero);
										RequestChangeState(IdleCharacterState::ID());
										aiTimer_ = 0.0f;
										return;
									}
									toWait.Normalize();
									SetMoveDirection(toWait);
								}
								RequestChangeState(RunCharacterState::ID());
								aiTimer_ = 5.0f; // 追跡タイマーをリセットしない
								return;
							}
						}
						SetMoveDirection(chaseDirection_);
						RequestChangeState(RunCharacterState::ID());
						aiTimer_ = 5.0f; // 追跡タイマーをリセットしない
					}
					/** DEBUG: いらないかも */
					else
					{
						RequestChangeState(RunCharacterState::ID());
						aiTimer_ = 0.0f;
					}
					return;
				}
			}

			/** デバッグテスト: 待機⇒左に走る⇒右に走る⇒待機のモーション */
			if (IsEqualCurrentState(IdleCharacterState::ID()))
			{
				if (isAIEnabled_)
				{
					aiTimer_ += g_gameTime->GetFrameDeltaTime();
					if (aiTimer_ > WAIT_TIME)
					{
						RequestChangeState(PatrolCharacterState::ID());
						aiTimer_ = 0.0f;
					}
				}
				return;
			}

			if (IsEqualCurrentState(PatrolCharacterState::ID()))
			{
				if (CanChangeState())
				{
					SetMoveDirection(Vector3::Zero);
					RequestChangeState(IdleCharacterState::ID());
				}
				return;
			}

			if (IsEqualCurrentState(RunCharacterState::ID()))
			{
				aiTimer_ += g_gameTime->GetFrameDeltaTime();

				if (aiTimer_ <= 2.0f)
				{
					SetMoveDirection(Vector3::Left);
				}
				else if (aiTimer_ <= 4.0f)
				{
					SetMoveDirection(Vector3::Right);
				}
				else
				{
					SetMoveDirection(Vector3::Zero);
					RequestChangeState(IdleCharacterState::ID());
					aiTimer_ = 0.0f;
				}
				return;
			}

			RequestChangeState(IdleCharacterState::ID());
		}
	}
}