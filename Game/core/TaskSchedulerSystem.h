/**
 * TaskSchedulerSystem.h
 * タスクスケジューラーシステム
 */
#pragma once


namespace app
{
	namespace core
	{
        class TaskSchedulerSystem : public Noncopyable
        {
            using Callback = std::function<void()>;


        private:
            struct Event
            {
                float triggerTime;      // 実行される目標時刻
                Callback callback;
                float interval;         // ループ用の間隔保持
                bool delayNextFrame;    // 実行条件を満たした後、さらに1フレーム待つか
            };

            // ループシーケンス(一定周期でリセットされるタイマー群)
            struct LoopSequence
            {
                float totalDuration;        // ループ1周の長さ（秒）
                float pheseTime;            // このシーケンス内での経過時間（0 〜 totalDuration）
                std::vector<Event> events;  // シーケンス内のイベント（phaseTime基準でソート済み）
                int nextEventIndex;         // 次に確認するイベントのインデックス
            };


        private:
            float currentTime_ = 0.0f;
            std::vector<Event> events_;
            bool needsSort_ = false;
            // ダブルバッファ構造で「次のフレーム」を確実に保証する
            std::vector<Event> currentFrameEvents_;         // 今のフレームで実行するもの
            std::vector<Event> pendingNextFrameEvents_;     // 次のフレームに回すもの


            std::vector<LoopSequence> loopSequences_;       //ループシーケンスリスト

        public:
            TaskSchedulerSystem() {}
			~TaskSchedulerSystem() {}
            

            /**
             * タイマーをスケジュールする
             * @param delay 実行までの秒数
             * @param callback 実行される処理
             * @param loop ループするか
             */
            void AddTimer(float delay, Callback callback, bool delayNextFrame = false)
            {
                float targetTime = currentTime_ + delay;
                events_.push_back({ targetTime, std::move(callback), delay, delayNextFrame });
                needsSort_ = true;
            }

            /**
             * ループシーケンスを作成する
             * @param totalDuration ループ1周の長さ（秒）。この秒数が経過したら0秒目に戻る
             * @return シーケンスID（AddLoopTimerに渡す）
             */
            int CreateLoopSequence(float totalDuration)
            {
                LoopSequence seq;
                seq.totalDuration = totalDuration;
                seq.pheseTime = 0.0f;
                seq.nextEventIndex = 0;
                loopSequences_.push_back(std::move(seq));
                return static_cast<int>(loopSequences_.size()) - 1;
            }

            /**
             * ループシーケンスにタイマーを登録する
             * シーケンスが0秒目にリセットされるたびに、delay秒後にcallbackが呼ばれる
             * @param sequence Id CreateLoopSequenceの戻り値
             * @param delay シーケンス先頭(0秒目)から実行までの秒数
             * @param callback 実行される処理
             * @param delayNextFrame 実行条件を満たした後、さらに1フレーム待つか
             */
            void AddLoopTimer(int sequenceId, float delay, Callback callcack, bool delayNextFrame = false)
            {
                assert(sequenceId >= 0 && sequenceId < static_cast<int>(loopSequences_.size()));
                auto& seq = loopSequences_[sequenceId];
                assert(delay < seq.totalDuration && "delayはtotalDurationよりちいさくしてください");

                seq.events.push_back({ delay,std::move(callcack),0.0f,delayNextFrame });

                // triggerTime（= delay)昇順で挿入ソート
                std::sort(seq.events.begin(), seq.events.end(), [](const Event& a, const Event& b)
                    {
                        return a.triggerTime < b.triggerTime;
                    });
            }

            /**
             * 毎フレームの更新
             * @param deltaTime フレーム経過時間
             */
            void Update(float deltaTime)
            {
                currentTime_ += deltaTime;

                if (needsSort_) {
                    SortEvents();
                    needsSort_ = false;
                }

                // 実行時刻が来たイベントを処理
                for (auto it = events_.begin(); it != events_.end(); ) {
                    if (currentTime_ >= it->triggerTime) {
                        if (it->delayNextFrame) {
                            // 「次のフレームで実行」リストへ移送する
                            pendingNextFrameEvents_.push_back(std::move(*it));
                        } else {
                            // コールバック実行
                            if (it->callback) {
                                it->callback();
                            }
                        }
                        it = events_.erase(it);
                    } else {
                        // ソート済みなので、現在の時刻より先の話であればこれ以上回さない
                        break;
                    }
                }

                // ループシーケンスの更新
                UpdateLoopSequence(deltaTime);

                // 「次フレーム実行」タスクの処理
                // 前回のupdateで「実行待ち」になったものをここで動かす
                std::vector<Event> eventsToExecute;
                eventsToExecute.swap(currentFrameEvents_); // 前回分を吸い出す

                for (const auto& task : eventsToExecute) {
                    if (task.callback) task.callback();
                }

                // 今回のupdateで発生した「次フレーム待ち」を次回の実行用へ移動
                currentFrameEvents_.swap(pendingNextFrameEvents_);
            }

        private:
            void SortEvents()
            {
				// std::vectorのsortを使ってイベントをトリガー時間でソート
                std::sort(events_.begin(), events_.end(), [](const Event& a, const Event& b)
                    {
                        return a.triggerTime < b.triggerTime;
                    });
            }

            /**
             * ループシーケンス群の更新
             * deltaTimeを加算し、totalDurationを超えたら折り返す。
             * その仮定で通過したイベントを発火させる。
             */
            void UpdateLoopSequence(float deltaTime)
            {
                for (auto& seq : loopSequences_) {
                    if (seq.events.empty())
                    {
                        seq.pheseTime += deltaTime;
                        // リセット処理だけ行う
                        while (seq.pheseTime >= seq.totalDuration) {
                            seq.pheseTime -= seq.totalDuration;
                        }
                        continue;
                    }

                    float remaining = deltaTime;

                    // remaining（残り時間）を使い切るまで処理する。
                    // 1フレームのdeltaTimeがtotalDuractionを超える極端なケースにも対応するためループ構造にしている。
                    while (remaining > 0.0f) {
                        float prevPhase = seq.pheseTime;
                        float advanced = (std::min)(remaining, seq.totalDuration - prevPhase);
                        seq.pheseTime += advanced;
                        remaining -= advanced;

                        // prevPhase ～ seq.pheseTime の間に入るイベントを発火
                        FireSequenceEvents(seq, prevPhase, seq.pheseTime);

                        // ループ折り返し
                        if (seq.pheseTime >= seq.totalDuration) {
                            seq.pheseTime -= seq.totalDuration;
                            seq.nextEventIndex = 0; // インデックスを先頭に戻す
                        }
                    }
                }
            }

            /**
             * シーケンス内の[fromPhese, toPhase]区間に含まれるイベントを発火する
             */
            void FireSequenceEvents(LoopSequence& seq, float fromPhese, float toPhase)
            {
                // nextEventIndexから線形スキャン（イベントはtrigger昇順でソート済み）
                for (int i = seq.nextEventIndex; i < static_cast<int>(seq.events.size()); ++i) {
                    const Event& ev = seq.events[i];
                    if (ev.triggerTime >= fromPhese && ev.triggerTime < toPhase) {
                        if (ev.delayNextFrame) {
                            pendingNextFrameEvents_.push_back(ev);
                        }else {
                            if (ev.callback) ev.callback();
                        }
                        seq.nextEventIndex = i + 1;
                    } else if (ev.triggerTime >= toPhase) {
                        break; // ソート済みなのでここで打ち切り
                    }
                }
            }
        };
	}
}