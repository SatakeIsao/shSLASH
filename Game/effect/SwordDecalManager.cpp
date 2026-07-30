/**
 * 剣痕デカール管理
 */
#include "stdafx.h"
#include "effect/SwordDecalManager.h"


namespace
{
    namespace sword_decal
    {
        /** 同時に保持するデカール数 */
        constexpr int   MAX_DECALS = 16;
        /** デカールの表示時間 */
        constexpr float LIFETIME = 3.0f;
        /** 接地面から少し浮かせるための法線方向オフセット */
        constexpr float SURFACE_BIAS = 0.0f;

        /** 壁用デカールモデル */
        constexpr const char* ATK_DEFAULT_TKM_PATH = "Assets/ModelData/decal/atkDefaultDecal.tkm";
        /** ストーンモンスターの床用デカールモデル */
        constexpr const char* STONE_FLOOR_TKM_PATH = "Assets/ModelData/decal/stoneDebrisScar.tkm";
        /** きのこモンスターの床用デカールモデル */
        constexpr const char* MUSHROOM_FLOOR_TKM_PATH = "Assets/ModelData/decal/mushroomDebrisScar.tkm";
        /** 溜め攻撃Lv1の壁用デカールモデル */
        constexpr const char* CHARGE_LV1_TKM_PATH = "Assets/ModelData/decal/atkChargeLv1.tkm";
        /** 溜め攻撃Lv2の壁用デカールモデル */
        constexpr const char* CHARGE_LV2_TKM_PATH = "Assets/ModelData/decal/atkChargeLv2.tkm";
        /** 溜め攻撃Lv3の壁用デカールモデル */
        constexpr const char* CHARGE_LV3_TKM_PATH = "Assets/ModelData/decal/atkChargeLv3.tkm";
        /** 溜め攻撃Lv1の床用デカールモデル */
        constexpr const char* CHARGE_FLOOR_LV1_TKM_PATH = "Assets/ModelData/decal/atkChargeLv1_Floor.tkm";
        /** 溜め攻撃Lv2の床用デカールモデル */
        constexpr const char* CHARGE_FLOOR_LV2_TKM_PATH = "Assets/ModelData/decal/atkChargeLv2_Floor.tkm";
        /** 溜め攻撃Lv3の床用デカールモデル */
        constexpr const char* CHARGE_FLOOR_LV3_TKM_PATH = "Assets/ModelData/decal/atkChargeLv3_Floor.tkm";

        /** 床と判定する法線Y成分のしきい値 */
        constexpr float FLOOR_DOT_THRESHOLD = 0.999f;
        /** 斬撃方向が短すぎる場合にフォールバックするしきい値 */
        constexpr float MIN_DIRECTION_LENGTH_SQ = 0.001f;
        /** 床デカールのY座標補正 */
        constexpr float FLOOR_OFFSET_Y = -43.0f;
        /** 壁デカールのY座標補正 */
        constexpr float WALL_OFFSET_Y = 20.0f;
        /** 床デカールのスケール */
        constexpr float FLOOR_SCALE = 1.5f;
        /** 壁デカールのスケール */
        constexpr float WALL_SCALE = 50.0f;
        /** 寿命の何割を過ぎたらフェードを開始するか */
        constexpr float FADE_START_RATIO = 0.6f;
        /** 地響きデカール Level1 の基準スケール（要調整） */
        constexpr float QUAKE_FLOOR_BASE_SCALE = 2.5f;
        /** 地響きデカールのY座標補正（地面の高さちょうどだとZファイティングするため僅かに浮かせる） */
        constexpr float QUAKE_FLOOR_OFFSET_Y = 3.0f;
    }
}


namespace app
{
    namespace effect
    {
        /** 静的メンバ定義 */
        SwordDecalManager* SwordDecalManager::instance_ = nullptr;


        SwordDecalManager::SwordDecalManager()
            : pool_(sword_decal::MAX_DECALS)
        {
        }


        SwordDecalManager::~SwordDecalManager()
        {
            for (int i = 0; i < sword_decal::MAX_DECALS; ++i) {
                pool_[i].model.reset();
                pool_[i].isActive = false;
            }
        }


        void SwordDecalManager::Initialize()
        {
            if (!instance_) {
                instance_ = new SwordDecalManager();
            }
        }


        void SwordDecalManager::Finalize()
        {
            if (instance_) {
                delete instance_;
                instance_ = nullptr;
            }
        }


