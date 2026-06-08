#pragma once

namespace nsK2EngineLow {

	// 各種ライトの最大配置数（定数バッファのサイズ制限などに使用）
	static const int MAX_DIRECTIONAL_LIGHT = 4; // ディレクショナルライトの最大数
	static const int MAX_POINT_LIGHT = 32;       // ポイントライトの最大数
	static const int MAX_SPOT_LIGHT = 32;        // スポットライトの最大数

	// TBDR（タイルベースディファードレンダリング）用定数
	static const int MAX_TBDR_POINT_LIGHT = 1000; // TBDRポイントライトの最大数
	static const int TBDR_TILE_WIDTH  = 16;        // タイル幅（ピクセル）
	static const int TBDR_TILE_HEIGHT = 16;        // タイル高さ（ピクセル）

	// TBDRポイントライト構造体（StructuredBuffer 経由でシェーダーへ渡す）
	struct alignas(16) TBDRPointLight
	{
		Vector3 position;   // ワールド座標
		float   pad0 = 0.f;
		Vector3 color;      // カラー・輝度
		float   range = 0.f; // 影響範囲
	};

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

		// バトルシーン用のライト設定（減光）
		void SetBattleLighting();
		// リザルトシーン用のライト設定（明るめ）
		void SetResultLighting();

		// 指定座標にスポーンフラッシュライトを発動する（フェードアウトあり）
		// intensityMultiplier: ピーク輝度の倍率（デフォルト1.0f = 敵スポーン標準）
		void TriggerSpawnLight(const Vector3& position, float intensityMultiplier = 1.0f);

		// ---- TBDR ポイントライト管理 ----
		TBDRPointLight& GetTBDRPointLight(int index)
		{
			return m_tbdrPointLights[index];
		}
		const TBDRPointLight* GetTBDRPointLightsData() const
		{
			return m_tbdrPointLights;
		}
		int GetNumTBDRPointLights() const
		{
			return m_numTBDRPointLights;
		}
		void SetNumTBDRPointLights(int num)
		{
			m_numTBDRPointLights = (num < MAX_TBDR_POINT_LIGHT) ? num : MAX_TBDR_POINT_LIGHT;
		}

	private:
		Light m_light;			// シーンライトの構造体データ
		Camera m_lightCamera;	// シャドウマップ描画時などに使用する光源視点のカメラ

		// TBDRポイントライトデータ（StructuredBuffer 用。SceneLight::Light とは独立）
		TBDRPointLight m_tbdrPointLights[MAX_TBDR_POINT_LIGHT] = {};
		int m_numTBDRPointLights = 0;

		// スポーンフラッシュライト（ポイントライト + スポットライト）
		float   spawnLightTimer_       = 0.0f;
		float   spawnLightIntensity_   = 1.0f;
		Vector3 spawnLightPosition_    = Vector3::Zero;

		static constexpr float SPAWN_LIGHT_DURATION = 3.0f;

		// ポイントライト（近距離グロー）
		static constexpr float SPAWN_LIGHT_RANGE    = 150.0f;
		static const     Vector3 SPAWN_LIGHT_PEAK_COLOR;   // warm orange-yellow

		// スポットライト（真上から円錐状に照らす）
		static constexpr float SPAWN_SPOT_HEIGHT    = 300.0f;   // スポーン位置からの基準高さ
		static constexpr float SPAWN_SPOT_MAX_MULT  = 1.95f;   // 最大高さ倍率（1.5 × 1.3）
		static constexpr float SPAWN_SPOT_RANGE     = 5000.0f; // 距離減衰を抑えるため大きめ
		static constexpr float SPAWN_SPOT_ANGLE     = 0.1309f; // 7.5度（旧15度の半分）
		static const     Vector3 SPAWN_SPOT_PEAK_COLOR;        // bright warm white
		//Shadow m_shadow; // シャドウマップ生成・管理用オブジェクト
		//ModelInitData bgModelInitData;	// 影を受ける背景モデルの初期化データ
		//DirectionLight m_dirLight; // ディレクショナルライトの個別データ
		//SPointLight m_pointLight; // ポイントライトの個別データ
	};
}