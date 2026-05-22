#pragma once
#include <vector>


namespace app
{
	namespace actor
	{
		/// <summary>
		/// 敵の各フェーズのデータ
		/// </summary>
		struct EnemyPhaseData
		{
			int requiredPlayerLevel;
			float hp;
			float attackPower;
			float moveSpeed;
		};


		class EnemyPhase
		{
		private:
			std::vector<EnemyPhaseData> phases_;	/// 各フェーズのデータ
			int currentIndex_ = -1;					/// 現在のフェーズのインデックス
			int phaseCount_ = 0;					/// フェーズの数


		public:
			/// <summary>
			/// コンストラクタ
			/// </summary>
			EnemyPhase() = default;


			/// <summary>
			/// フェーズのデータをセットする
			/// </summary>
			/// <param name="phases"> フェーズのデータのリスト</param>
			void SetPhases(const std::vector<EnemyPhaseData>& phases)
			{
				phases_ = phases;
				currentIndex_ = -1;
			}


			/// <summary>
			/// プレイヤーのレベルに応じてフェーズを更新する
			/// </summary>
			/// <param name="playerLevel"> プレイヤーのレベル</param>
			/// <param name="outData"> 更新後のフェーズのデータ</param>
			/// <returns> フェーズが更新されたかどうか</returns>
			bool Update(int playerLevel, EnemyPhaseData& outData);

			void Reset() 
			{ 
				currentIndex_ = -1; 
				phaseCount_ = 0;
			}

			bool TryGetCurrentPhaseData(EnemyPhaseData& outData) const
			{
				if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(phases_.size()))
				{
					return false;
				}
				outData = phases_[currentIndex_];
				return true;
			}

			int GetCurrentIndex() const { return currentIndex_; }
			int GetPhaseCount() const { return phaseCount_; }
		};


		class PhaseUI : public IGameObject
		{
		private:
			FontRender fontRender_;		// フォントレンダラー
			int currentPhase_ = -1;		// 現在のフェーズ

		public:
			PhaseUI();
			~PhaseUI() {}
			void Update();
			void Render(RenderContext& rc);

			void SetPhaseCount(int count) 
			{ 
				if (currentPhase_ == count) return;
				currentPhase_ = count;

				std::wstring wstr = L"Phase " + std::to_wstring(count);
				fontRender_.SetText(wstr.c_str());	
			}

		};
	}
}