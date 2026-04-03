/**
 * BattleSequence.h
 * バトルシーケンス
 */

#pragma once
#include "Layout.h"
#include "UIParts.h"

namespace app
{
	namespace ui
	{
		class BattleSequence : public IGameObject
		{
			enum class SequenceName {
				Wait,
				Ready,
				GO,
				Finished
			};

		private:
			std::unique_ptr<app::ui::Layout> layout_;
			SequenceName currentDown_ = SequenceName::Wait;
			
			float delayTimer_ = 0.5f;
			float maxCountDownTimer_ = 2.0f;
			float goTimer_ = 1.0f;

			UIIcon* readyIcon_ = nullptr;
			UIIcon* goIcon_ = nullptr;


		public:
			BattleSequence();
			~BattleSequence();
		public:
			void Update() override;
			void Render(RenderContext& rc);

		public:
			bool IsPlaying() const {
				return currentDown_ != SequenceName::Wait && currentDown_ != SequenceName::Finished;
			}
		};
	}
}

