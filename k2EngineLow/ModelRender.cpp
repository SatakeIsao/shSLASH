#include "k2EngineLowPreCompile.h"
#include "ModelRender.h"

namespace nsK2EngineLow {

    ModelRender::ModelRender() :
        m_animationClips(nullptr),
        m_numAnimationClips(0),
        m_animationSpeed(1.0f),
        m_isShadowCaster(false) // コンストラクタで初期値を明示
    {}

    ModelRender::~ModelRender()
    {}

    void ModelRender::Init(
        const char* tkmFilePath,
        AnimationClip* animationClips,
        int numAnimationClips,
        EnModelUpAxis enModelUpAxis,
        bool isShadowCaster,
        bool isShadowReceiver,
        const char* fxFilePath)
    {
        m_isShadowCaster = isShadowCaster; // メンバ変数への保存漏れを修正

        // 1. スケルトンとアニメーションの初期化
        InitSkeleton(tkmFilePath);
        InitAnimation(animationClips, numAnimationClips, enModelUpAxis);

        // 2. モデル初期化データのセットアップ
        ModelInitData initData;
        initData.m_tkmFilePath = tkmFilePath;
        initData.m_fxFilePath = (fxFilePath != nullptr) ? fxFilePath : "Assets/Shader/model.fx";

        // ライト情報のセットアップ
        // NOTE: Init内でカメラをNewするのは不自然なので、
        // 本来はシーン全体のライト管理クラスから行列をもらうのが理想的です。
        initData.m_expandConstantBuffer = &g_sceneLight->GetLightData();
        initData.m_expandConstantBufferSize = sizeof(g_sceneLight->GetLightData());

        // エントリポイントの決定
        initData.m_vsEntryPointFunc = "VSMain";
        initData.m_vsSkinEntryPointFunc = "VSSkinMain";

        // シャドウレシーバー（影を受ける側）の設定
        if (isShadowReceiver)
        {
            initData.m_psEntryPointFunc = "PSShadowReceverMain";
            // シャドウマップをテクスチャレジスタ0にセット
            initData.m_expandShaderResoruceView[0] =
                &(g_renderingEngine->GetShadowMap().GetRenderTargetTexture());
        }
        else
        {
            initData.m_psEntryPointFunc = "PSNormalMain";
        }

        // スキニングの有無によるシェーダー切り替え
        if (animationClips != nullptr) {
            initData.m_skeleton = &m_skeleton;
        }
        else {
            // アニメーションがない場合はスキニングなしのVSを使う
            initData.m_vsSkinEntryPointFunc = "VSMain";
        }

        m_model.Init(initData);

        // 3. シャドウキャスター（影を落とす側）なら影専用モデルも初期化
        if (m_isShadowCaster)
        {
            InitShadowModel(tkmFilePath, enModelUpAxis);
        }
    }

    void ModelRender::InitShadowModel(const char* tkmFilePath, EnModelUpAxis modelUpAxis)
    {
        ModelInitData shadowInitData;
        shadowInitData.m_tkmFilePath = tkmFilePath;
        shadowInitData.m_fxFilePath = "Assets/Shader/drawShadowMap.fx";
        shadowInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;

        if (m_animationClips != nullptr)
        {
            shadowInitData.m_skeleton = &m_skeleton;
        }

        shadowInitData.m_vsEntryPointFunc = "VSMain";
        shadowInitData.m_vsSkinEntryPointFunc = "VSSkinMain";

        m_shadowModel.Init(shadowInitData);
    }

    void ModelRender::InitSkeleton(const char* tkmFilePath)
    {
        std::string skeletonFilePath = tkmFilePath;
        size_t pos = skeletonFilePath.find(".tkm");
        if (pos != std::string::npos) {
            skeletonFilePath.replace(pos, 4, ".tks");
            m_skeleton.Init(skeletonFilePath.c_str());
        }
    }

    void ModelRender::InitAnimation(AnimationClip* animationClips, int numAnimationClips, EnModelUpAxis enModelUpAxis)
    {
        m_animationClips = animationClips;
        m_numAnimationClips = numAnimationClips;

        if (m_animationClips != nullptr && m_numAnimationClips > 0)
        {
            m_animation.Init(m_skeleton, m_animationClips, m_numAnimationClips);
        }
    }

    void ModelRender::Update()
    {
        // ワールド行列の更新
        m_model.UpdateWorldMatrix(m_position, m_rotation, m_scale);

        if (m_isShadowCaster) {
            m_shadowModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
        }

        // スケルトンとアニメーションの更新
        m_skeleton.Update(m_model.GetWorldMatrix());

        if (m_animationClips != nullptr) {
            m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
        }
    }

    void ModelRender::Draw(RenderContext& rc)
    {
        // レンダリングエンジンに自身を登録
        g_renderingEngine->AddRenderObject(this);
    }

    void ModelRender::OnRenderShadowMap(RenderContext& rc, const Matrix& lvpMatrix)
    {
        if (!m_isShadowCaster) return;

        m_shadowModel.Draw(rc, g_matIdentity, lvpMatrix, 1);
    }

    void ModelRender::OnRenderModel(RenderContext& rc)
    {
        // 非表示なら描画しない
        if (!m_isVisible_) return;
        m_model.Draw(rc);
    }
}