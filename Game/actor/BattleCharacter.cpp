/**
 * BattleCharacterファイル
 */
#include "stdafx.h"
#include "BattleCharacter.h"
#include "ActorStatus.h"


namespace app
{
	namespace actor
	{
		BattleCharacter::BattleCharacter()
		{
			currentHP_ = 8;
			characterController_ = std::make_unique<CharacterController>();
			stateMachine_ = std::make_unique<BattleCharacterStateMachine>();
			status_ = new app::actor::BattleCharacterStatus();
			ghostBody_ = std::make_unique<app::collision::GhostBody>();
			equipmentSlots_.Equip(
				EquipmentSlotType::Weapon, std::make_unique<Weapon>()
			);
		}


		BattleCharacter::~BattleCharacter()
		{
		}


		bool BattleCharacter::Start()
		{
			stateMachine_->Initialize();
			stateMachine_->Setup(this);
			status_->Setup();

			characterController_->Init(status_->GetRadius(), status_->GetHeight(), transform.position);
			characterController_->SetGravity(status_->GetGravity());

			return true;
		}


		void BattleCharacter::Update()
		{
			if (isPause_) {	return; }

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			stateMachine_->Update();

			auto nextPosition = characterController_->Execute(stateMachine_->transform.position, deltaTime);

			transform.localPosition = nextPosition;
			transform.localScale = stateMachine_->transform.scale;
			transform.localRotation = stateMachine_->transform.rotation;
			transform.UpdateTransform();
			stateMachine_->transform.position = nextPosition;

			ghostBody_->SetPosition(transform.position);

			SuperClass::Update();
		}


		void BattleCharacter::Render(RenderContext& rc)
		{
			SuperClass::Render(rc);
		}


		void BattleCharacter::Initialize(CharacterInitializeParameter& param)
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

			/** DEBUG: スライムと座標同じだったのでテスト用でずらした */
			//transform.position = Vector3::Zero;
			transform.position = Vector3(0.0f,0.0f,-500.0f);
			transform.scale = Vector3::One;
			transform.rotation = Quaternion::Identity;

			ghostBody_->CreateCapsule(this, ID(), status_->GetRadius(), status_->GetHeight(), app::collision::ghost::CollisionAttribute::Player, app::collision::ghost::CollisionAttributeMask::All);
		}


		float BattleCharacter::GetTotalAttack() const
		{
			// ベースの攻撃力（マスターデータ由来）
			float base = status_ ? status_->GetAttackPower() : 0.0f;

			// 武器スロットの攻撃力を加算
			float weaponPower = equipmentSlots_.GetTotalAttackPower();

			return base + weaponPower;
			//// 武器を持っていない場合
			//if (weapon_ == nullptr)
			//{
			//	return attackPower_;
			//}

			// 武器の攻撃力
			//return weapon_->GetAttackPower();
		}
	}
}