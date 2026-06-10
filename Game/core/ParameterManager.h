#pragma once

/*
* ParameterManager.h
* パラメーター管理
* 主に１つのキャラクターのステータスなど外部ファイルを読み込ませて保持したり、それを受け取ったりして使う。
* パラメーターの実体はファイルパスで行っているため、パラメーター取得、削除などで実行するごとにパスを要求するが、
* LoadParameter関数以外で読み込みが行われることはない。
* シングルトンクラス。
*/

#include <iostream>
#include <fstream>
#include "json/json.hpp"
#include "util/CRC32.h"


/** ホットリロード機能 */
#ifdef K2_DEBUG
#define APP_ENABLE_PARAM_HOT_RELOAD
#endif


namespace app
{
	namespace core
	{

#ifdef APP_ENABLE_PARAM_HOT_RELOAD

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}\
std::function<void(const nlohmann::json& j, name& p)> load;\
void Load(const nlohmann::json& j) override { load(j, *this); }

#else

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}

#endif //APP_ENABLE_PARAM_HOT_RELOAD


		/**
		 * パラメーターインターフェース
		 */
		struct IParameter
		{
			virtual ~IParameter() = default;

#ifdef APP_ENABLE_PARAM_HOT_RELOAD
			std::string m_path;								//パラメーターのファイルパス
			time_t m_lastWriteTime;							//最終更新時間
			virtual void Load(const nlohmann::json& j) {};	// 読み込み関数
#endif // APP_PARAM_HOT_RELOAD
		};


		/** バトル全体 */
		struct MasterBattleParameter : public IParameter
		{
			appParameter(MasterBattleParameter);

			float battleTime;			// 戦闘時間
		};


		/** バトルキャラクター */
		struct MasterBattleCharacterParameter : public IParameter
		{
			appParameter(MasterBattleCharacterParameter);

			float moveSpeed;					// 移動速度
			float jumpMoveSpeed;				// ジャンプ中の移動速度
			float jumpPower;					// ジャンプ力
			float radius;						// 半径
			float height;						// 高さ
			float hp;							// HP
			float attackPower;					// 攻撃力
			float chargeAttackMultiplierLevel1;	// 溜め攻撃Lv1の倍率
			float chargeAttackMultiplierLevel2;	// 溜め攻撃Lv2の倍率
			float chargeAttackMultiplier;		// 溜め攻撃Lv3（最大）の倍率
			float criticalRate;					// クリティカル発生確率 (0.0~1.0)
			float criticalMultiplier;			// クリティカル時のダメージ倍率
			float spawnLightColorR = 1.0f;		// スポーンライト色 R
			float spawnLightColorG = 0.9f;		// スポーンライト色 G
			float spawnLightColorB = 0.6f;		// スポーンライト色 B
			float spawnLightRange    = 350.f;	// スポーンライト範囲
			float spawnLightDuration = 3.0f;	// スポーンライト持続時間
		};


		/** 武器 */
		struct MasterWeaponParameter : public IParameter
		{
			appParameter(MasterWeaponParameter);

			float attackPower;			// 攻撃力
		};


		/** イベントキャラクター */
		struct MasterEventCharacterParameter : public IParameter
		{
			appParameter(MasterEventCharacterParameter);

			float moveSpeed;			// 移動速度
			float jumpMoveSpeed;		// ジャンプ中の移動速度
			float jumpPower;			// ジャンプ力
			float radius;				// 半径
			float height;				// 高さ
		};


		/** フェーズ単位のパラメーター */
		struct EnemyPhaseParameter
		{
			int requiredPlayerLevel;	// フェーズ開始のプレイヤーLv
			float attackPower;			// フェーズ毎の攻撃力
		};


		/** きのこイベントキャラクター */
		struct MasterMushroomEventCharacterParameter : public IParameter
		{
			appParameter(MasterMushroomEventCharacterParameter);

			float moveSpeed;			// 移動速度
			float jumpMoveSpeed;		// ジャンプ中の移動速度
			float jumpPower;			// ジャンプ力
			float radius;				// 半径
			float height;				// 高さ
			float hp;					// HP
			float spawnLightColorR = 0.2f;	// スポーンライト色 R
			float spawnLightColorG = 1.0f;	// スポーンライト色 G
			float spawnLightColorB = 0.35f;	// スポーンライト色 B
			std::vector<EnemyPhaseParameter> phases;	// フェーズデータ
		};


		/** ストーンイベントキャラクター */
		struct MasterStoneEventCharacterParameter : public IParameter
		{
			appParameter(MasterStoneEventCharacterParameter);

			float moveSpeed;			// 移動速度
			float jumpMoveSpeed;		// ジャンプ中の移動速度
			float jumpPower;			// ジャンプ力
			float radius;				// 半径
			float height;				// 高さ
			float hp;					// HP
			float spawnLightColorR = 1.0f;	// スポーンライト色 R
			float spawnLightColorG = 0.35f;	// スポーンライト色 G
			float spawnLightColorB = 0.05f;	// スポーンライト色 B
			std::vector<EnemyPhaseParameter> phases;	// フェーズデータ
		};


		/** ステージ全体 */
		struct MasterStageParameter : public IParameter
		{
			appParameter(MasterStageParameter);

			float gravity;				// 重力
			float fallLimitY;			// 落下リミットY座標
			float friction;				// 摩擦係数
			float warpStartScale;		// ワープ開始スケール
			float warpEndScale;			// ワープ終了スケール
			float warpTime;				// ワープ時間
		};


		/** バトルカメラ */
		struct MasterBattleCameraParameter : public IParameter
		{
			appParameter(MasterBattleCameraParameter);

			float distance;		// カメラの距離
			float height;		// カメラの高さ
			float fov;			// カメラFOV
			float nearClip;		// ニアクリップ
			float farClip;		// ファークリップ
			float rotationX;	// 回転X
			float rotationY;	// 回転Y
		};


		/** 音量設定メニュー */
		struct MasterSoundOptionMenuParameter : public IParameter
		{
			appParameter(MasterSoundOptionMenuParameter);
			//
			float gaugeBarX[11];
			float gaugeBarY[3];

			//
			float gaugeBarScaleX[11];

			float knobX[11];
		};


		/** ポーズメニュー */
		struct MasterPauseMenuParameter : public IParameter
		{
			appParameter(MasterPauseMenuParameter);

			float cursolPositionX[2]; //
			float cursolPositionY[2]; //
		};


		/** タイトルに戻るメニュー */
		struct ReturnToTitleMenuParameter : public IParameter
		{
			appParameter(ReturnToTitleMenuParameter);

			float cursolPositionX[2]; //
			float cursolPositionY[2]; //
		};

		/** カメラオプションメニュー */
		struct CameraOptionMenuParameter : public IParameter
		{
			appParameter(CameraOptionMenuParameter);

			float cursolPositionX[4];
			float cursolPositionY[4];

			float highlightPositionX[4];
			float highlightPositionY[4];

			float barScaleX[11];
		};

		/** PlayerのHPUI */
		struct MasterHpUIParameter : public IParameter
		{
			appParameter(MasterHpUIParameter);

			// HPバーの座標X
			float hpBarPositionX[11];
			// HPバーのスケールX
			float hpBarScaleX[11];
			// レベルバーの座標X
			float levelBarPositionX[11];
			// レベルバーのスケールX
			float levelBarScaleX[11];
		};


		/** EnemyのHPUI */
		struct MasterEnemyHpUIParameter : public IParameter
		{
			appParameter(MasterEnemyHpUIParameter);

			// HPバーの座標X
			float enemyHpBarPositionX[11];
			// HPバーのスケールX
			float enemyHpBarScaleX[11];
		};



