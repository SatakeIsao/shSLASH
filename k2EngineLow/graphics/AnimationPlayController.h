/*!
 * @brief アニメーション再生コントローラー。
 */
#pragma once

namespace nsK2EngineLow {
	class Skeleton;
	class AnimationClip;
	class Animation;
	class Bone;

	/*!
	 * @brief アニメーションの再生コントローラー。
	 * @details 1つのアニメーションクリップに対してアニメーションを進めて、ローカルポーズを計算します。
	 */

	 /// <summary>
	 /// アニメーションの再生コントローラー。
	 /// </summary>
	 /// <remark>
	 /// 1つのアニメーションクリップに対して、アニメーションを進めてローカルポーズを計算します。
	 /// </remark>
	class AnimationPlayController : public Noncopyable {
	public:
		/*!
		 * @brief コンストラクタ。
		 */
		AnimationPlayController()
		{}
		/*!
		 * @brief デストラクタ。
		 */
		~AnimationPlayController()
		{}
		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="skeleton">スケルトン</param>
		/// <param name="footStepBoneNo">footstepボーンの番号</param>
		void Init(Skeleton* skeleton, int footStepBoneNo);

		/// <summary>
		/// アニメーションクリップの変更
		/// </summary>
		/// <param name="clip">変更後のアニメーションクリップ</param>
		void ChangeAnimationClip(AnimationClip* clip);

		/// <summary>
		/// アニメーション補間時間を設定。
		/// </summary>
		/// <param name="interpolateTime">補間時間</param>
		void SetInterpolateTime(float interpolateTime)
		{
			if (interpolateTime < 0.0f) {
				// interpolateTime is negative!!
				return;
			}
			m_interpolateEndTime = interpolateTime;
			m_interpolateTime = 0.0f;
		}
		/// <summary>
		/// 現在のアニメーション補間率を取得
		/// </summary>
		/// <returns></returns>
		float GetInterpolateRate() const
		{
			if (m_interpolateEndTime <= 0.0f) {
				return 1.0f;
			}
			return min(1.0f, m_interpolateTime / m_interpolateEndTime);
		}

		/// <summary>
		/// アニメーションを進める
		/// </summary>
		/// <param name="deltaTime">アニメーションを進める時間(単位:秒)</param>
		/// <param name="animation"></param>
		/// <param name="isInvokeAnimationEvent"></param>
		void Update(float deltaTime, Animation* animation, bool isInvokeAnimationEvent);


		/// <summary>
		/// ローカルボーン行列を取得。
		/// </summary>
		/// <returns></returns>
		const std::vector<Matrix>& GetBoneLocalMatrix() const
		{
			return m_boneMatrix;
		}

		/// <summary>
		/// アニメーションクリップの取得。
		/// </summary>
		/// <returns></returns>
		AnimationClip* GetAnimClip() const
		{
			return m_animationClip;
		}

		/// <summary>
		/// アニメーションが再生中か判定。
		/// </summary>
		/// <returns></returns>
		bool IsPlaying() const
		{
			return m_isPlaying;
		}
		void SetLoopStartTime(float time) { m_loopStartTime = time; }
		void SetCurrentTime(float time);
		/// <summary>
		/// Update関数が呼び出されたときの、footstepボーンの移動量を取得。
		/// </summary>
		/// <returns></returns>
		Vector3 GetFootStepDeltaValueOnUpdate() const
		{
			return m_footstepDeltaValue;
		}
	private:


		/// <summary>
		/// アニメーションイベントを発火させる。
		/// </summary>
		/// <param name="animation"></param>
		void InvokeAnimationEvent(Animation* animation);

		/// <summary>
		/// ループ再生を開始するときに実行する処理。
		/// </summary>
		void StartLoop();
		/// <summary>
		/// ルートのボーン空間でのボーン行列を計算する。
		/// </summary>
		/// <param name="bone">計算する骨</param>
		/// <param name="parentMatrix">親の行列</param>
		void CalcBoneMatrixInRootBoneSpace(Bone& bone, Matrix parentMatrix);
		/// <summary>
		/// ボーン行列をアニメーションクリップからサンプリングする。
		/// </summary>
		void SamplingBoneMatrixFromAnimationClip();
		/// <summary>
		/// ルートボーン空間での行列を計算する。
		/// </summary>
		void CalcBoneMatrixInRootBoneSpace();
		/// <summary>
		/// footstepボーンの変化量をサンプリングする。
		/// </summary>
		void SamplingDeltaValueFootstepBone();
		/// <summary>
		/// footstepボーンの座標を全体の座標から減算する。
		/// </summary>
		void SubtractFootstepbonePosFromAllBone();
		/// <summary>
		/// キーフレーム番号を進める。
		/// </summary>
		void ProgressKeyframeNo(float deltaTime);
	private:
		AnimationClip* m_animationClip = nullptr;					// アニメーションクリップ。
		int						m_currentKeyFrameNoLastFrame = 0;	// 前フレームのキーフレーム番号。
		int						m_currentKeyFrameNo = 0;			// 現在再生中のキーフレーム番号。
		float					m_time = 0.0f;
		float					m_loopStartTime = 0.0f;
		std::vector<Matrix>		m_boneMatrix;						// このコントローラーで再生中のアニメーションのボーン行列。
		float					m_interpolateTime;					// 補間時間
		float					m_interpolateEndTime;				// 補間終了時間
		bool					m_isPlaying = false;				// 再生中か？
		Skeleton* m_skeleton = nullptr;								// スケルトン。
		Vector3					m_footstepDeltaValue = g_vec3Zero;	// フットステップの移動ベクトル。
		Vector3					m_footstepPos = g_vec3Zero;			// フットステップボーンの座標。
		int						m_footstepBoneNo = -1;				// フットステップのボーン番号。
	};
}