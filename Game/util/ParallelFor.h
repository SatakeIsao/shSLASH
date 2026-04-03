#pragma once
#include "ThreadPool.h"


namespace app
{
    namespace util
    {
        /**
         * インデックスベース
         */
        template <typename Func>
        inline void ParallelFor(size_t start, size_t end, Func body)
        {
            if (start >= end) return;

            auto& pool = core::ThreadPool::GetInstance();
            const size_t total = end - start;
            const size_t numThreads = pool.GetThreadCount();

            // 1スレッドあたりの担当件数
            size_t chunkSize = (total + numThreads - 1) / numThreads;
            // 最小粒度保証
            const size_t minGrain = 100;
            // 粒度調整
            if (chunkSize < minGrain)
            {
                chunkSize = minGrain;
            }

            // タスク投入
            std::vector<std::future<void>> futures;
            for (size_t i = start; i < end; i += chunkSize)
            {
                // チャンクの開始と終了インデックス
                size_t chunk_start = i;
                size_t chunk_end = (std::min)(i + chunkSize, end);
                // タスクをスレッドプールに投入
                futures.emplace_back(pool.Enqueue([chunk_start, chunk_end, body]
                    {
                        for (size_t j = chunk_start; j < chunk_end; ++j) {
                            body(j);
                        }
                    })
                );
            }
            // 全タスク完了待ち
            for (auto& f : futures) {
                f.get();
            }
        }


        /**
         * 直列版インデックス
         * 通常のForと同じ
         * ParallelForと同じ構造にするために用意
         * ParallelForでのデバッグ等で使用される
         * 通常使用しても問題ない
         */
        template <typename Func>
        inline void For(size_t start, size_t end, Func body)
        {
            for (size_t i = start; i < end; ++i) {
                body(i);
            }
        }


        /**
         * 配列用(マルチスレッド用)
         */
        template <typename T, size_t N, typename Func>
        inline void ParallelFor(T(&arr)[N], Func body)
        {
            ParallelFor(size_t(0), N, [&arr, &body](size_t i)
                {
                    body(arr[i]);
                });
        }


        /**
         * 配列用(シングルスレッド用)
         */
        template <typename T, size_t N, typename Func>
        inline void For(T(&arr)[N], Func body)
        {
            for (size_t i = 0; i < N; ++i) body(arr[i]);
        }


        /**
         * コンテナ用(マルチスレッド用)
         * std::vector, std::array等
         */
        template <typename Container, typename Func>
        inline void ParallelFor(Container& c, Func body)
        {
            ParallelFor(size_t(0), c.size(), [&c, &body](size_t i)
                {
                    body(c[i]);
                });
        }


        /**
         * コンテナ用(シングルスレッド用)
         * std::vector, std::array等
         */
        template <typename Container, typename Func>
        inline void For(Container& c, Func body)
        {
            for (auto& item : c) body(item);
        }
    }
}