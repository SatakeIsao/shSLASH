/**
 * EventCharacterファイル
 */
#include "stdafx.h"
#include "EventCharacter.h"
#include "core/ParameterManager.h"

namespace
{
	constexpr float SPAWN_FADE_DURATION = 0.5f;
	constexpr float DEATH_FADE_DURATION = 0.5f;
}

namespace app
{
	namespace actor
	{
        int StoneEventCharacter::instanceCount_ = 0;
		int MushroomEventCharacter::instanceCount_ = 0;

		EventCharacter::EventCharacter()
		{
			characterController_ = std::make_unique<CharacterController>();
			stateMachine_ = std::make_unique<EventCharacterStateMachine>();
			status_ = new app::actor::EventCharacterStatus();
			ghostBody_ = std::make_unique<app::collision::GhostBody>();
		}


		EventCharacter::~EventCharacter()
		{
		}


		bool EventCharacter::Start()
		{
			stateMachine_->Initialize();
			stateMachine_->Setup(this);
			status_->Setup();

			ghostBody_->CreateCapsule(this, ID(), status_->GetRadius(), status_->GetHeight(), app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);

			characterController_->Init(status_->GetRadius(), status_->GetHeight(), transform.position);
			characterController_->SetGravity(status_->GetGravity());

			return true;
		}


		void EventCharacter::Update()
		{
			if (isPause_) { return; }

			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			stateMachine_->Update();
			auto nextPosition = characterController_->Execute(stateMachine_->transform.position, deltaTime);

			transform.localPosition = nextPosition;
			transform.localScale = stateMachine_->transform.scale;
			transform.localRotation = stateMachine_->transform.rotation;
			transform.UpdateTransform();
			stateMachine_->transform.position = nextPosition;

			// ゴーストボディ
			Vector3 centerPos = transform.position;
			centerPos.y += status_->GetRadius() * 2.0f;
			ghostBody_->SetPosition(centerPos);

			SuperClass::Update();
		}


		void EventCharacter::Render(RenderContext& rc)
		{
			SuperClass::Render(rc);
		}


		void EventCharacter::Initialize(CharacterInitializeParameter& param)
		{
			param.Load();

			const uint32_t animationCount = static_cast<uint32_t>(param.animationDataList.size());
			animationClips_.Create(animationCount);
			for (uint32_t i = 0; i < animationCount; ++i) {
				animationClips_[i].Load(param.animationDataList[i].filename);
				animationClips_[i].SetLoopFlag(param.animationDataList[i].loop);
			}

			modelRender_ = std::make_unique<ModelRender>();
			modelRender_->Init(param.modelName, animationClips_.data(), animationClips_.size());

			transform.position = Vector3::Zero;
			transform.scale = Vector3::One;
			transform.rotation = Quaternion::Identity;
		}


