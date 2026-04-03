/**
 * EventCharacterファイル
 */
#include "stdafx.h"
#include "EventCharacter.h"
#include "ActorStatus.h"

namespace app
{
	namespace actor
	{
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
			modelRender_->Init(param.modelName,animationClips_.data(),animationClips_.size());

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
	}
}