#pragma once
#include "ParallelFor.h"


namespace app
{
    namespace core
    {
        /**
         * ジョブシステムクラス
         * 処理をジョブ単位で管理し依存関係を考慮して並列実行する
         */
        class JobSystem
        {
        public:
            /**
             * ジョブ構造体
             */
            struct Job
            {
                std::function<void()> m_task;                               // ジョブのタスク
                std::atomic<int> m_unfinishedDependencies{ 0 };             // 未完了の依存ジョブ数
                std::vector<std::shared_ptr<Job>> m_dependents;             // 依存しているジョブリスト
                std::vector<std::function<void()>> m_callbacks;             // 完了時に呼ぶ関数リスト
                std::atomic<bool> m_completed{ false };                     // ジョブ完了フラグ
                std::mutex m_syncMutex;                                     // 同期用ミューテックス
                std::condition_variable m_completedConditionVariable;       // 完了通知用条件変数
                //
                Job(const std::function<void()>& t) : m_task(std::move(t)) {}
            };


        public:
            /** ジョブハンドル型定義 */
            using JobHandle = std::shared_ptr<Job>;


        private:
            JobSystem() = default;


        public:
            /** ジョブを作成する */
            JobHandle CreateJob(std::function<void()> task)
            {
                return std::make_shared<Job>(std::move(task));
            }

            /** 完了時に実行するコールバックを登録 */
            void SetCallback(JobHandle job, std::function<void()> callback)
            {
                if (!job) return;
                std::lock_guard<std::mutex> lock(job->m_syncMutex);
                if (job->m_completed) {
                    callback(); // すでに終わっていれば即実行
                } else {
                    job->m_callbacks.push_back(std::move(callback));
                }
            }

            /** 依存関係を追加 */
            void AddDependency(JobHandle parent, JobHandle child)
            {
                if (!parent || !child) return;
                std::lock_guard<std::mutex> lock(parent->m_syncMutex);
                if (parent->m_completed) return;
                child->m_unfinishedDependencies++;
                parent->m_dependents.push_back(child);
            }

            /**
             * ジョブを実行
             * NOTE: 依存関係を考慮
             */
            void Run(JobHandle job)
            {
                if (!job) return;
                if (job->m_unfinishedDependencies == 0) {
                    Submit(job);
                }
            }

            /** 完了したかチェック */
            bool IsCompleted(JobHandle job) const
            {
                return job ? job->m_completed.load() : true;
            }

            /** 終わるまで待機 */
            void Wait(JobHandle job)
            {
                if (!job || job->m_completed) return;
                std::unique_lock<std::mutex> lock(job->m_syncMutex);
                job->m_completedConditionVariable.wait(lock, [&] { return job->m_completed.load(); });
            }


        private:
            /**
             * ジョブをスレッドに投入する
             */
            void Submit(JobHandle job)
            {
                ThreadPool::GetInstance().Enqueue([this, job]
                    {
                        // タスク実行
                        job->m_task();

                        std::vector<JobHandle> ready_jobs;
                        std::vector<std::function<void()>> local_callbacks;
                        {
                            std::lock_guard<std::mutex> lock(job->m_syncMutex);
                            job->m_completed = true;

                            // 後続ジョブの準備
                            for (auto& dep : job->m_dependents) {
                                if (--dep->m_unfinishedDependencies == 0) ready_jobs.push_back(dep);
                            }
                            // コールバックのコピー
                            local_callbacks = std::move(job->m_callbacks);
                        }

                        // 後続ジョブをキューへ
                        for (auto& r : ready_jobs) {
                            Submit(r);
                        }

                        // コールバック実行
                        for (auto& cb : local_callbacks) {
                            cb();
                        }

                        // 完了通知
                        job->m_completedConditionVariable.notify_all();
                    });
            }




        public:
            /**
             * シングルトンインスタンス取得
             */
            static JobSystem& GetInstance()
            {
                static JobSystem instance;
                return instance;
            }
        };
    }
}