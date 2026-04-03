/**
 * UIAnimation.h
 * UI用のアニメーション処理群
 */
#pragma once

#include <functional>       // std::function
#include "util/Curve.h"     // EasingType, LoopMode

namespace app
{
	namespace ui
	{
		class UIBase;


		template <typename T>
		using UIAnimationApplyFunc = std::function<void(const T&)>;

		
		/** UIアニメーション基底クラス */
		class UIAnimationBase
		{
		protected:
			UIBase* ui_ = nullptr;

			float timeSec_ = 0.0f;
			app::util::EasingType type_ = app::util::EasingType::Linear;
			app::util::LoopMode loopMode_ = app::util::LoopMode::Once;

		public:
			UIAnimationBase() {}
			~UIAnimationBase() {}

			virtual void Update() = 0;
			virtual void Play() = 0;
			virtual void Stop() = 0;
			virtual bool IsPlay() = 0;

			void SetUI(UIBase* ui) { ui_ = ui; }
		};




		/***************************************/


		/** Floatのアニメーション */
		class UIFloatAnimation :public UIAnimationBase {
		protected:
			app::util::FloatCurve curve_;
			/** カーブ用のパラメーター */
			float start_ = 0.0f;
			float end_ = 0.0f;

			UIAnimationApplyFunc<float> applyFunc_;


		public:
			UIFloatAnimation() {}
			~UIFloatAnimation() {}

			/** 更新 */
			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}

			/** 再生 */
			void Play() override
			{
				curve_.Play();
			}

			/** 停止 */
			void Stop() override
			{
				curve_.Stop();
			}

			/** 再生中か */
			bool IsPlay() override
			{
				return curve_.IsPlaying();
			}

			/** UIアニメーションのパラメーターを設定 */
			void SetParameter(float start, float end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				start_ = start;
				end_ = end;
				timeSec_ = timeSec;
				type_ = type;
				loopMode_ = loopMode;
				curve_.Initialize(start_, end_, timeSec_, type_, loopMode_);
			}

			/** アニメーション中の現在の値を取得 */
			float GetCurrendtValue()
			{
				return curve_.GetCurrentValue();
			}

			/** アニメーション後の情報を適用する関数を設定 */
			void SetFunc(const UIAnimationApplyFunc<float>& func)
			{
				applyFunc_ = func;
			}
		};

		/** 2Dアニメーション */
		class  UIVector2Animation : public UIAnimationBase
		{
		protected:
			app::util::Vector2Curve curve_;
			Vector2 start_ = Vector2::Zero;
			Vector2 end_ = Vector2::Zero;
			UIAnimationApplyFunc<Vector2> applyFunc_;


		public:
			UIVector2Animation() {}
			~UIVector2Animation() {}

			/** 更新 */
			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}

			/** 再生 */
			void Play() override
			{
				curve_.Play();
			}

			/** 停止 */
			void Stop() override
			{
				curve_.Stop();
			}

			/** 再生してるか */
			bool IsPlay() override
			{
				return curve_.IsPlaying();
			}

			/** UIアニメーションの情報を設定 */
			void SetParameter(Vector2 start, Vector2 end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				start_ = start;
				end_ = end;
				timeSec_ = timeSec;
				type_ = type;
				loopMode_ = loopMode;
				curve_.Initialize(start_, end_, timeSec_, type_, loopMode_);
			}

			/** アニメーション中の現在の値を取得 */
			Vector2 GetCurrentValue()
			{
				return curve_.GetCurrentValue();
			}

