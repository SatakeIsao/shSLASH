#pragma once

#include "actor/types.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <random>


namespace app
{
	namespace actor
	{

		/** イベントキャラクターのスポーンする象限 */
		enum class SpawnDirection
		{
			// 4隅
			NORTH_WEST,
			NORTH_EAST,
			SOUTH_WEST,
			SOUTH_EAST,
			// 4辺の中央
			NORTH,
			SOUTH,
			WEST, 
			EAST, 
			// 北辺の1/4・3/4地点
			NORTH_WEST_MID, 
			NORTH_EAST_MID, 

			MAX             
		};


		/**
		 * スポーン象限を管理するクラス
		 * 責務：
		 *   - 初期スポーン時の象限シャッフルキュー管理
		 *   - 使用中象限のフラグ管理
		 *   - 空き象限のランダム選択（敵死亡後の再スポーン用）
		 */
		class QuadrantManager
		{
		public:
			QuadrantManager() = default;
			~QuadrantManager() = default;


			/**
			 * 初期化
			 * 全象限をシャッフルしてキューに積む
			 */
			void Initialize()
			{
				// キューをリセット
				while (!queue_.empty()) { queue_.pop(); }

				// 全象限をシャッフルしてキューに積む
				std::vector<SpawnDirection> directions = {
					SpawnDirection::NORTH_WEST,
					SpawnDirection::NORTH_EAST,
					SpawnDirection::SOUTH_WEST,
					SpawnDirection::SOUTH_EAST,
					SpawnDirection::NORTH,
					SpawnDirection::SOUTH,
					SpawnDirection::WEST,
					SpawnDirection::EAST,
					SpawnDirection::NORTH_WEST_MID,
					SpawnDirection::NORTH_EAST_MID,
				};

				std::shuffle(directions.begin(), directions.end(), std::mt19937(std::random_device{}()));

				for (const auto& dir : directions)
				{
					queue_.push(dir);
				}

				// 使用中フラグをリセット
				std::fill(std::begin(used_), std::end(used_), false);
			}


			/**
			 * 次のスポーン先象限を取得する
			 * 初期スポーン中はキューから順番に取得
			 * キューが空の場合（敵死亡後の再スポーン）は空き象限からランダムに選択
			 */
			SpawnDirection GetNext()
			{
				if (!queue_.empty())
				{
					const SpawnDirection dir = queue_.front();
					queue_.pop();
					used_[static_cast<int>(dir)] = true;
					return dir;
				}

				return GetAvailableRandom();
			}


			/**
			 * 象限を解放する
			 * 敵が死亡したときに呼ぶ
			 */
			void Release(SpawnDirection direction)
			{
				used_[static_cast<int>(direction)] = false;
			}


			/**
			 * 空き象限があるか
			 */
			bool HasAvailable() const
			{
				for (int i = 0; i < static_cast<int>(SpawnDirection::MAX); ++i)
				{
					if (!used_[i]) { return true; }
				}
				return false;
			}


		private:
			/**
			 * 空き象限からランダムに1つ選ぶ（敵死亡後の再スポーン用）
			 * 選ばれた象限は使用済みにする
			 */
			SpawnDirection GetAvailableRandom()
			{
				std::vector<SpawnDirection> available;
				for (int i = 0; i < static_cast<int>(SpawnDirection::MAX); ++i)
				{
					if (!used_[i])
					{
						available.push_back(static_cast<SpawnDirection>(i));
					}
				}

				// フォールバック：全象限埋まっていたらランダム（本来は起きないはず）
				if (available.empty())
				{
					return static_cast<SpawnDirection>(rand() % static_cast<int>(SpawnDirection::MAX));
				}

				const SpawnDirection dir = available[rand() % available.size()];
				used_[static_cast<int>(dir)] = true;
				return dir;
			}


		private:
			std::queue<SpawnDirection> queue_;                          // 初期スポーン用キュー
			bool used_[static_cast<int>(SpawnDirection::MAX)] = {};    // 使用中象限フラグ
		};

	}
}