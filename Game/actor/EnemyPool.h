#pragma once
#include <array>
#include <stack>

namespace app
{
    namespace actor
    {
        // プール可能なオブジェクトが持つインターフェース
        struct IPoolable
        {
            bool isActive = false;      // プール内で使用中かどうか
            virtual void OnSpawn() = 0; // プールから取り出したとき
            virtual void OnRecycle() = 0; // プールに戻すとき（非表示・リセット）
        };


        template<typename T, int PoolSize>
        class EnemyPool
        {
        public:
            void Initialize()
            {
                for (int i = 0; i < PoolSize; ++i)
                {
                    // 起動時に一括でNewGO → 以降はNewGOしない
                    auto* obj = NewGO<T>(
                        static_cast<uint8_t>(ObjectPriority::Character));
                    //obj->OnRecycle(); // 非アクティブ状態にしておく
                    freeList_.push(obj);
                    pool_[i] = obj;
                }
            }

            // プールから1体取得（空なら nullptr）
            T* Acquire()
            {
                if (freeList_.empty()) { return nullptr; }
                T* obj = freeList_.top();
                freeList_.pop();
               // obj->isActive = true;
                obj->OnSpawn();
                return obj;
            }

            // プールに返却
            void Release(T* obj)
            {
                if (!obj) { return; }
                //obj->isActive = false;
                obj->OnRecycle();
                freeList_.push(obj);
            }

            void Finalize()
            {
                for (auto*& obj : pool_)
                {
                    if (obj) { DeleteGO(obj); obj = nullptr; }
                }
            }

        private:
            std::array<T*, PoolSize> pool_ = {};
            std::stack<T*>           freeList_;
        };
    }
}