/**
 * UIAnimationFactory.h
 * UIAnimationParameter の定義から UIAnimation を生成するラッパー
 *
 * 依存方向:
 *   UIAnimationFactory → UIAnimation.h（具象クラス）
 *   UIAnimationFactory → UIAnimationParameter.h（定義データ）
 *   UIAnimationFactory → UIParts.h（UIBase::AddAnimation）
 *
 * UIAnimation.h, UIAnimationParameter.h 側はこのファイルを知らないため循環しない
 */
#pragma once
#include "UIAnimation.h"
#include "UIParts.h"
#include "UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		class UIAnimationFactory
		{
		public:
			/**
			 * 具象クラスを指定してアニメーションを生成する
			 * @tparam T UIColorAnimation, UIScaleAnimation 等の具象型
			 * @param key CRC32ハッシュキー
			 * @return 生成されたアニメーション（定義が無ければ nullptr）
			 *
			 * 使用例:
			 *   auto anim = UIAnimationFactory::Create<UIColorAnimation>(Hash32("FadeIn"));
			 */
			template <typename T>
			static std::unique_ptr<T> Create(uint32_t key)
			{
				const UIAnimationDef* def = UIAnimationParameter::Get().Find(key);
				if (!def) return nullptr;

				auto anim = std::make_unique<T>();
				ApplyParameter(anim.get(), *def);
				return anim;
			}


			/**
			 * アニメーションを生成し、UIに直接登録する
			 * @tparam T 具象アニメーション型
			 * @param target 登録先の UIBase
			 * @param key CRC32ハッシュキー（JSON の key と AddAnimation の key を共用）
			 * @return 成功したか
			 *
			 * 使用例:
			 *   UIAnimationFactory::Attach<UIColorAnimation>(icon, Hash32("FadeIn"));
			 *   UIAnimationFactory::Attach<UIScaleAnimation>(icon, Hash32("ScaleUp"));
			 */
			template <typename T>
			static bool Attach(UIBase* target, uint32_t key)
			{
				if (!target) return false;

				auto anim = Create<T>(key);
				if (!anim) return false;

				target->AddAnimation(key, std::move(anim));
				return true;
			}


		private:
			/** Float 系（UIFloatAnimation, UIRotationAnimation 等） */
			static void ApplyParameter(UIFloatAnimation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startF, def.endF, def.duration, def.easingType, def.loopMode);
			}

			/** Vector2 系 */
			static void ApplyParameter(UIVector2Animation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startV2, def.endV2, def.duration, def.easingType, def.loopMode);
			}

			/** Vector3 系（UIScaleAnimation, UITranslateAniamtion 等） */
			static void ApplyParameter(UIVector3Animation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startV3, def.endV3, def.duration, def.easingType, def.loopMode);
			}

			/** Vector4 系（UIColorAnimation 等） */
			static void ApplyParameter(UIVector4Animation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startV4, def.endV4, def.duration, def.easingType, def.loopMode);
			}
		};
	}
}
