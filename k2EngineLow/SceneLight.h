#pragma once

namespace nsK2EngineLow {
	
	static const int MAX_DIRECTIONAL_LIGHT = 4;
	static const int MAX_POINT_LIGHT = 32;
	static const int MAX_SPOT_LIGHT = 32;
	////ディレクションライトの構造体
	//struct DirectionLight
	//{
	//	Vector3 direction;
	//	float pad0;
	//	//int castShadow = true;
	//	Vector3 color;
	//	float pad1;
	//};

	////ポイントライトの構造体
	//struct SPointLight
	//{
	////private: 
	//	Vector3 position;		//座標
	//	int isUse = false;					//使用中フラグ
	//	Vector3 color;			//ライトのカラー
	//	float pad2;
	//	float range = 0.0f;	//減衰パラメータ。Xに影響範囲、Yには影響率に累乗するパラメータ
	//	float pad3;
	//};

	////スポットライトの構造体
	//struct SSpotLight
	//{
	////private:
	//	Vector3 position = g_vec3Zero;	//座標
	//	int isUse = false;				//使用中フラグ
	//	Vector3 color = g_vec3One;		//ライトのカラー
	//	float range;					//影響範囲
	//	Vector3 direction = g_vec3Down; //射出方向
	//	float angle;					//射出角度
	//	Vector3 pow = { 1.0f,1.0f,0.0f };//影響率に累乗するパラメータ
	//									 //xが距離による影響率に乗算するパラメータ
	//									 //yが角度による影響率に乗算するパラメータ
	//	float pad4;

	//
	//};

	//ライトの構造体
	struct Light
	{
		//ディレクションライト用のメンバ変数
		Vector3 dirDirection; //ライトの方向
		float pad1;
		Vector3 color;		  //ライトのカラー
		float pad2;

		Vector3 lightPos;
		float padlight;
		
		//DirectionLight directionalLight[MAX_DIRECTIONAL_LIGHT];	//ディレクションライトの配列
		//SPointLight pointLights[MAX_POINT_LIGHT];				//ポイントライトの配列[
		//DirectionLight directionalLight;
		//PointLight pointLight;
		
		//ポイントライト用のメンバ変数
		Vector3 ptPosition;
		float pad3;
		Vector3 ptColor;
		float ptRange;

		//スポットライト用のメンバ変数
		Vector3 spPosition;	//位置
		float pad4;
		Vector3 spColor;	//ライトのカラー
		float spRange;		//影響範囲
		Vector3 spDirection;//射出方向
		float spAngle;		//射出角度


		Vector3 eyePos;		//視点の位置
		float specPow;		//スペキュラの絞り
		Vector3 ambientLight;	//環境光
		float pad5;

		//地面色と天球色、地面の法線を追加
		Vector3 groundColor;	//地面色
		float pad7;
		Vector3 skyColor;		//天球色
		float pad8;
		Vector3 groundNormal;	//地面の法線
		float pad9;

		Matrix mLVP;
	};

	//シーンライトクラス
	class SceneLight : public Noncopyable
	{
	public:
		SceneLight();

		//初期化
		void Init();
		//ディレクションライト
		void InitDirectionLight();
		//ポイントライト
		void InitPointLight();
		//スポットライト
		void InitSpotLight();
		//環境光
		void InitAmbientLight();
		//半球ライト
		void InitHemisphereLight();
		//ディレクションライトを回す
		void DirRot();
		//シーンライトを取得
		Light& GetSceneLight()
		{
			return m_light;
		}
		

		//更新処理
		void Update();

		

		Light& GetLightData()
		{
			return m_light;
		}

	private:
		Light m_light;	//シーンライト
		Camera m_lightCamera;
		//Shadow m_shadow;
		//ModelInitData bgModelInitData;	//影を受ける背景を初期化
		//DirectionLight m_dirLight; //ディレクションライト
		//SPointLight m_pointLight; //ポイントライト
	};
}


