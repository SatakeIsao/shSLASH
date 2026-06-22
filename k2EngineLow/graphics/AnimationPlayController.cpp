/*!
 * @brief アニメーション再生コントローラー。
 */
#include "k2EngineLowPreCompile.h"
#include "AnimationPlayController.h"
#include "AnimationClip.h"
#include "Animation.h"
#include "Skeleton.h"

namespace nsK2EngineLow {
	void AnimationPlayController::Init(Skeleton* skeleton, int footStepBoneNo)
	{
		m_footstepBoneNo = footStepBoneNo;
		int numBones = skeleton->GetNumBones();
		// ボーン行列のバッファを確保。
		m_boneMatrix.resize(numBones);
		m_skeleton = skeleton;
	}
	void AnimationPlayController::ChangeAnimationClip(AnimationClip* clip)
	{
		m_animationClip = clip;
		m_currentKeyFrameNo = 0;
		m_time = 0.0f;
		m_isPlaying = true;
		m_footstepPos = g_vec3Zero;
		m_footstepDeltaValue = g_vec3Zero;
		// アニメーションイベントの発火フラグをすべてfalseにする。
		auto& animEventArray = m_animationClip->GetAnimationEvent();
		for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
			animEventArray[i].SetInvokedFlag(false);
		}
	}
	void AnimationPlayController::InvokeAnimationEvent(Animation* animation)
	{
		auto& animEventArray = m_animationClip->GetAnimationEvent();
		for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
			if (m_time > animEventArray[i].GetInvokeTime()
				&& animEventArray[i].IsInvoked() == false) {
				// アニメーションの起動時間を超えている、かつ、まだイベントが発火していない。
				animation->NotifyAnimationEventToListener(
					m_animationClip->GetName(), animEventArray[i].GetEventName()
				);
				animEventArray[i].SetInvokedFlag(true);
			}
		}
	}

	void AnimationPlayController::SetCurrentTime(float time)
	{
		if (m_animationClip == nullptr) return;
		m_time = time;
		const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();
		m_currentKeyFrameNo = 0;
		for (int i = 0; i < (int)topBoneKeyFrameList.size(); i++) {
			if (topBoneKeyFrameList.at(i)->time >= time) {
				m_currentKeyFrameNo = i;
				break;
			}
		}
	}

	void AnimationPlayController::StartLoop()
	{
		m_footstepPos = g_vec3Zero;
		SetCurrentTime(m_loopStartTime);
		// アニメーションイベントをすべて未発火にする。
		auto& animEventArray = m_animationClip->GetAnimationEvent();
		for (auto i = 0; i < m_animationClip->GetNumAnimationEvent(); i++) {
			animEventArray[i].SetInvokedFlag(false);
		}
	}
	void AnimationPlayController::CalcBoneMatrixInRootBoneSpace(Bone& bone, Matrix parentMatrix)
	{
		// ワールド行列を計算する。
		auto& mBoneInRootSpace = m_boneMatrix[bone.GetNo()];
		Matrix localMatrix = m_boneMatrix[bone.GetNo()];
		// 親の行列とローカル行列を乗算して、ワールド行列を計算する。
		mBoneInRootSpace = localMatrix * parentMatrix;

		// 子のワールド行列を計算する。
		for (auto& childBone : bone.GetChildren()) {
			CalcBoneMatrixInRootBoneSpace(*childBone, mBoneInRootSpace);
		}
	}
	void AnimationPlayController::SamplingBoneMatrixFromAnimationClip()
	{
		const auto& keyFramePtrListArray = m_animationClip->GetKeyFramePtrListArray();
		const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();

		// m_currentKeyFrameNo は time >= m_time となる「次」フレーム。
		// 補間は「前フレーム(prev)→現フレーム(current)」の間で行う。
		float interpT = 0.0f;
		const int prevFrameNo = m_currentKeyFrameNo - 1;
		if (prevFrameNo >= 0 && m_currentKeyFrameNo < (int)topBoneKeyFrameList.size()) {
			const float prevTime = topBoneKeyFrameList.at(prevFrameNo)->time;
			const float curTime = topBoneKeyFrameList.at(m_currentKeyFrameNo)->time;
			const float duration = curTime - prevTime;
			if (duration > 0.0f) {
				interpT = max(0.0f, min(1.0f, (m_time - prevTime) / duration));
			}
		}

		for (const auto& keyFrameList : keyFramePtrListArray) {
			if (keyFrameList.size() == 0) {
				continue;
			}

			const int safeFrameNo = min(m_currentKeyFrameNo, (int)keyFrameList.size() - 1);
			KeyFrame* cur = keyFrameList.at(safeFrameNo);

			if (interpT <= 0.0f || prevFrameNo < 0) {
				m_boneMatrix[cur->boneIndex] = cur->transform;
			}
			else {
				KeyFrame* prev = keyFrameList.at(prevFrameNo);

				DirectX::XMVECTOR s0, r0, t0;
				DirectX::XMVECTOR s1, r1, t1;
				DirectX::XMMatrixDecompose(&s0, &r0, &t0, prev->transform);
				DirectX::XMMatrixDecompose(&s1, &r1, &t1, cur->transform);

				const DirectX::XMVECTOR sOut = DirectX::XMVectorLerp(s0, s1, interpT);
				const DirectX::XMVECTOR rOut = DirectX::XMQuaternionSlerp(r0, r1, interpT);
				const DirectX::XMVECTOR tOut = DirectX::XMVectorLerp(t0, t1, interpT);

				const DirectX::XMMATRIX result =
					DirectX::XMMatrixScalingFromVector(sOut)
					* DirectX::XMMatrixRotationQuaternion(rOut)
					* DirectX::XMMatrixTranslationFromVector(tOut);

				DirectX::XMStoreFloat4x4(&m_boneMatrix[cur->boneIndex].mat, result);
			}
		}
	}
	void AnimationPlayController::CalcBoneMatrixInRootBoneSpace()
	{
		int numBone = m_skeleton->GetNumBones();
		for (int boneNo = 0; boneNo < numBone; boneNo++) {
			// ルートの親なしボーンを探す。
			auto bone = m_skeleton->GetBone(boneNo);
			if (bone->GetParentBoneNo() != -1) {
				continue;
			}
			CalcBoneMatrixInRootBoneSpace(*bone, g_matIdentity);
		}
	}
	void AnimationPlayController::SamplingDeltaValueFootstepBone()
	{
		if (m_currentKeyFrameNoLastFrame == m_currentKeyFrameNo) {
			// キーフレームが進んでいない。
			return;
		}
		if (m_footstepBoneNo == -1) {
			return;
		}
		int numBone = m_skeleton->GetNumBones();

		for (int boneNo = 0; boneNo < numBone; boneNo++) {
			auto bone = m_skeleton->GetBone(boneNo);
			if (m_footstepBoneNo == bone->GetNo()) {
				auto mat = m_boneMatrix[bone->GetNo()];
				Vector3 footstepBonePos;
				footstepBonePos.x = mat.m[3][0];
				footstepBonePos.y = mat.m[3][1];
				footstepBonePos.z = mat.m[3][2];
				// 今のフレームでのfootstepの移動量を計算する。
				m_footstepDeltaValue = footstepBonePos - m_footstepPos;
				// 今のフレームでのfootstepの座標を更新する。
				m_footstepPos = footstepBonePos;
				break;
			}
		}
	}
	void AnimationPlayController::SubtractFootstepbonePosFromAllBone()
	{
		if (m_footstepBoneNo == -1) {
			return;
		}
		int numBone = m_skeleton->GetNumBones();

		for (int boneNo = 0; boneNo < numBone; boneNo++) {
			auto bone = m_skeleton->GetBone(boneNo);
			m_boneMatrix[bone->GetNo()].m[3][0] -= m_footstepPos.x;
			m_boneMatrix[bone->GetNo()].m[3][1] -= m_footstepPos.y;
			m_boneMatrix[bone->GetNo()].m[3][2] -= m_footstepPos.z;
		}
	}
	void AnimationPlayController::ProgressKeyframeNo(float deltaTime)
	{
		// 1フレーム前のキーフレーム番号を記録しておく。
		m_currentKeyFrameNoLastFrame = m_currentKeyFrameNo;

		const auto& topBoneKeyFrameList = m_animationClip->GetTopBoneKeyFrameList();

		// ブレンド進行は実時間で行う（アニメ速度に関係なく常に一定時間でブレンド完了）
		m_interpolateTime = min(1.0f, m_interpolateTime + g_gameTime->GetFrameDeltaTime());
		while (true) {
			if (m_currentKeyFrameNo >= (int)topBoneKeyFrameList.size()) {
				// 末尾まで進行した。
				if (m_animationClip->IsLoop()) {
					// ループ。
					StartLoop();
				}
				else {
					// ワンショット再生。
					m_currentKeyFrameNo--;
					m_isPlaying = false;	// 再生終了。
				}
				break;
			}
			if (topBoneKeyFrameList.at(m_currentKeyFrameNo)->time >= m_time) {
				// 終了。
				break;
			}
			// 次へ。
			m_currentKeyFrameNo++;
		}
	}
	void AnimationPlayController::Update(
		float deltaTime,
		Animation* animation,
		bool isInvokeAnimationEvent)
	{
		if (m_animationClip == nullptr) {
			return;
		}

		m_time += deltaTime;

		if (isInvokeAnimationEvent) {
			// アニメーションイベントの発火。
			InvokeAnimationEvent(animation);
		}
		// キーフレーム番号を進める。
		ProgressKeyframeNo(deltaTime);

		// ボーン行列をアニメーションクリップからサンプリングしていく。
		SamplingBoneMatrixFromAnimationClip();

		// 親のローカル座標系になっているボーン行列をルートのボーン空間（モデル空間）に変換していく。
		CalcBoneMatrixInRootBoneSpace();

		// footstepボーンの移動量を取得する。
		SamplingDeltaValueFootstepBone();

		// footstepボーンの移動量を全体の座標から減算する。
		SubtractFootstepbonePosFromAllBone();

		// アニメーション再生中であることをスケルトンに通知。
		m_skeleton->SetMarkPlayAnimation();
	}
}