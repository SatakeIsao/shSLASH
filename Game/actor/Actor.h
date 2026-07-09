/**
 * Actorファイル
 */
#pragma once
#include "EquipmentSlotManager.h"
#include "ActorStatus.h"

namespace app 
{ 
	namespace collision 
	{ 
		class GhostBody;
	} 
}


namespace app
{
	namespace actor
	{
		struct CharacterStatus;


		/**
		 * キャラクターの初期化で渡す情報
		 * NOTE モデル表示やアニメーションなど
		 */
		struct CharacterInitializeParameter
		{
			using LoaderFunc = std::function<void(CharacterInitializeParameter* parameter)>;

			struct AnimationData
			{
				const char* filename;
				bool loop;
			};

			const char* modelName = nullptr;
			app::memory::Array<AnimationData> animationDataList;
			LoaderFunc loaderFunc = nullptr;
			//
			CharacterInitializeParameter(const LoaderFunc& func)
			{
				loaderFunc = std::move(func);
			}
			void Load()
			{
				loaderFunc(this);
			}
		};


		class Character : public IGameObject
		{
			/** 例外としてpublic */
		public:
			app::math::Transform transform;


		protected:
			std::unique_ptr<ModelRender> modelRender_ = nullptr;
			std::unique_ptr<CharacterController> characterController_ = nullptr;
			app::memory::Array<AnimationClip> animationClips_;
			EquipmentSlotManager equipmentSlots_;

			app::actor::CharacterStatus* status_ = nullptr;

			int currentHP_ = 0;

		private:
			/** 見た目のみに適用するオフセット（物理・コリジョン位置には影響しない） */
			Vector3 renderPositionOffset_ = Vector3::Zero;

		protected:
			const Vector3& GetRenderPositionOffset() const { return renderPositionOffset_; }

		public:
			void SetRenderPositionOffset(const Vector3& offset) { renderPositionOffset_ = offset; }
			Character();
			virtual ~Character();

			bool Start() override { return true; }
			void Update() override;
			void Render(RenderContext& rc) override;

			virtual void Initialize(CharacterInitializeParameter& param) = 0;

			app::actor::CharacterStatus* GetStatus() { return status_; }
			ModelRender* GetModelRender() { return modelRender_.get(); }
			CharacterController* GetCharacterController() { return characterController_.get(); }

			virtual app::collision::GhostBody* GetGhostBody() const { return nullptr; }

			int GetCurrentHP() const { return currentHP_; }

			float GetTotalAttackPower() const
			{
				return status_->GetAttackPower() + equipmentSlots_.GetTotalAttackPower();
			}
			float GetTotalDefensePower() const
			{
				return equipmentSlots_.GetTotalDefensePower();
			}
			void TakeDamage(int damage)
			{
				currentHP_ -= damage;
				if (currentHP_ < 0)
				{
					currentHP_ = 0;
				}
			}

		};
	}
}