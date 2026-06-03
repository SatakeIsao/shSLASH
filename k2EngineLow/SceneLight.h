#pragma once

namespace nsK2EngineLow {

	// 各種ライトの最大配置数（定数バッファのサイズ制限などに使用）
	static const int MAX_DIRECTIONAL_LIGHT = 4; // ディレクショナルライトの最大数
	static const int MAX_POINT_LIGHT = 32;       // ポイントライトの最大数
	static const int MAX_SPOT_LIGHT = 32;        // スポットライトの最大数

	//// ディレクショナルライトの構造体
	//struct DirectionLight
	//{
	//	Vector3 direction;					// ライトの照射方向
	//	float pad0;							// アライメント調整用パディング
	//	//int castShadow = true;			// 影を生成するかどうかのフラグ
	//	Vector3 color;						// ライトの輝度・色
	//	float pad1;							// アライメント調整用パディング
	//};

	//// ポイントライトの構造体
	//struct SPointLight
	//{
	////private: 
	//	Vector3 position;					// 座標
	//	int isUse = false;					// 使用フラグ
	//	Vector3 color;						// ライトのカラー
	//	float pad2;							// アライメント調整用パディング
	//	float range = 0.0f;					// 減衰パラメータ。Xに影響範囲、Yに影響度を乗算するパラメータ
	//	float pad3;							// アライメント調整用パディング
	//};

	//// スポットライトの構造体
	//struct SSpotLight
	//{
	////private:
	//	Vector3 position = g_vec3Zero;		// 座標
	//	int isUse = false;					// 使用フラグ
	//	Vector3 color = g_vec3One;			// ライトのカラー
	//	float range;						// 影響範囲
	//	Vector3 direction = g_vec3Down;		// 照射方向
	//	float angle;						// 照射角度
	//	Vector3 pow = { 1.0f,1.0f,0.0f };	// 減衰を計算するパラメータ
	//										// x：距離による減衰に乗算するパラメータ
	//										// y：角度による減衰に乗算するパラメータ
	//	float pad4;							// アライメント調整用パディング
	//
	//};

	// ライトの構造体（シェーダーへ渡す定数バッファのデータ構造）
	struct Light
	{
		// ディレクショナルライト用のメンバー変数
		Vector3 dirDirection;				// ライトの方向
		float pad1;							// アライメント調整用パディング
		Vector3 color;						// ライトのカラー
		float pad2;							// アライメント調整用パディング

		Vector3 lightPos;					// ライトの位置（シャドウマップ生成時の光源カメラ座標など）
		float padlight;						// アライメント調整用パディング

		//DirectionLight directionalLight[MAX_DIRECTIONAL_LIGHT];	// ディレクショナルライトの配列
		//SPointLight pointLights[MAX_POINT_LIGHT];					// ポイントライトの配列
		//DirectionLight directionalLight;
		//PointLight pointLight;

		// ポイントライト用のメンバー変数
		Vector3 ptPosition;					// ポイントライトの位置座標
		float pad3;							// アライメント調整用パディング
		Vector3 ptColor;					// ポイントライトの色・輝度
		float ptRange;						// ポイントライトの影響範囲（半径）

		// スポットライト用のメンバー変数
		Vector3 spPosition;					// 位置
		float pad4;							// アライメント調整用パディング
		Vector3 spColor;					// ライトのカラー
		float spRange;						// 影響範囲
		Vector3 spDirection;				// 照射方向
		float spAngle;						// 照射角度（コーンのコサイン値など）


		Vector3 eyePos;						// 視点（カメラ）の位置
		float specPow;						// スペキュラー（鏡面反射）の輝度（または広がり・鋭さ）
		Vector3 ambientLight;				// 環境光（アンビエントライト）の色・強度
		float pad5;							// アライメント調整用パディング

		// 地面色と天球色、地面の法線を追加（半球ライト用）
		Vector3 groundColor;				// 地面色（下方向からの照り返しの色）
		float pad7;							// アライメント調整用パディング
		Vector3 skyColor;					// 天球色（上方向からの環境光の色）
		float pad8;							// アライメント調整用パディング
		Vector3 groundNormal;				// 地面の法線ベクトル（基準となる上方向）
		float pad9;							// アライメント調整用パディング

		Matrix mLVP;						// ライトビュープロジェクション行列（影の計算用等）

		int ditherEnabled = 1;				// ディザリングを有効にするか（1=有効, 0=無効）
		int ditherPad[3] = {};				// 16バイト境界に合わせるためのパディング
	};

	// シーンライトクラス（ゲーム内の光源を一括管理するクラス）
	class SceneLight : public Noncopyable
	{
	public:
		// コンストラクタ
		SceneLight();

		// 初期化（全体の初期化処理）
		void Init();
		// ディレクショナルライトの初期化
		void InitDirectionLight();
		// ポイントライトの初期化
		void InitPointLight();
		// スポットライトの初期化
		void InitSpotLight();
		// 環境光の初期化
		void InitAmbientLight();
		// 半球ライト（ヘミスフィアライト）の初期化
		void InitHemisphereLight();
		// ディレクショナルライト回転（時間経過などによるライト方向の更新）
		void DirRot();
		// シーンライトの生データを取得
		Light& GetSceneLight()
		{
			return m_light;
		}

		// 更新処理（フレームごとのライトデータの更新）
		void Update();

		// ライトデータを取得（GetSceneLightと同じ機能の汎用アクセサ）
		Light& GetLightData()
		{
			return m_light;
		}

		// ディザリングの有効/無効を設定する
		void SetDitherEnabled(bool enabled)
		{
			m_light.ditherEnabled = enabled ? 1 : 0;
		}

	private:
		Light m_light;			// シーンライトの構造体データ
		Camera m_lightCamera;	// シャドウマップ描画時などに使用する光源視点のカメラ
		//Shadow m_shadow; // シャドウマップ生成・管理用オブジェクト
		//ModelInitData bgModelInitData;	// 影を受ける背景モデルの初期化データ
		//DirectionLight m_dirLight; // ディレクショナルライトの個別データ
		//SPointLight m_pointLight; // ポイントライトの個別データ
	};
}