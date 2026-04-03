#pragma once

/*
* ParameterManager.h
* パラメーター管理
* こいつにキャラクターのステータスなど外部ファイルから読み込ませて保持させ、それを受け取って使う。
* パラメーターの識別をファイルパスで行っているので、パラメーター取得、解放などで事あるごとにパスを要求するが、
* LoadParameter関数以外で読み込みを行ったりはしない。
* シングルトンクラス。
*/

#include <iostream>
#include <fstream>
#include "json/json.hpp"
#include "util/CRC32.h"


/** ホットリロード有効か */
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
		 * パラメーター基底構造体
		 */
		struct IParameter 
		{
		#ifdef APP_ENABLE_PARAM_HOT_RELOAD
			std::string m_path;								//パラメーターのファイルパス
			time_t m_lastWriteTime;							//最終更新時刻
			virtual void Load(const nlohmann::json& j) {};	// 読み込み関数
		#endif // APP_PARAM_HOT_RELOAD
		};


		/** バトル全般 */
		struct MasterBattleParameter : public IParameter
		{
			appParameter(MasterBattleParameter);

			float battleTime;			// 戦闘時間
		};


		/** バトルキャラクター */
		struct MasterBattleCharacterParameter : public IParameter
		{
			appParameter(MasterBattleCharacterParameter);

			float moveSpeed;			// 移動速度
			float jumpMoveSpeed;		// ジャンプ中の移動速度
			float jumpPower;			// ジャンプ力
			float radius;				// 半径
			float height;				// 高さ
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


		/** ステージ全般 */
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
			
			float distance;		// カメラ距離
			float height;			// カメラ高さ
			float fov;			// カメラFOV
			float nearClip;		// ニアクリップ
			float farClip;		// ファークリップ
			float rotationX;	// 回転X
			float rotationY;	// 回転Y
		};



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
			/** パラメータとIDのリスト */
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
				//ファイルを開く
				std::ifstream file(path);
				if (!file.is_open())
				{
					return;
				}

				//jsonファイルとして読み込む？
				nlohmann::json jsonRoot;
				file >> jsonRoot;

				//読み込んだパラメーターを一時的に持つ受け皿
				ParameterVector parameters;

				for (const auto& j : jsonRoot)
				{
					T* parameter = new T();
		#ifdef APP_ENABLE_PARAM_HOT_RELOAD
					parameter->m_path = std::string(path);
					parameter->m_lastWriteTime = GetFileLastWriteTime(path);
					parameter->load = func;
		#endif // APP_ENABLE_PARAM_HOT_RELOAD

					//パラメータ読み込み処理
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
			/// <param name="index">一つのファイルに複数のパラメーターを入れた場合は何番目かこれで指定する</param>
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

			//ファイル更新日時取得
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
				//ファイルの更新日時から変更があったか確認
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