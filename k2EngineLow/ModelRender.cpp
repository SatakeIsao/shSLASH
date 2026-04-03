#include "k2EngineLowPreCompile.h"
#include "ModelRender.h"

namespace nsK2EngineLow {

	ModelRender::ModelRender()
	{
		//コンストラクタの処理
	}

	ModelRender::~ModelRender()
	{
		// デストラクタの処理
	}

	void ModelRender::Init(
		const char* tkmFilePath,
		AnimationClip* animationClips,
		int numAnimationClips,
		EnModelUpAxis enModelUpAcxis,
		bool isShadowCaster,
		bool isShadowReceiver)
	{
		//スケルトンを初期化
		InitSkeleton(tkmFilePath);
		//アニメーションを初期化。
		InitAnimation(animationClips, numAnimationClips, enModelUpAcxis);
		//モデルの初期化
		ModelInitData initData;
		Camera lightCamera;

		initData.m_tkmFilePath = tkmFilePath;
		//シャドウレシーバー（影が落とされるモデル）用のシェーダーを指定する
		initData.m_fxFilePath = "Assets/Shader/model.fx";
		//initData.m_fxFilePath = "Assets/shader/shadowReceiverModel.fx";

		lightCamera.SetAspectOnrFlag(true);
		lightCamera.SetViewAngle(Math::DegToRad(80.0f));
		lightCamera.SetPosition(-6000.0f, 500.0f, -500.0f);
		lightCamera.SetTarget(0, 0, 0);
		lightCamera.Update();

		m_light.m_mt = lightCamera.GetViewProjectionMatrix();
		initData.m_expandConstantBuffer = &g_sceneLight->GetLightData();

		initData.m_expandConstantBufferSize = sizeof(g_sceneLight->GetLightData());
	
		//ノンスキンメッシュ用の頂点シェーダーのエントリーポイントを指定する
		initData.m_vsEntryPointFunc = "VSMain";
	
		//スキンメッシュ用の頂点シェーダーのエントリーポイントを指定。
		initData.m_vsSkinEntryPointFunc = "VSSkinMain";
		
		if (isShadowCaster == true)
		{
			InitShadowModel(tkmFilePath, enModelUpAcxis);
		}

		if (isShadowReceiver == true)
		{
			initData.m_psEntryPointFunc = "PSShadowReceverMain";

			initData.m_expandShaderResoruceView[0] =
				&(g_renderingEngine->GetShadowMap().GetRenderTargetTexture());
		}
		else
		{
			initData.m_psEntryPointFunc = "PSNormalMain";
		}
		if (animationClips != nullptr) {
			initData.m_skeleton = &m_skeleton;
		}
		//スケルトンが無い場合、スケルトンあり用の頂点シェーダーを使わないように変更する。
		if (initData.m_skeleton == nullptr)
		{
			initData.m_vsSkinEntryPointFunc = "VSMain";
		}

		m_model.Init(initData);
	}

	void ModelRender::InitShadowModel(const char* tkmFilePath, EnModelUpAxis modelUpAxis)
	{
		ModelInitData shadowInitData;
		shadowInitData.m_tkmFilePath = tkmFilePath;
		//shadowInitData.m_modelUpAxis = modelUpAxis;
		shadowInitData.m_fxFilePath = "Assets/Shader/drawShadowMap.fx";
		shadowInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;

		//アニメーションがあるならスケルトンを指定する
		if (m_animationClips != nullptr)
		{
			shadowInitData.m_skeleton = &m_skeleton;
		}
	
		//ノンスキンメッシュ用の頂点シェーダーのエントリーポイントを指定する
		shadowInitData.m_vsEntryPointFunc = "VSMain";
		
		//スキンメッシュ用の頂点シェーダーのエントリーポイントを指定。
		shadowInitData.m_vsSkinEntryPointFunc = "VSSkinMain";
		m_shadowModel.Init(shadowInitData);
	}

	void ModelRender::InitSkeleton(const char* tkmFilePath)
	{
		//スケルトンのデータを読み込み。
		std::string skeletonFilePath = tkmFilePath;
		int pos = (int)skeletonFilePath.find(".tkm");
		skeletonFilePath.replace(pos, 4, ".tks");
		m_skeleton.Init(skeletonFilePath.c_str());
	}

	void ModelRender::InitAnimation(AnimationClip* animationClips, int numAnimationClips,EnModelUpAxis EnModelUpAxis)
	{
		//アニメーションの設定
		m_animationClips = animationClips;
		m_numAnimationClips = numAnimationClips;
		if (m_animationClips != nullptr)
		{
			m_animation.Init(
				m_skeleton,
				m_animationClips,
				numAnimationClips
			);
		}
	}

	/*void ModelRender::InitComputeAnimationVertexBuffer(const char* tkmFilePath, EnModelUpAxis enModelUpAxis)
	{
		StructuredBuffer* worldMatrxiArraySB = nullptr;
		if (m_isEnableInstancingDraw) {
			worldMatrxiArraySB = &m_worldMatrixArraySB;
		}

		m_computeAnimationVertexBuffer.Init(
			tkmFilePath,
			m_skeleton.GetNumBones(),
			m_skeleton.GetBoneMatricesTopAddress(),
			enModelUpAxis,
			m_maxInstance,
			worldMatrxiArraySB
		);
	}*/

	void ModelRender::Update()
	{
		//モデルのワールド行列更新
		m_model.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_shadowModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);

		m_skeleton.Update(m_model.GetWorldMatrix());
		//アニメーションを進める
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
	}

	//void ModelRender::UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale)
	//{
	//	K2_ASSERT(instanceNo < m_maxInstance, "インスタンス番号が不正です。");
	//	if (!m_isEnableInstancingDraw) {
	//		return;
	//	}
	//	Matrix worldMatrix;
	//	if (m_translucentModel.IsInited()) {
	//		// 半透明モデルはZPrepassモデルを初期化していないので、こちらを使う。
	//		worldMatrix = m_translucentModel.CalcWorldMatrix(pos, rot, scale);
	//	}
	//	else {
	//		worldMatrix = m_zprepassModel.CalcWorldMatrix(pos, rot, scale);
	//	}
	//	// インスタンス番号から行列のインデックスを取得する。
	//	int matrixArrayIndex = m_instanceNoToWorldMatrixArrayIndexTable[instanceNo];
	//	// インスタンシング描画を行う。
	//	m_worldMatrixArray[matrixArrayIndex] = worldMatrix;
	//	if (m_numInstance == 0) {
	//		//インスタンス数が0の場合のみアニメーション関係の更新を行う。
	//		// スケルトンを更新。
	//		// 各インスタンスのワールド空間への変換は、
	//		// インスタンスごとに行う必要があるので、頂点シェーダーで行う。
	//		// なので、単位行列を渡して、モデル空間でボーン行列を構築する。
	//		m_skeleton.Update(g_matIdentity);
	//		//アニメーションを進める。
	//		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
	//	}
	//	m_numInstance++;
	//}

	void ModelRender::Draw(RenderContext& rc)
	{
		g_renderingEngine->AddRenderObject(this);
	}

	void ModelRender::OnRenderShadowMap(RenderContext& rc, const Matrix& lvpMatrix)
	{
		m_shadowModel.Draw(
			rc,
			g_matIdentity,
			lvpMatrix,
			1
		);
	}

	void ModelRender::OnRenderModel(RenderContext& rc)
	{
		m_model.Draw(rc);
		
	}


	

}