/**
 * Actorファイル
 */
#include "stdafx.h"
#include "Gimmick.h"

namespace
{
    /** 地面 */
    // 地面のYオフセット
    static constexpr float GROUND_OFFSET_Y = -350.0f;

    /** 壁 */
    // 壁の半径
    static constexpr float WALL_RADIUS = 0.385f;
    // 壁の高さ
    static constexpr float WALL_HEIGHT = 500.0f;
    // 壁の厚み
    static constexpr float WALL_THICKNESS = 200.0f;
    // 壁幅の余白
    static constexpr float WALL_WIDTH_MARGIN = 50.0f;
    // 壁のYオフセット
    static constexpr float WALL_OFFSET_Y = 350.0f;
    // 壁の追加Y
    static constexpr float WALL_EXTRA_Y = 500.0f;
    // 壁の分割数
    static constexpr int   WALL_COUNT = 16;

    /** 計算時に使用 */
    // 円周率
    static constexpr float PI = 3.14159265f;
    // 度をラジアンに変換する係数
    static constexpr float DEG_TO_RAD = PI / 180.0f;
    // 円一周の角度
    static constexpr float FULL_CIRCLE_DEG = 360.0f;
    // 半分
    static constexpr float HALF = 0.5f;
}


namespace app
{
	namespace actor
	{
		IGimmick::IGimmick()
		{
        }


		IGimmick::~IGimmick()
		{
        }


		void IGimmick::Update()
		{
			modelRender_->SetTRS(transform.position, transform.rotation, transform.scale);
			modelRender_->Update();
		}


		void IGimmick::Render(RenderContext& rc)
		{
			modelRender_->Draw(rc);
		}




		/************************************/


		StaticGimmick::StaticGimmick()
		{
        }


		StaticGimmick::~StaticGimmick()
		{
        }


		bool StaticGimmick::Start()
		{
			return true;
		}


		void StaticGimmick::Update()
		{
			IGimmick::Update();
		}


		void StaticGimmick::Render(RenderContext& rc)
		{
			IGimmick::Render(rc);
		}


        void StaticGimmick::Initialize(const char* path)
        {
            // モデル読み込み
            modelRender_ = std::make_unique<ModelRender>();
            modelRender_->Init(path);
            modelRender_->SetTRS(transform.position, transform.rotation, transform.scale);
            modelRender_->Update();

            // バウンディングボックス計算
            app::collision::Bounds bounds;
            bounds.Compute(modelRender_->GetModel());
            Vector3 size = bounds.maxPoint - bounds.minPoint;
            size.x *= transform.scale.x;
            size.y *= transform.scale.y;
            size.z *= transform.scale.z;

            // モデルの底面のY座標を計算（スケール適用済み）
            float bottomY = transform.position.y + bounds.minPoint.y * transform.scale.y + GROUND_OFFSET_Y;
            // 壁用
            float wallBottomY = transform.position.y + bounds.minPoint.y * transform.scale.y + WALL_OFFSET_Y;
            // 地面の当たり判定
            {
                Quaternion rot;
                rot.SetRotationDegX(90.0f);
                Vector3 pos = transform.position;
                pos.y = bottomY + size.y * HALF;

                auto body = std::make_unique<app::collision::PhysicalBody>();
                body->CreateBox(size, pos, app::collision::CollisionAttribute::Ground, rot);
                physicalBodies_.push_back(std::move(body));
            }
            // リング型の壁
            {
                float radius = size.x * WALL_RADIUS;
                float angleStep = FULL_CIRCLE_DEG / WALL_COUNT;
                float wallWidth = 2.0f * radius * sinf(angleStep * HALF * DEG_TO_RAD) + WALL_WIDTH_MARGIN;

                for (int i = 0; i < WALL_COUNT; i++)
                {
                    float angleDeg = angleStep * i;
                    float angleRad = angleDeg * DEG_TO_RAD;

                    // 円周上の位置
                    Vector3 pos = transform.position;
                    pos.x += radius * sinf(angleRad);
                    pos.z += radius * cosf(angleRad);
                    pos.y = wallBottomY + WALL_HEIGHT * HALF + WALL_EXTRA_Y;

                    // 円の接線方向に向ける回転
                    Quaternion rot;
                    rot.SetRotationDegY(angleDeg);

                    Vector3 wallSize(wallWidth, WALL_HEIGHT, WALL_THICKNESS);

                    auto body = std::make_unique<app::collision::PhysicalBody>();
                    body->CreateBox(wallSize, pos, app::collision::CollisionAttribute::Ground, rot);
                    physicalBodies_.push_back(std::move(body));
                }
            }
        }
	}
}
