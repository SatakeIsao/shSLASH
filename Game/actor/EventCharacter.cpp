/**
 * EventCharacterファイル
 */
#include "stdafx.h"
#include "EventCharacter.h"
#include "core/ParameterManager.h"

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
			//SuperClass::Render(rc);
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
			if (isPause_) { return; }

			const float deltaTime = g_gameTime->GetFrameDeltaTime();
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
			transform.localRotation = stateMachine_	->transform.rotation;
			transform.UpdateTransform();
			stateMachine_->transform.position = nextPosition;

			// ゴーストボディ
			Vector3 centerPos = transform.position;
			centerPos.y += status_->GetRadius() * 2.0f;
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
			modelRender_->Init(param.modelName, animationClips_.data(), animationClips_.size(), enModelUpAxisZ, true, false);

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
			if (isPause_) { return; }

			const float deltaTime = g_gameTime->GetFrameDeltaTime();
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
			modelRender_->Init(param.modelName, animationClips_.data(), animationClips_.size(), enModelUpAxisZ, true, false);

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