#include "k2EngineLowPreCompile.h"
#include "k2EngineLow.h"
#include "SceneLight.h"

namespace nsK2EngineLow {
	SceneLight::SceneLight()
	{

	}

	void SceneLight::Init()
	{

		InitDirectionLight();
		//InitPointLight();
		//InitSpotLight();
		InitAmbientLight();
		InitHemisphereLight();

		//影描画用のライトカメラを作成する
		Camera lightCamera;
		//カメラの位置を設定、これはライトの位置
		lightCamera.SetPosition(0, 600, 300);
		//カメラの注視点を設定、これはライトが照らしている場所
		lightCamera.SetTarget(0, 0, 0);
		//上方向を設定、今回はライトが真下に向いているので、X方向を上にしている
		lightCamera.SetUp(0, 1, 0);
		//画角を狭めにしておく
		//lightCamera.SetViewAngle(Math::DegToRad(80.0f));
		//ライトビュープロジェクション行列を計算している
		lightCamera.Update();

		m_light.mLVP = lightCamera.GetViewProjectionMatrix();



		//m_light.directionalLight[0].castShadow = true;


		////すべてのポイントライトを未使用にする。
		//for (auto& pt : m_light.pointLights) {
		//	pt.UnUse();
		//	pt.SetAffectPowParam(1.0f);
		//}
		//m_light.numPointLight = 0;

		//
	}

	void SceneLight::Update()
	{
		m_light.mLVP = g_renderingEngine->GetLigCameraViewProjection();

		//DirRot();
	}

	void SceneLight::InitDirectionLight()
	{
		//太陽光
		//ディレクションライトのデータを作成する
		//ライトは斜め上から当たっている
		m_light.dirDirection.x = 1.0f;
		m_light.dirDirection.y = -1.0f;
		m_light.dirDirection.z = -1.0f;

		//正規化する
		m_light.dirDirection.Normalize();

		//ライトのカラーは灰色(0.6f),bloom(5.8f)
		//ライトを強めに設定
		m_light.color.x = 0.6f;
		m_light.color.y = 0.6f;
		m_light.color.z = 0.6f;

		//視点
		m_light.eyePos = g_camera3D->GetPosition();
		//スペキュラの絞り
		m_light.specPow = 5.0f;
	}

	void SceneLight::InitPointLight()
	{
		//ポイントライト
		m_light.ptPosition.x = 0.0f;
		m_light.ptPosition.y = 50.0f;
		m_light.ptPosition.z = 50.0f;

		m_light.ptColor.x = 15.0f; //もとは１５
		m_light.ptColor.y = 0.0f;
		m_light.ptColor.z = 0.0f;


		m_light.ptRange = 100.0f;
	}

	void SceneLight::InitSpotLight()
	{
		//スポットライト
		//初期座標
		m_light.spPosition.x = 0.0f;
		m_light.spPosition.y = 50.0f;
		m_light.spPosition.z = 0.0f;
		//ライトのカラー
		m_light.spColor.x = 10.0f;
		m_light.spColor.y = 10.0f;
		m_light.spColor.z = 10.0f;
		//初期方向
		m_light.spDirection.x = 1.0f;
		m_light.spDirection.y = -1.0f;
		m_light.spDirection.z = 1.0f;
		//方向データを正規化
		m_light.spDirection.Normalize();
		//射出範囲
		m_light.spRange = 300.0f;
		//射出角度
		m_light.spAngle = Math::DegToRad(25.0f);

	}

	void SceneLight::InitAmbientLight()
	{
		//環境光
		m_light.ambientLight.x = 0.4f;
		m_light.ambientLight.y = 0.4f;
		m_light.ambientLight.z = 0.4f;
	}

	void SceneLight::InitHemisphereLight()
	{
		//地面色
		m_light.groundColor.x = 0.7f;
		m_light.groundColor.y = 0.5f;
		m_light.groundColor.z = 0.3f;

		//天球色
		m_light.skyColor.x = 0.15f;
		m_light.skyColor.y = 0.7f;
		m_light.skyColor.z = 0.95f;

		//地面の法線を設定
		m_light.groundNormal.x = 0.0f;
		m_light.groundNormal.y = 1.0f;
		m_light.groundNormal.z = 0.0f;
	}

	void SceneLight::DirRot()
	{
		//ディレクションライトを回す
		Quaternion qRotY;
		qRotY.SetRotation(g_vec3AxisY, g_pad[0]->GetLStickXF() * 0.02f);
		qRotY.Apply(m_light.dirDirection);
	}

	/*void SceneLight::Update()
	{
		m_light.numPointLight = 0;
		for (int i = 0; i < MAX_POINT_LIGHT; i++)
		{
			if (m_light.pointLights[i].GetUse())
			{
				m_light.numPointLight++;
			}
		}
	}*/

	/*SPointLight* SceneLight::NewPointLight()
	{
		if (m_light.numPointLight >= MAX_POINT_LIGHT)
		{
			return nullptr;
		}

		for (int i = 0; i < MAX_POINT_LIGHT; i++)
		{
			if (m_light.pointLights[i].GetUse() != true)
			{
				m_light.pointLights[i].Use();
				return &m_light.pointLights[i];
			}
		}
	}*/

	//void SceneLight::DeletePointLight(SPointLight* m_pointLight)
	//{
	//	//ポインタにアドレスが入っていなかったら何もせずに返す
	//	if (m_pointLight == nullptr)
	//	{
	//		return;
	//	}
	//	//万が一、シーンライトが管理していないポイントライトに対して削除処理を実行すると
	//	//管理がバクる（特に使用中のライトの数）ので本当に削除処理をしていいか確認する。
	//	//
	//	//送られてきたポインタに入っているアドレスと、配列の各要素のアドレスを照らし合わせて、
	//	//アドレスが同じものを見つけたら、削除処理を実行する
	//	for (int i = 0; i < MAX_POINT_LIGHT; i++)
	//	{
	//		if (m_pointLight == &m_light.pointLights[i])
	//		{
	//			m_pointLight->UnUse();
	//			m_pointLight->SetColor(0.0f, 0.0f, 0.0f);
	//			m_pointLight->SetPosition(0.0f, 0.0f, 0.0f);
	//			m_pointLight->SetRange(0.0f);
	//			m_pointLight->SetAffectPowParam(1.0f);
	//		}
	//	}
	//}


}