			/** アニメーション後の情報を適用する関数を設定 */
			void SetFunc(const UIAnimationApplyFunc<Vector2>& func)
			{
				applyFunc_ = func;
			}
		};




		/***************************************/


		/** Vector3のアニメーション(拡縮、座標、回転など) */
		class UIVector3Animation : public UIAnimationBase
		{
		protected:
			app::util::Vector3Curve curve_;
			/** カーブ用のパラメーター */
			Vector3 start_ = Vector3::Zero;
			Vector3 end_ = Vector3::Zero;
			float timeSec_ = 0.0f;
			app::util::EasingType type_ = app::util::EasingType::Linear;
			app::util::LoopMode loopMode_ = app::util::LoopMode::Once;

			UIAnimationApplyFunc<Vector3> applyFunc_;


		public:
			UIVector3Animation() {}
			~UIVector3Animation() {}

			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}

			void Play() override
			{
				curve_.Play();
			}

			void Stop() override
			{
				curve_.Stop();
			}

			bool IsPlay() override
			{
				return curve_.IsPlaying();
			}

			void SetParameter(Vector3 start, Vector3 end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				start_ = start;
				end_ = end;
				timeSec_ = timeSec;
				type_ = type;
				loopMode_ = loopMode;
				curve_.Initialize(start_, end_, timeSec_, type_, loopMode_);
			}

			Vector3 GetCurrentValue()
			{
				return curve_.GetCurrentValue();
			}

			void SetFunc(const UIAnimationApplyFunc<Vector3>& func)
			{
				applyFunc_ = func;
			}
		};




		/*****************************************/


		/** Vector4のアニメーション(color~~) */
		class UIVector4Animation : public UIAnimationBase
		{
		protected:
			app::util::Vector4Curve curve_;
			/** カーブ用のパラメーター */
			Vector4 start_ = Vector4::White;
			Vector4 end_ = Vector4::White;
			float timeSec_ = 0.0f;
			app::util::EasingType type_ = app::util::EasingType::Linear;
			app::util::LoopMode loopMode_ = app::util::LoopMode::Once;

			UIAnimationApplyFunc<Vector4> applyFunc_;

		public:
			UIVector4Animation() {}
			~UIVector4Animation() {}

			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}

			void Play() override
			{
				curve_.Play();
			}

			void Stop() override
			{
				curve_.Stop();
			}

			bool IsPlay() override
			{
				return curve_.IsPlaying();
			}

			void SetParameter(Vector4 start, Vector4 end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				start_ = start;
				end_ = end;
				timeSec_ = timeSec;
				type_ = type;
				loopMode_ = loopMode;
				curve_.Initialize(start_, end_, timeSec_, type_, loopMode_);
			}

			Vector4 GetCurrendtValue()
			{
				return curve_.GetCurrentValue();
			}

			void SetFunc(const UIAnimationApplyFunc<Vector4>& func)
			{
				applyFunc_ = func;
			}
		};




		/*******************************/


		/** カラーアニメーション */
		class UIColorAnimation : public UIVector4Animation
		{
		public:
			UIColorAnimation();
			~UIColorAnimation() {}

			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}
		};




		/** 拡縮アニメーション */
		class UIScaleAnimation : public UIVector3Animation
		{
		public:
			UIScaleAnimation();
			~UIScaleAnimation() {}

			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}
		};




		/** 座標アニメーション */
		class UITranslateAniamtion : public UIVector3Animation
		{
		public:
			UITranslateAniamtion();
			~UITranslateAniamtion() {}

			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}
		};




		/** 差分アニメーション */
		class UITranslateOffsetAnimation : public UIVector3Animation
		{
		public:
			UITranslateOffsetAnimation();
			~UITranslateOffsetAnimation() {}

			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}
		};




		/** 回転アニメーション */
		class UIRotationAnimation : public UIFloatAnimation
		{
		public:
			UIRotationAnimation();
			~UIRotationAnimation() {}

			void Update() override
			{
				bool wasPlaying = curve_.IsPlaying();
				curve_.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 今フレームで終了した瞬間に適用
				if (curve_.IsPlaying() || wasPlaying) {
					applyFunc_(curve_.GetCurrentValue());
				}
			}
		};




		/*******************************/


		/**
		 * シーケンス内の1ステップ
		 */
		struct UIAnimationStep
		{
			uint32_t animationKey = 0;        // 再生するアニメーションのキー
			float delayBefore = 0.0f;         // このステップ開始前の待機時間
			std::function<void()> onStart;    // 開始時コールバック
			std::function<void()> onComplete; // 完了時コールバック
		};


		/**
		 * アニメーションシーケンス
		 * 複数のアニメーションを順番に再生する
		 *
		 * 使用例:
		 *   UIAnimationSequence seq;
		 *   seq.Add(Hash32("FadeIn"))
		 *      .Add(Hash32("ScaleUp"), 0.1f)   // 0.1秒待ってから
		 *      .Add(Hash32("SlideOut"));
		 *   seq.Play(uiBase);
		 */
		class UIAnimationSequence
		{
		private:
			std::vector<UIAnimationStep> steps_;
			int currentIndex_ = -1;
			bool isPlaying_ = false;
			float delayTimer_ = 0.0f;
			bool waitingDelay_ = false;

			UIBase* target_ = nullptr;

			std::function<void()> onSequenceComplete_;


		public:
			UIAnimationSequence() {}
			~UIAnimationSequence() {}


			/**
			 * ステップを追加（メソッドチェーン対応）
			 */
			UIAnimationSequence& Add(uint32_t animKey, float delayBefore = 0.0f)
			{
				UIAnimationStep step;
				step.animationKey = animKey;
				step.delayBefore = delayBefore;
				steps_.push_back(step);
				return *this;
			}

			/**
			 * コールバック付きステップ追加
			 */
			UIAnimationSequence& Add(uint32_t animKey, float delayBefore,
				std::function<void()> onStart,
				std::function<void()> onComplete = nullptr)
			{
				UIAnimationStep step;
				step.animationKey = animKey;
				step.delayBefore = delayBefore;
				step.onStart = std::move(onStart);
				step.onComplete = std::move(onComplete);
				steps_.push_back(step);
				return *this;
			}

			/**
			 * シーケンス全体の完了コールバック
			 */
			UIAnimationSequence& OnComplete(std::function<void()> callback)
			{
				onSequenceComplete_ = std::move(callback);
				return *this;
			}

			/**
			 * 再生開始
			 */
			void Play(UIBase* target)
			{
				if (steps_.empty()) return;
				target_ = target;
				currentIndex_ = -1;
				isPlaying_ = true;
				AdvanceToNext();
			}

			/**
			 * 停止
			 */
			void Stop()
			{
				isPlaying_ = false;
				currentIndex_ = -1;
				waitingDelay_ = false;
			}

			/**
			 * 毎フレーム更新
			 */
			void Update(float deltaTime);

			bool IsPlaying() const { return isPlaying_; }

			/**
			 * ステップをクリア
			 */
			void Clear()
			{
				steps_.clear();
				Stop();
			}


		private:
			void AdvanceToNext()
			{
				currentIndex_++;
				if (currentIndex_ >= static_cast<int>(steps_.size())) {
					// 全ステップ完了
					isPlaying_ = false;
					if (onSequenceComplete_) onSequenceComplete_();
					return;
				}

				const auto& step = steps_[currentIndex_];
				if (step.delayBefore > 0.0f) {
					delayTimer_ = step.delayBefore;
					waitingDelay_ = true;
				}
				else {
					StartCurrentStep();
				}
			}

			void StartCurrentStep();
		};
	}
}