#undef appParameter

		/**
		 * パラメーター管理クラス
		 */
		class ParameterManager
		{
		private:
			using ParameterVector = std::vector<IParameter*>;
			using ParameterMap = std::map<uint32_t, ParameterVector>;

		private:
			/** パラメーターIDのリスト */
			ParameterMap m_parameterMap;

		private:
			ParameterManager();
			~ParameterManager();

		public:

			/**
			 * パラメーターファイルを読み込む
			 * 関数ポインタで読み込み処理を受け取る
			 * @typename T パラメーターの種類
			 * @param path ファイルパス
			 * @param func 読み込み処理
			 */
			template<typename T>
			void LoadParameter(const char* path, const std::function<void(const nlohmann::json& json, T& p)>& func)
			{
				if (m_parameterMap.find(T::ID()) != m_parameterMap.end())
				{
					return;
				}
				//ファイルを開く
				std::ifstream file(path);
				if (!file.is_open())
				{
					return;
				}

				//jsonファイルとして読み込む？
				nlohmann::json jsonRoot;
				file >> jsonRoot;

				//読み込んだパラメーターを一時的に受け取る
				ParameterVector parameters;

				for (const auto& j : jsonRoot)
				{
					T* parameter = new T();
#ifdef APP_ENABLE_PARAM_HOT_RELOAD
					parameter->m_path = std::string(path);
					parameter->m_lastWriteTime = GetFileLastWriteTime(path);
					parameter->load = func;
#endif // APP_ENABLE_PARAM_HOT_RELOAD

					//パラメーター読み込み処理
					func(j, *parameter);
					parameters.push_back(static_cast<IParameter*>(parameter));
				}

				//パラメーターを登録
				m_parameterMap.emplace(T::ID(), parameters);
			}

			/// <summary>
			/// パラメーター解放
			/// </summary>
			/// <param name="path">解放するパラメーターのファイルパス</param>
			template <typename T>
			void UnloadParameter()
			{
				auto it = m_parameterMap.find(T::ID());
				if (it != m_parameterMap.end())
				{
					auto& parameters = it->second;
					for (auto* p : parameters)
					{
						delete p;
					}
					m_parameterMap.erase(it);
				}
			}

			/// <summary>
			/// パラメーターの取得
			/// </summary>
			/// <typeparam name="T">取得するパラメーターの構造体</typeparam>
			/// <param name="path">取得するパラメーターのファイルパス</param>
			/// <param name="index">１つのファイルに複数のパラメーターがある場合は何番目かを指定する</param>
			/// <returns></returns>
			template <typename T>
			const T* GetParameter(const int index = 0) const
			{
				const auto parameters = GetParameters<T>();
				if (parameters.size() == 0)
				{
					return nullptr;
				}
				if (parameters.size() <= index)
				{
					return nullptr;
				}

				return parameters[index];
			}

			/// <summary>
			/// 複数のパラメーターを取得する
			/// </summary>
			/// <typeparam name="T">取得するパラメーターの構造体</typeparam>
			/// <param name="path">取得するパラメーターのファイルパス</param>
			/// <returns></returns>
			template <typename T>
			const std::vector<T*> GetParameters() const
			{
				std::vector<T*> parameters;

				auto it = m_parameterMap.find(T::ID());
				if (it != m_parameterMap.end())
				{
					for (auto* parameter : it->second)
					{
						parameters.push_back(static_cast<T*>(parameter));
					}
				}
				return parameters;
			}

			/// <summary>
			/// パラメーターをラムダ式で回す
			/// </summary>
			/// <typeparam name="T"></typeparam>
			/// <param name="path"></param>
			/// <param name="func"></param>
			template<typename T>
			void ForEach(std::function<void(const T&)> func) const
			{
				const std::vector<T*> parameters = GetParameters<T>();
				for (const T* parameter : parameters)
				{
					func(*parameter);
				}
			}

		public:
#ifdef APP_ENABLE_PARAM_HOT_RELOAD
			void Update()
			{
				for (auto paramPair : m_parameterMap)
				{
					for (auto param : paramPair.second)
					{
						if (CheckFileModified(param))
						{
							std::ifstream file(param->m_path);
							if (!file.is_open())
							{
								return;
							}

							nlohmann::json jsonRoot;
							file >> jsonRoot;

							ParameterVector parameters;

							for (const auto& j : jsonRoot)
							{
								param->m_lastWriteTime = GetFileLastWriteTime(param->m_path.c_str());
								param->Load(j);
							}
						}
					}
				}
			}

			//ファイル更新時間取得
			static time_t GetFileLastWriteTime(const char* path)
			{
				struct stat result;
				//stat関数でファイル情報を取得(0なら成功)
				if (stat(path, &result) == 0)
				{
					return result.st_mtime;
				}

				return 0;
			}

			//ファイル更新チェック
			static bool CheckFileModified(const IParameter* param)
			{
				//ファイルの更新時間が変更されているか確認
				if (GetFileLastWriteTime(param->m_path.c_str()) > param->m_lastWriteTime)
				{
					return true;
				}
				return false;
			}
#endif // APP_ENABLE_PARAM_HOT_RELOAD


			/*
			* シングルトン用コード
			*/
		private:
			static ParameterManager* m_instance;
		public:

			/// <summary>
			/// インスタンス生成
			/// </summary>
			static void Initialize()
			{
				if (m_instance == nullptr)
				{
					m_instance = new ParameterManager();
				}
			}

			/// <summary>
			/// インスタンスを取得
			/// </summary>
			/// <returns></returns>
			static ParameterManager& Get()
			{
				return *m_instance;
			}

			static void Finalize()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}
		};


	}
}