        ModelRender* SwordDecalManager::AllocModel(const char* useTkm)
        {
            int slot = -1;
            for (int i = 0; i < sword_decal::MAX_DECALS; ++i) {
                int candidate = (nextSlot_ + i) % sword_decal::MAX_DECALS;
                DecalInstance& e = pool_[candidate];
                if (!e.isActive && (!e.model || e.tkmPath == useTkm)) {
                    slot = candidate;
                    break;
                }
            }
            if (slot < 0) return nullptr;

            nextSlot_ = (slot + 1) % sword_decal::MAX_DECALS;

            DecalInstance& entry = pool_[slot];

            if (!entry.model) {
                entry.model = std::make_unique<ModelRender>();
                entry.model->Init(
                    useTkm,
                    nullptr, 0,
                    enModelUpAxisY,
                    false,
                    false,
                    nullptr,
                    "Assets/Shader/decal_gbuffer.fx",
                    &entry.cbData,
                    static_cast<int>(sizeof(entry.cbData))
                );
                entry.tkmPath = useTkm;
            }

            entry.cbData.fadeRatio = 1.0f;
            entry.model->SetVisible(true);
            entry.age      = 0.0f;
            entry.lifetime = sword_decal::LIFETIME;
            entry.isActive = true;

            return entry.model.get();
        }


        Quaternion SwordDecalManager::CalcDecalRotation(const Vector3& surfaceNormal,const Vector3& slashDir)
        {
            const Vector3 worldUp = Vector3::Up;

            Quaternion rot;
            float dotVal = surfaceNormal.Dot(worldUp);
            if (dotVal > sword_decal::FLOOR_DOT_THRESHOLD || dotVal < -sword_decal::FLOOR_DOT_THRESHOLD) {
                /** 床: 斬撃方向に合わせてY軸回転 */
                Vector3 slashFwd = slashDir;
                slashFwd.y = 0.0f;
                if (slashFwd.LengthSq() < sword_decal::MIN_DIRECTION_LENGTH_SQ) {
                    slashFwd = Vector3::Front;
                }
                slashFwd.Normalize();
                rot.SetRotationYFromDirectionXZ(slashFwd);
            }
            else {
                /** 壁: 平面が壁に沿い、斬撃方向に向くように回転行列を作成。 */

                /** 斬撃方向を壁面へ射影。 */
                Vector3 slashOnWall = slashDir - surfaceNormal * slashDir.Dot(surfaceNormal);
                if (slashOnWall.LengthSq() < sword_decal::MIN_DIRECTION_LENGTH_SQ) {
                    /** 壁に沿う水平方向を使用。 */
                    slashOnWall = Cross(surfaceNormal, worldUp);
                    if (slashOnWall.LengthSq() < sword_decal::MIN_DIRECTION_LENGTH_SQ) slashOnWall = Vector3::AxisX;
                }
                slashOnWall.Normalize();

                /** 斬撃方向と法線に垂直な、壁面内の方向 */
                Vector3 wallUp = Cross(slashOnWall, surfaceNormal);
                wallUp.Normalize();

                /**
                 * 回転行列の行:
                 * ローカルX (1,0,0) slashOnWall  (斬撃方向に沿うデカール幅)
                 * ローカルY (0,1,0) surfaceNormal (平面が壁の外側を向く)
                 * ローカルZ (0,0,1) wallUp        (壁面内のデカール高さ)
                 */
                Matrix rotMat = Matrix::Identity;
                rotMat._11 = slashOnWall.x;   rotMat._12 = slashOnWall.y;   rotMat._13 = slashOnWall.z;
                rotMat._21 = surfaceNormal.x;  rotMat._22 = surfaceNormal.y;  rotMat._23 = surfaceNormal.z;
                rotMat._31 = wallUp.x;         rotMat._32 = wallUp.y;         rotMat._33 = wallUp.z;
                rot.SetRotation(rotMat);
            }

            return rot;
        }


        void SwordDecalManager::SpawnDecal(const Vector3& hitPos,
                                            const Vector3& surfaceNormal,
                                            const Vector3& slashDir)
        {
            SpawnDecalInternal(hitPos, surfaceNormal, slashDir, nullptr);
        }


        void SwordDecalManager::SpawnMushroomFloorDecal(const Vector3& hitPos,
                                                         const Vector3& surfaceNormal,
                                                         const Vector3& slashDir)
        {
            SpawnDecalInternal(hitPos, surfaceNormal, slashDir, sword_decal::MUSHROOM_FLOOR_TKM_PATH);
        }


        void SwordDecalManager::SpawnChargeDecal(const Vector3& hitPos,
                                                  const Vector3& surfaceNormal,
                                                  const Vector3& slashDir,
                                                  int chargeLevel)
        {
            const char* tkm = sword_decal::CHARGE_LV1_TKM_PATH;
            if (chargeLevel >= 3) {
                tkm = sword_decal::CHARGE_LV3_TKM_PATH;
            }
            else if (chargeLevel == 2) {
                tkm = sword_decal::CHARGE_LV2_TKM_PATH;
            }

            SpawnDecalInternal(hitPos, surfaceNormal, slashDir, tkm, 1.0f);
        }


