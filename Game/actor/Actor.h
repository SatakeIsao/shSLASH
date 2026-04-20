/**
 * Actorファイル
 */
#pragma once
#include "EquipmentSlotManager.h"


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

		public:
			Character();
			virtual ~Character();

			bool Start() override { return true; }
			void Update() override;
			void Render(RenderContext& rc) override;

			virtual void Initialize(CharacterInitializeParameter& param) = 0;

			app::actor::CharacterStatus* GetStatus() { return status_; }
			ModelRender* GetModelRender() { return modelRender_.get(); }
			CharacterController* GetCharacterController() { return characterController_.get(); }

			int GetCurrentHP() const { return currentHP_; }

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