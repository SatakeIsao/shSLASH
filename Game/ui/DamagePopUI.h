/**
 * DamagePopObject.h
 *
 * 敵にダメージを与えたとき、頭上に数字が浮かぶUIオブジェクト。
 *
 * 設計方針:
 *   - 命名      : 既存の TimerUIObject / PlayerHpUIObject に合わせ "〜UIObject" サフィックス
 *   - レイアウト: SoundOptionMenu と同様に Layout + UIDigit::SetNumber() で管理
 *                 スプライトを C++ 側で直接持たない
 *   - プール    : EnemyHpUIObject と同様に IPoolable を実装
 *   - 依存分離  : BattleManager は IDamagePopListener 経由でのみ通知し
 *                 UI クラスを直接 #include しない
 */
#pragma once
#include "actor/EnemyPool.h"
#include "battle/IDamagePopListener.h"
#include "ui/Layout.h"
#include "ui/UIAnimationFactory.h"
#include "ui/UIAnimation.h""


namespace app
{
    namespace ui
    {
        /** DamagePopUIObject */

        /**
         * 1件分のダメージポップ。
         *
         * - Layout を通じて JSON からスプライトを読み込む
         *   （SoundOptionMenu の UIDigit 操作パターンと同一）
         * - IPoolable を実装し、DamagePopPool で使い回す
         *   （EnemyHpUIObject と同一パターン）
         */
        class DamagePopUIObject : public IGameObject, public app::actor::IPoolable
        {
        private:
            /** レイアウト管理 */
            std::unique_ptr<app::ui::Layout> layout_;
            /** 追従するワールド座標（被ダメキャラの足元） */
            Vector3 worldPos_ = Vector3::Zero;
            /** 残り表示時間（秒） */
            float lifeTimer_ = 0.0f;
            /** 上方向への累積スクリーンオフセット（px） */
            float riseOffset_ = 0.0f;           
            /** OnSpawn() に渡せないので事前にセット */
            int     pendingDamage_ = 0;
            Vector3 pendingWorldPos_ = Vector3::Zero;
            bool    pendingIsCritical_ = false;
            bool    isCritical_ = false;
            /** 時間切れ時に自分を Release するためのプール参照 */
            class DamagePopPool* ownerPool_ = nullptr;
            /** フェード・スケールアニメーションシーケンス */
            app::ui::UIAnimationSequence* animSeq_ = nullptr;


            /** 定数 */
            static constexpr char  LAYOUT_PATH[] = "Assets/ui/layout/damagePopLayout.json";
            static constexpr char  DIGIT_UI_NAME[] = "damageNumbers";
            static constexpr char  CRITICAL_DIGIT_UI_NAME[] = "criticalDamageNumbers";
            /** 頭上オフセット */
            static constexpr float WORLD_Y_OFFSET = 90.0f;
            /** 浮上速度（スクリーンpx/秒） */
            static constexpr float RISE_SPEED = 60.0f;
            /** 表示総時間（秒） */
            static constexpr float LIFE_TIME = 1.2f;
            /** フェードアウト開始の残り時間（秒） */
            static constexpr float FADE_START = 0.4f;


        public:
            DamagePopUIObject();
            ~DamagePopUIObject();

            void Update() override;
            void Render(RenderContext& rc) override;

            /** IPoolable */

            /**
             * プールから取り出すとき呼ぶ
             * @param damage    表示するダメージ値（自動で 1〜999 にclamp）
             * @param worldPos  被ダメキャラのワールド座標
             */
             /**
             * EnemyPool::Acquire() が内部で呼ぶ。
             * 事前に SetSpawnParams() を呼ぶ。
             */
            void OnSpawn() override;

            /** EnemyPool::Release() が内部で呼ぶ */
            void OnRecycle() override;

            /** Acquire() の後に呼ぶ。UIDigit に値を反映する */
            void ApplySpawnParams();

            void SetSpawnParams(int damage, const Vector3& worldPos, bool isCritical = false)
            {
                pendingDamage_ = max(1, min(damage, 999));
                pendingWorldPos_ = worldPos;
                pendingIsCritical_ = isCritical;
            }

            void SetOwnerPool(DamagePopPool* pool) { ownerPool_ = pool; }
        };




        
        /**
         * プールへの返却インターフェース
         * DamagePopUIObject が具体型 DamagePopPool を知らずに済む
         **/
        struct IRecyclable
        {
            virtual ~IRecyclable() {}
            virtual void Recycle(DamagePopUIObject* obj) = 0;
        };


        

        /**
         * DamagePopUIObject を EnemyPool<T> で管理するプール。
         * IDamagePopListener を実装しているので、
         * BattleManager::SetDamagePopListener(pool) で登録するだけで動く。
         *
         * 使い方:
         *   // Start()
         *   damagePopPool_ = new DamagePopPool();
         *   damagePopPool_->Initialize(priority);
         *   BattleManager::Get().SetDamagePopListener(damagePopPool_);
         *
         *   // ~BattleManager()
         *   damagePopPool_->Finalize();
         *   delete damagePopPool_;
         */
        class DamagePopPool : public app::battle::IDamagePopListener, public IRecyclable
        {
        public:
            /** 同時に表示できる最大数 */
            static constexpr int POOL_SIZE = 12;

        private:
            app::actor::EnemyPool<DamagePopUIObject, POOL_SIZE> pool_;

        public:
            DamagePopPool() = default;
            ~DamagePopPool() = default;

            /** ゲームオブジェクトを生成してプールを準備 */
            void Initialize();

            /** 全ゲームオブジェクトを破棄 */
            void Finalize();

            // ── IDamagePopListener ─────────────────────────────

            /** 空きスロットがなければ何もしない（最大 POOL_SIZE 件まで同時表示） */
            void OnDamageDealt(int damage, const Vector3& worldPos, bool isCritical = false) override;
            /** IRecyclable: 時間切れになった DamagePopUIObject をプールに返す */
            void Recycle(DamagePopUIObject* obj) override { pool_.Release(obj); }
        };

    }
}