/**
 * DamagePopObject.cpp
 */
#include "stdafx.h"
#include "DamagePopUI.h"


namespace app
{
    namespace ui
    {
        DamagePopUIObject::DamagePopUIObject()
        {
            layout_ = std::make_unique<app::ui::Layout>();
            layout_->Initialize<app::ui::MenuBase>(LAYOUT_PATH);
        }


        DamagePopUIObject::~DamagePopUIObject()
        {
            delete animSeq_;
        }


        void DamagePopUIObject::OnSpawn()
        {
            lifeTimer_ = LIFE_TIME;
            riseOffset_ = 0.0f;
            isActive = true;
        }


        void DamagePopUIObject::ApplySpawnParams()
        {
            worldPos_ = pendingWorldPos_;

            auto* digit = layout_->GetMenu()->GetUI<UIDigit>(Hash32(DIGIT_UI_NAME));
            if (!digit) return;

            digit->SetNumber(pendingDamage_);
            digit->transform.localScale = Vector3(1.0f, 1.0f, 1.0f);
            digit->color = Vector4(1.0f, 1.0f, 1.0f, 0.0f);

            /** 付与していたアニメーションを取り外し、再度付与 */
            digit->RemoveAnimation(Hash32("DamagePop_FadeIn"));
            app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(digit, Hash32("DamagePop_FadeIn"));

            digit->RemoveAnimation(Hash32("DamagePop_ScaleUp"));
            app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(digit, Hash32("DamagePop_ScaleUp"));

            digit->RemoveAnimation(Hash32("DamagePop_ScaleDown"));
            app::ui::UIAnimationFactory::Attach<app::ui::UIScaleAnimation>(digit, Hash32("DamagePop_ScaleDown"));

            digit->RemoveAnimation(Hash32("DamagePop_FadeOut"));
            app::ui::UIAnimationFactory::Attach<app::ui::UIColorAnimation>(digit, Hash32("DamagePop_FadeOut"));

            /** シーケンスを生成し直して再生 */
            delete animSeq_;
            animSeq_ = new app::ui::UIAnimationSequence();
            animSeq_->Add(Hash32("DamagePop_FadeIn"));
            animSeq_->Add(Hash32("DamagePop_ScaleUp"));
            animSeq_->Add(Hash32("DamagePop_ScaleDown"));
            animSeq_->Add(Hash32("DamagePop_FadeOut"));
            animSeq_->Play(digit);
        }


        void DamagePopUIObject::OnRecycle()
        {
            isActive = false;
        }


        void DamagePopUIObject::Update()
        {
            if (!isActive) return;

            const float dt = g_gameTime->GetFrameDeltaTime();

            /** 残り時間を消費 */
            lifeTimer_ -= dt;
            if (lifeTimer_ <= 0.0f)
            {
                if (ownerPool_)
                {
                    /** 返却 */
                    ownerPool_->Recycle(this);
                }
                else
                {
                    OnRecycle();
                }
                return;
            }

            /** 上昇 */
            riseOffset_ += RISE_SPEED * dt;

            /** ワールド座標 → スクリーン座標 */
            Vector3 spawnPos = worldPos_;
            spawnPos.y += WORLD_Y_OFFSET;

            Vector2 screenPos;
            g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, spawnPos);

            /** UIDigit の座標・アルファを更新 */
            auto* digit = layout_->GetMenu()->GetUI<UIDigit>(Hash32(DIGIT_UI_NAME));
            if (digit)
            {
                digit->transform.localPosition.x = screenPos.x;
                digit->transform.localPosition.y = screenPos.y + riseOffset_;
            }

            /** アニメーションシーケンスを更新 */
            animSeq_->Update(dt);
            layout_->GetMenu()->Update();
        }


        void DamagePopUIObject::Render(RenderContext& rc)
        {
            if (!isActive) return;

            layout_->GetMenu()->Render(rc);
        }




        /************************************************************/


        void DamagePopPool::Initialize()
        {
            pool_.Initialize();
        }


        void DamagePopPool::Finalize()
        {
            pool_.Finalize();
        }


        void DamagePopPool::OnDamageDealt(int damage, const Vector3& worldPos)
        {
            auto* slot = pool_.Acquire();
            if (!slot) return;

            slot->SetOwnerPool(this);
            slot->SetSpawnParams(damage, worldPos);
            slot->ApplySpawnParams();
        }
    }
}