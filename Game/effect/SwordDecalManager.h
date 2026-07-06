/**
 * 剣痕デカール管理
 */
#pragma once
#include <memory>
#include <vector>


namespace app
{
    namespace effect
    {
        class SwordDecalManager
        {
        public:
            /** デカール用シェーダーに渡す定数バッファ */
            struct DecalCB
            {
                /** フェードの残り割合 */
                float fadeRatio = 1.0f;
                /** 定数バッファの16バイトアライン用パディング */
                float pad[3]    = {};
            };

        private:
            /** プール内で管理するデカール1枚分の情報 */
            struct DecalInstance
            {
                /** 表示に使用するモデル */
                std::unique_ptr<ModelRender> model;
                /** シェーダーへ渡すフェード情報 */
                DecalCB     cbData;
                /** 読み込んでいるTKMファイルのパス */
                const char* tkmPath  = nullptr;
                /** 生成されてからの経過時間 */
                float age            = 0.0f;
                /** 表示を続ける時間 */
                float lifetime       = 0.0f;
                /** 現在表示中か */
                bool  isActive       = false;
            };


        public:
            /** マネージャー生成 */
            static void Initialize();
            /** マネージャー破棄 */
            static void Finalize();
            /** マネージャー取得 */
            static SwordDecalManager& Get() { return *instance_; }
            /** マネージャーが使用可能か */
            static bool IsAvailable() { return instance_ != nullptr; }

            /** デカール生成 */
            void SpawnDecal(const Vector3& hitPos,
                            const Vector3& surfaceNormal,
                            const Vector3& slashDir);

            /** きのこモンスターの床デカール */
            void SpawnMushroomFloorDecal(const Vector3& hitPos,
                                         const Vector3& surfaceNormal,
                                         const Vector3& slashDir);

            /** 溜め攻撃の壁デカール */
            void SpawnChargeDecal(const Vector3& hitPos,
                                  const Vector3& surfaceNormal,
                                  const Vector3& slashDir,
                                  int chargeLevel);

            /** 溜め攻撃の地響き床デカール */
            void SpawnChargeFloorDecal(const Vector3& hitPos,
                                       const Vector3& surfaceNormal,
                                       const Vector3& slashDir,
                                       int chargeLevel);

            void Update();
            void Render(RenderContext& rc);


        private:
            SwordDecalManager();
            ~SwordDecalManager();

            /** ModelRenderの確保 */
            ModelRender* AllocModel(const char* useTkm);
            /** デカール生成の共通処理 */
            void SpawnDecalInternal(const Vector3& hitPos,
                                     const Vector3& surfaceNormal,
                                     const Vector3& slashDir,
                                     const char*                    overrideTkm,
                                     float                          overrideScale = 0.0f);

            /** デカールの回転 */
            static Quaternion CalcDecalRotation(
                const Vector3& surfaceNormal,
                const Vector3& slashDir);

            /** デカールの再利用用プール */
            std::vector<DecalInstance> pool_;
            /** 次に確保を試すプール番号 */
            int           nextSlot_ = 0;

            /** マネージャーの唯一のインスタンス */
            static SwordDecalManager* instance_;
        };
    }
}