		void EventCharacter::ResizeCollision()
		{
			//古いデータを破棄してリセット
			ghostBody_ = std::make_unique<app::collision::GhostBody>();
			ghostBody_->CreateCapsule(this, ID(), status_->GetRadius(), status_->GetHeight(), app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
		}




		/****************************************************/

		StoneEventCharacter::StoneEventCharacter()
		{
			characterController_ = std::make_unique<CharacterController>();
			stateMachine_ = std::make_unique<StoneEventCharacterStateMachine>();
			status_ = new app::actor::StoneEventCharacterStatus();
			ghostBody_ = std::make_unique<app::collision::GhostBody>();

			instanceCount_++;
		}


		StoneEventCharacter::~StoneEventCharacter()
		{
			instanceCount_--;
			if (instanceCount_ < 0) 
			{
				instanceCount_ = 0;
			}
		}


		bool StoneEventCharacter::Start()
		{
			stateMachine_->Initialize();
			stateMachine_->Setup(this);
			status_->Setup();
			ghostBody_->CreateCapsule(this, ID(), status_->GetRadius(), status_->GetHeight(), app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
			ghostBody_->SetActive(false);

			// プール待機中は原点に剛体が残らないよう、ステージ外の位置で初期化する
			characterController_->Init(status_->GetRadius(), status_->GetHeight(), Vector3(0.0f, -99999.0f, 0.0f));
			characterController_->SetGravity(status_->GetGravity());
			stateMachine_->transform.position = transform.position;

			currentHP_ = static_cast<int>(status_->GetMaxHp());

			return true;
		}

		void StoneEventCharacter::Update()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// フェード更新はポーズ中でも行う（スポーン直後に凍結されても正しく表示されるよう）
			if (isFadingIn_)
			{
				fadeTimer_ += deltaTime;
				cbData_.fadeRatio = fadeTimer_ / SPAWN_FADE_DURATION;
				if (cbData_.fadeRatio >= 1.0f)
				{
					cbData_.fadeRatio = 1.0f;
					isFadingIn_ = false;
				}
			}
			if (isFadingOut_)
			{
				fadeTimer_ += deltaTime;
				cbData_.fadeRatio = 1.0f - fadeTimer_ / DEATH_FADE_DURATION;
				if (cbData_.fadeRatio < 0.0f) cbData_.fadeRatio = 0.0f;
			}

			if (isPause_)
			{
				if (stateMachine_->IsDeadState())
				{
					// 死亡フラグが立ったら凍結を解除して通常の死亡処理へ移行する
					isPause_ = false;
				}
				else
				{
					// ゴーストボディを正しい位置に維持する（SetActiveされても位置は自動更新されないため）
					Vector3 centerPos = transform.position;
					centerPos.y += status_->GetRadius() * 2.0f;
					ghostBody_->SetPosition(centerPos);
					return;
				}
			}

			stateMachine_->Update();

			// バウンド攻撃終了後、移動しながら描画オフセットを元の高さへ戻す
			// とびかかり攻撃中（バウンド含む）はステート側が管理するためスキップ
			// 速度は二段階：めり込み帯（< -5）は素早く抜け、その後はゆっくり戻す
			{
				auto* stoneSM = static_cast<StoneEventCharacterStateMachine*>(stateMachine_.get());
				if (!stoneSM->IsInPounceAttack())
				{
					const Vector3& curOffset = GetRenderPositionOffset();
					if (curOffset.y < 0.0f)
					{
						// -5 より低い間は歩行中に下あごが地面にめり込むため素早く通過
						constexpr float SINK_THRESHOLD =  -5.0f;
						constexpr float FAST_SPEED     =  30.0f; // めり込み帯を抜ける速度
						constexpr float SLOW_SPEED     =   8.0f; // 元の高さへ戻る速度
						const float speed = (curOffset.y < SINK_THRESHOLD) ? FAST_SPEED : SLOW_SPEED;
						const float newY = (std::min)(curOffset.y + speed * deltaTime, 0.0f);
						SetRenderPositionOffset(Vector3(curOffset.x, newY, curOffset.z));
					}
				}
			}

			Vector3 nextPosition = stateMachine_->transform.position;

			// 死亡中はRequestTeleport+Executeで地下へ退避（AABBも含めて正しく更新するため）
			if (stateMachine_->IsDeadState())
			{
				static const Vector3 undergroundPos(0.0f, -99999.0f, 0.0f);
				characterController_->RequestTeleport();
				characterController_->Execute(undergroundPos, deltaTime);
			}
			else
			{
				if (justSpawned_)
				{
					// 地下から復帰する初回フレームはテレポートでsweep testをスキップ
					characterController_->RequestTeleport();
				}
				nextPosition = characterController_->Execute(stateMachine_->transform.position, deltaTime);
				if (justSpawned_)
				{
					justSpawned_ = false;
					nextPosition = stateMachine_->transform.position; // コントローラーの古い位置を無視

					// 死亡時に非表示にしたCharacterControllerの剛体を再表示する
					btRigidBody* ccBody = characterController_->GetRigidBody()->GetBody();
					ccBody->setCollisionFlags(ccBody->getCollisionFlags() & ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
				}
			}

			transform.localPosition = nextPosition;
			transform.localScale = stateMachine_->transform.scale;
			transform.localRotation = stateMachine_	->transform.rotation;
			transform.UpdateTransform();
			stateMachine_->transform.position = nextPosition;

			// ガード中のプレイヤーには、AIステートに関わらず防御球（防御エフェクトの手前）より
			// 内側へ入り込ませない（噛みつき等でめり込んで見えるのを防ぐ）
			if (battleCharacter_ && !stateMachine_->IsDeadState()
				&& battleCharacter_->GetStateMachine()->IsGuarding())
			{
				const Vector3& playerPos = battleCharacter_->transform.position;
				Vector3 toEnemy = transform.position - playerPos;
				toEnemy.y = 0.0f;
				const float dist = toEnemy.Length();
				// コリジョン用の半径だけでなく、モデルの見た目の大きさ分の余裕(kGuardModelMargin)も確保する
				constexpr float minDist = app::actor::BattleCharacterStateMachine::kGuardBlockRadius
					+ app::actor::StoneEventCharacterStateMachine::kGuardModelMargin;

				if (dist < minDist)
				{
					if (dist > 0.0001f)
					{
						toEnemy /= dist;
					}
					else
					{
						toEnemy = stateMachine_->GetMoveDirection() * -1.0f;
						toEnemy.y = 0.0f;
						if (toEnemy.LengthSq() < 0.0001f) { toEnemy = Vector3::Front; }
						toEnemy.Normalize();
					}

					Vector3 correctedPos = playerPos + toEnemy * minDist;
					correctedPos.y = transform.position.y;

					transform.localPosition = correctedPos;
					transform.UpdateTransform();
					stateMachine_->transform.position = correctedPos;
					characterController_->SetPosition(correctedPos);
				}
			}

			// ゴーストボディ
			Vector3 centerPos = transform.position;
			centerPos.y += status_->GetRadius() * 2.0f;

			// 噛みつき攻撃中は、顔を突き出す動きにゴーストボディを連動させる
			const float lungeOffset = stateMachine_->GetCurrentAttackLungeOffset();
			if (lungeOffset != 0.0f)
			{
				Vector3 forward = stateMachine_->GetMoveDirection();
				if (forward.LengthSq() > 0.01f)
				{
					centerPos += forward * lungeOffset;
				}

				// ガード中のプレイヤーの防御球より内側にはゴーストボディを進めない
				if (battleCharacter_ && battleCharacter_->GetStateMachine()->IsGuarding())
				{
					const Vector3& playerPos = battleCharacter_->transform.position;
					Vector3 toBody = centerPos - playerPos;
					toBody.y = 0.0f;
					const float dist = toBody.Length();
					constexpr float minDist = app::actor::BattleCharacterStateMachine::kGuardBlockRadius;
					if (dist < minDist && dist > 0.0001f)
					{
						toBody /= dist;
						const float bodyY = centerPos.y;
						centerPos = playerPos + toBody * minDist;
						centerPos.y = bodyY;
					}
				}
			}

			ghostBody_->SetPosition(centerPos);

			SuperClass::Update();
		}

		void StoneEventCharacter::Render(RenderContext& rc)
		{
			// ポーズ中は描画する、プール中（非表示）は描画しない
			if (!isVisible_) { return; }
			SuperClass::Render(rc);
		}


		void StoneEventCharacter::Initialize(CharacterInitializeParameter& param)
		{
			param.Load();
			const uint32_t animationCount = static_cast<uint32_t>(param.animationDataList.size());
			animationClips_.Create(animationCount);
			for (uint32_t i = 0; i < animationCount; ++i)
			{
				animationClips_[i].Load(param.animationDataList[i].filename);
				animationClips_[i].SetLoopFlag(param.animationDataList[i].loop);
			}

			modelRender_ = std::make_unique<ModelRender>();
			modelRender_->Init(param.modelName, animationClips_.data(), animationClips_.size(), enModelUpAxisZ, true, false,
				nullptr, "Assets/Shader/model_enemy_fade.fx",
				&cbData_, static_cast<int>(sizeof(cbData_)));

			transform.scale = Vector3::One;
			transform.rotation = Quaternion::Identity;
		}


		void StoneEventCharacter::ResizeCollision()
		{
			//古いデータを破棄してリセット
			ghostBody_ = std::make_unique<app::collision::GhostBody>();
			ghostBody_->CreateCapsule(this, ID(), status_->GetRadius(), status_->GetHeight(), app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
		}




		/****************************************************/


		MushroomEventCharacter::MushroomEventCharacter()
		{
			characterController_ = std::make_unique<CharacterController>();
			stateMachine_ = std::make_unique<MushroomEventCharacterStateMachine>();
			status_ = new app::actor::MushroomEventCharacterStatus();
			ghostBody_ = std::make_unique<app::collision::GhostBody>();

			instanceCount_++;
		}

		MushroomEventCharacter::~MushroomEventCharacter()
		{
			instanceCount_--;
			if (instanceCount_ < 0)
			{
				instanceCount_ = 0;
			}
		}

		bool MushroomEventCharacter::Start()
		{
			stateMachine_->Initialize();
			stateMachine_->Setup(this);
			status_->Setup();
			ghostBody_->CreateCapsule(this, ID(), status_->GetRadius(), status_->GetHeight(), app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
			ghostBody_->SetActive(false);

			// プール待機中は原点に剛体が残らないよう、ステージ外の位置で初期化する
			characterController_->Init(status_->GetRadius(), status_->GetHeight(), Vector3(0.0f, -99999.0f, 0.0f));
			characterController_->SetGravity(status_->GetGravity());
			stateMachine_->transform.position = transform.position;

			currentHP_ = static_cast<int>(status_->GetMaxHp());

			return true;
		}

		void MushroomEventCharacter::Update()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// フェード更新はポーズ中でも行う（スポーン直後に凍結されても正しく表示されるよう）
			if (isFadingIn_)
			{
				fadeTimer_ += deltaTime;
				cbData_.fadeRatio = fadeTimer_ / SPAWN_FADE_DURATION;
				if (cbData_.fadeRatio >= 1.0f)
				{
					cbData_.fadeRatio = 1.0f;
					isFadingIn_ = false;
				}
			}
			if (isFadingOut_)
			{
				fadeTimer_ += deltaTime;
				cbData_.fadeRatio = 1.0f - fadeTimer_ / DEATH_FADE_DURATION;
				if (cbData_.fadeRatio < 0.0f) cbData_.fadeRatio = 0.0f;
			}

			if (isPause_)
			{
				if (stateMachine_->IsDeadState())
				{
					// 死亡フラグが立ったら凍結を解除して通常の死亡処理へ移行する
					isPause_ = false;
				}
				else
				{
					// ゴーストボディを正しい位置に維持する（SetActiveされても位置は自動更新されないため）
					Vector3 centerPos = transform.position;
					centerPos.y += status_->GetRadius() * 2.0f;
					ghostBody_->SetPosition(centerPos);
					return;
				}
			}

			stateMachine_->Update();

			Vector3 nextPosition = stateMachine_->transform.position;

			// 死亡中はRequestTeleport+Executeで地下へ退避（AABBも含めて正しく更新するため）
			if (stateMachine_->IsDeadState())
			{
				static const Vector3 undergroundPos(0.0f, -99999.0f, 0.0f);
				characterController_->RequestTeleport();
				characterController_->Execute(undergroundPos, deltaTime);
			}
			else
			{
				if (justSpawned_)
				{
					// 地下から復帰する初回フレームはテレポートでsweep testをスキップ
					characterController_->RequestTeleport();
				}
				nextPosition = characterController_->Execute(stateMachine_->transform.position, deltaTime);
				if (justSpawned_)
				{
					justSpawned_ = false;
					nextPosition = stateMachine_->transform.position; // コントローラーの古い位置を無視

					// 死亡時に非表示にしたCharacterControllerの剛体を再表示する
					btRigidBody* ccBody = characterController_->GetRigidBody()->GetBody();
					ccBody->setCollisionFlags(ccBody->getCollisionFlags() & ~btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
				}
			}

			transform.localPosition = nextPosition;
			transform.localScale = stateMachine_->transform.scale;
			transform.localRotation = stateMachine_->transform.rotation;
			transform.UpdateTransform();
			stateMachine_->transform.position = nextPosition;

			// ゴーストボディ
			Vector3 centerPos = transform.position;
			centerPos.y += status_->GetRadius() * 2.0f;
			ghostBody_->SetPosition(centerPos);

			SuperClass::Update();
		}

		void MushroomEventCharacter::Render(RenderContext& rc)
		{
			// ポーズ中は描画する、プール中（非表示）は描画しない
			if (!isVisible_) { return; }
			SuperClass::Render(rc);
		}

		void MushroomEventCharacter::Initialize(CharacterInitializeParameter& param)
		{
			param.Load();
			const uint32_t animationCount = static_cast<uint32_t>(param.animationDataList.size());
			animationClips_.Create(animationCount);
			for (uint32_t i = 0; i < animationCount; ++i)
			{
				animationClips_[i].Load(param.animationDataList[i].filename);
				animationClips_[i].SetLoopFlag(param.animationDataList[i].loop);
			}
			modelRender_ = std::make_unique<ModelRender>();
			modelRender_->Init(param.modelName, animationClips_.data(), animationClips_.size(), enModelUpAxisZ, true, false,
				nullptr, "Assets/Shader/model_enemy_fade.fx",
				&cbData_, static_cast<int>(sizeof(cbData_)));

			transform.scale = Vector3::One;
			transform.rotation = Quaternion::Identity;
		}

		void MushroomEventCharacter::ResizeCollision()
		{
			//古いデータを破棄してリセット
			ghostBody_ = std::make_unique<app::collision::GhostBody>();
			ghostBody_->CreateCapsule(this, ID(), status_->GetRadius(), status_->GetHeight(), app::collision::ghost::CollisionAttribute::Enemy, app::collision::ghost::CollisionAttributeMask::All);
		}
	}
}