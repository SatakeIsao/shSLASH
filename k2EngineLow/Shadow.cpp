#include "k2EngineLowPreCompile.h"
#include "Shadow.h"


void nsK2EngineLow::Shadow::Init()
{
	InitRenderTarget();
	InitLightCamera();

	// シャドウマップをぼかすためのガウシアンブラーを初期化
	m_shadowBlur.Init(&m_shadowMap.GetRenderTargetTexture());

	SpriteInitData spriteInitData;
	spriteInitData.m_textures[0] = &m_shadowMap.GetRenderTargetTexture();
	spriteInitData.m_fxFilePath = "Assets/Shader/sprite.fx";
	spriteInitData.m_width = 256;
	spriteInitData.m_height = 256;
	m_sprite.Init(spriteInitData);
}

void nsK2EngineLow::Shadow::Render(RenderContext& rc, std::vector<IRenderer*>& renderObjects)
{
	Matrix viewMatrix;
	Matrix projectionMatrix;

	// ライト方向を真上寄り
	// ブロック側面への投影角度が深くなりセルフシャドウ減少
	const Vector3 lightDir = Vector3(0.5f, -2.0f, -0.5f);
	// ライトの距離
	const float lightDistance = 3000.0f;
	// 並行投影の範囲
	const float ORTHO_SIZE = 4000.0f;
	// シャドウマップのサイズ
	const float SHADOW_MAP_SIZE = 4096.0f;

	// テクセル1つ分のワールド空間サイズ
	const float texelSize = ORTHO_SIZE / SHADOW_MAP_SIZE;

	// ターゲットのXZのみ追従、Yは固定
	Vector3 cameraTarget = g_camera3D->GetTarget();
	Vector3 lightTarget = Vector3(cameraTarget.x, 0.0f, cameraTarget.z);

	// ライトターゲットをテクセルサイズ単位にスナップ
	//lightTarget.x = floorf(lightTarget.x / texelSize + 0.5f) * texelSize;
	//lightTarget.z = floorf(lightTarget.z / texelSize + 0.5f) * texelSize;

	// lightDir を正規化して逆方向にカメラ配置
	Vector3 lightDirNorm = lightDir;
	lightDirNorm.Normalize();
	Vector3 lightCameraPos = lightTarget + Vector3(-lightDirNorm.x, -lightDirNorm.y, -lightDirNorm.z) * lightDistance;

	viewMatrix.MakeLookAt(
		lightCameraPos,
		lightTarget,
		Vector3::AxisY
	);

	// Far を固定値にして深度バッファの精度を確保
	const float SHADOW_NEAR = 100.0f;
	const float SHADOW_FAR = 6000.0f;

	projectionMatrix.MakeOrthoProjectionMatrix(
		ORTHO_SIZE,
		ORTHO_SIZE,
		SHADOW_NEAR,
		SHADOW_FAR
	);


	////ビュー行列の算出
	//viewMatrix.MakeLookAt(
	//	m_lightCamera.GetPosition(), 
	//	m_lightCamera.GetTarget(), 
	//	m_lightCamera.GetUp());

	////平行投影行列の計算
	//projectionMatrix.MakeOrthoProjectionMatrix(
	//	m_lightCamera.GetWidth(),
	//	m_lightCamera.GetHeight(),
	//	m_lightCamera.GetNear(),
	//	m_lightCamera.GetFar());

	//ビュープロジェクションの行列
	m_viewProjectionMatrix = viewMatrix * projectionMatrix;


	/*m_lightCamera.SetTarget(g_camera3D->GetTarget());
	m_lightCamera.SetPosition(g_camera3D->GetTarget() + Vector3{ 0.0f, 600.0f, 300.0f });
	m_lightCamera.Update();*/

	// シャドウマップに描画
	rc.WaitUntilToPossibleSetRenderTarget(m_shadowMap);
	rc.SetRenderTargetAndViewport(m_shadowMap);
	rc.ClearRenderTargetView(m_shadowMap);
	for (auto& renderer : renderObjects)
	{
		if (renderer == nullptr) { continue; }
		renderer->OnRenderShadowMap(rc, m_viewProjectionMatrix);
	}
	rc.WaitUntilFinishDrawingToRenderTarget(m_shadowMap);

	// シャドウマップをぼかすためのガウシアンブラーを実行
	m_shadowBlur.ExecuteOnGPU(rc, 5.0f);
}

void nsK2EngineLow::Shadow::InitRenderTarget()
{
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_shadowMap.Create(
		4096,
		4096,
		1,
		1,
		//DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_D32_FLOAT,
		clearColor
	);
}

void nsK2EngineLow::Shadow::InitLightCamera()
{
	//m_lightCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
	//m_lightCamera.SetPosition(0, 600, 300);
	//m_lightCamera.SetTarget(0, 0, 0);
	//m_lightCamera.SetUp(1, 0, 0);
	////m_lightCamera.SetViewAngle(Math::DegToRad(20.0f));
	//m_lightCamera.Update();
}

void nsK2EngineLow::Shadow::SpriteShadowDraw(RenderContext& rc)
{
	//sprite.Update({ FRAME_BUFFER_W / -2.0f, FRAME_BUFFER_H / 2.0f,  0.0f }, g_quatIdentity, g_vec3One, { 0.0f, 1.0f });
	//sprite.Draw(rc);
}