#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>
#include <type_traits>
#include <stdexcept>


// ==========================================
// ThreadPool クラス (シングルトン)
// 
// ThreadPoolとは
// 複数のスレッドを管理し、タスクを並列に実行するためのクラス
// タスクは関数オブジェクトとしてキューに追加され、ワーカースレッドがそれらを取り出して実行する
// ThreadPoolはシングルトンとして実装されており、アプリケーション全体で共有される
// ==========================================
namespace app
{
    namespace core
    {
        class ThreadPool
        {
        private:
            std::vector<std::thread> m_workers;
            std::queue<std::function<void()>> m_tasks;
            std::mutex m_queueMutex;
            std::condition_variable m_condition;
            bool m_isStop;


        private:
            ThreadPool(size_t threads) : m_isStop(false)
            {
                SetupThreads(threads);
            }


        public:
            ~ThreadPool()
            {
                {
                    std::unique_lock<std::mutex> lock(m_queueMutex);
                    m_isStop = true;
                }
                m_condition.notify_all();
                for (std::thread& worker : m_workers) {
                    if (worker.joinable()) worker.join();
                }
            }

            // コピー禁止
            ThreadPool(const ThreadPool&) = delete;
            ThreadPool& operator=(const ThreadPool&) = delete;

            // スレッドセットアップ
            void SetupThreads(size_t threads) {
                if (threads == 0) threads = 1;
                for (size_t i = 0; i < threads; ++i)
                {
                    // ワーカースレッド生成
                    // NOTE: ワーカースレッドとは
                    //       タスクキューからタスクを取り出して実行するスレッドのこと
                    m_workers.emplace_back([this]
                        {
                            // ワーカースレッドループ
                            for (;;)
                            {
                                std::function<void()> task;
                                {
                                    // ロック
                                    std::unique_lock<std::mutex> lock(this->m_queueMutex);
                                    // タスクが来るまで待機
                                    this->m_condition.wait(lock, [this]
                                        {
                                            return this->m_isStop || !this->m_tasks.empty();
                                        });
                                    // 終了条件
                                    if (this->m_isStop && this->m_tasks.empty()) {
                                        return;
                                    }
                                    // タスク取り出し
                                    task = std::move(this->m_tasks.front());
                                    this->m_tasks.pop();
                                }
                                task();
                            }
                        });
                }
            }

            // スレッド数取得
            size_t GetThreadCount() const { return m_workers.size(); }

        public:
            // NOTE: 初回実行前にスレッド数を指定したい場合に使用してください
            void Initialize(const size_t threads)
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                if (!m_workers.empty()) return;
                SetupThreads(threads);
            }

            // タスク投入メソッド
            template<class F, class... Args>
            auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
            {
                using return_type = typename std::result_of<F(Args...)>::type;

                // タスクを共有ポインタで包む（C++14での packaged_task の移動制限を回避）
                auto task = std::make_shared<std::packaged_task<return_type()>>
                    (
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                std::future<return_type> res = task->get_future();
                {
                    std::unique_lock<std::mutex> lock(m_queueMutex);
                    if (m_isStop) {
                        throw std::runtime_error("enqueue on stopped ThreadPool");
                    }
                    // タスクキューに追加
                    m_tasks.emplace([task]() { (*task)(); });
                }
                // ワーカースレッドに通知
                m_condition.notify_one();
                return res;
            }


        public:
            static ThreadPool& GetInstance()
            {
                static ThreadPool instance(std::thread::hardware_concurrency());
                return instance;
            }
        };
    }
}