        void SwordDecalManager::SpawnChargeFloorDecal(const Vector3& hitPos,
                                                     const Vector3& surfaceNormal,
                                                     const Vector3& slashDir,
                                                     int chargeLevel)
        {
            const float scale = sword_decal::QUAKE_FLOOR_BASE_SCALE
                * (chargeLevel >= 3 ? 2.0f : chargeLevel == 2 ? 1.5f : 1.0f);

            const char* tkm = sword_decal::CHARGE_FLOOR_LV1_TKM_PATH;
            if (chargeLevel >= 3)     tkm = sword_decal::CHARGE_FLOOR_LV3_TKM_PATH;
            else if (chargeLevel == 2) tkm = sword_decal::CHARGE_FLOOR_LV2_TKM_PATH;

            ModelRender* model = AllocModel(tkm);
            if (!model) return;

            int lastSlot = (nextSlot_ - 1 + sword_decal::MAX_DECALS) % sword_decal::MAX_DECALS;
            pool_[lastSlot].lifetime = sword_decal::LIFETIME;

            Vector3 decalPos = hitPos;
            decalPos.y += sword_decal::QUAKE_FLOOR_OFFSET_Y;

            model->SetPosition(decalPos);
            model->SetRotation(CalcDecalRotation(Vector3::Up, slashDir));
            model->SetScale({ scale, 1.0f, scale });
            model->Update();
        }


        void SwordDecalManager::SpawnDecalInternal(const Vector3& hitPos,
                                                    const Vector3& surfaceNormal,
                                                    const Vector3& slashDir,
                                                    const char*    overrideTkm,
                                                    float          overrideScale)
        {
            float dotUp  = surfaceNormal.Dot(Vector3::Up);
            bool isFloor = (dotUp > sword_decal::FLOOR_DOT_THRESHOLD || dotUp < -sword_decal::FLOOR_DOT_THRESHOLD);

            const char* tkm = sword_decal::ATK_DEFAULT_TKM_PATH;
            if (overrideTkm) {
                tkm = overrideTkm;
            }
            else if (isFloor) {
                tkm = sword_decal::STONE_FLOOR_TKM_PATH;
            }

            ModelRender* model = AllocModel(tkm);
            if (!model) return;

            /** AllocModelで確保したスロットに寿命を設定 */
            int lastSlot = (nextSlot_ - 1 + sword_decal::MAX_DECALS) % sword_decal::MAX_DECALS;
            pool_[lastSlot].lifetime = sword_decal::LIFETIME;

            Vector3 decalPos = hitPos + surfaceNormal * sword_decal::SURFACE_BIAS;
            if (isFloor) {
                /** 床 */
                decalPos.y += sword_decal::FLOOR_OFFSET_Y;
            } else {
                /** 壁 */
                decalPos.y += sword_decal::WALL_OFFSET_Y;
            }

            model->SetPosition(decalPos);
            model->SetRotation(CalcDecalRotation(surfaceNormal, slashDir));

            float s = sword_decal::WALL_SCALE;
            if (overrideScale > 0.0f) {
                s = overrideScale;
            }
            else if (isFloor) {
                s = sword_decal::FLOOR_SCALE;
            }

            model->SetScale({ s, 1.0f, s });
            model->Update();
        }


        void SwordDecalManager::Update()
        {
            const float dt = g_gameTime->GetFrameDeltaTime();

            for (int i = 0; i < sword_decal::MAX_DECALS; ++i) {
                DecalInstance& entry = pool_[i];
                if (!entry.isActive) continue;

                entry.age += dt;

                /** ディザフェード更新 */
                float fadeStart = entry.lifetime * sword_decal::FADE_START_RATIO;
                if (entry.age > fadeStart) {
                    entry.cbData.fadeRatio = 1.0f - (entry.age - fadeStart) / (entry.lifetime - fadeStart);
                }

                if (entry.age >= entry.lifetime) {
                    entry.model->SetVisible(false);
                    entry.isActive = false;
                    continue;
                }

                entry.model->Update();
            }
        }


        void SwordDecalManager::Render(RenderContext& rc)
        {
            for (int i = 0; i < sword_decal::MAX_DECALS; ++i) {
                DecalInstance& entry = pool_[i];
                if (!entry.isActive) continue;
                g_renderingEngine->AddRenderObject(entry.model.get());
            }
        }
    }
}
