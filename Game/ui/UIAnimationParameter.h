/**
 * UIAnimationParameter.h
 * アニメーション情報の外部ファイル管理＆ホットリロード
 */
#pragma once
#include "json/json.hpp"


namespace app
{
	namespace ui
	{
		/**
		 * 1つのアニメーション定義（JSONの1エントリに対応）
		 */
		struct UIAnimationDef
		{
			uint32_t key = 0;

			// 型判別 (jsonの"type"フィールド)
			enum class ValueType { Float, Vector2, Vector3, Vector4 };
			ValueType valueType = ValueType::Float;

			// 共通パラメーター
			float startF = 0.0f;
			float endF = 0.0f;
			Vector2 startV2 = Vector2::Zero;
			Vector2 endV2 = Vector2::Zero;
			Vector3 startV3 = Vector3::Zero;
			Vector3 endV3 = Vector3::Zero;
			Vector4 startV4 = Vector4::White;
			Vector4 endV4 = Vector4::White;

			float duration = 0.0f;
			app::util::EasingType easingType = app::util::EasingType::Linear;
			app::util::LoopMode loopMode = app::util::LoopMode::Once;
		};


		/**
		 * アニメーション情報の管理クラス
		 * JSONファイルから読み込み、ホットリロード対応
		 */
		class UIAnimationParameter : public Noncopyable
		{
		private:
			std::string filePath_;
			std::unordered_map<uint32_t, UIAnimationDef> defMap_;

			/** ホットリロード用: ファイル最終更新時刻 */
			uint64_t lastModifiedTime_ = 0;
			/** リロード監視間隔（秒） */
			float reloadInterval_ = 1.0f;
			float reloadTimer_ = 0.0f;

			/** リロード時のコールバック（UIへの再適用など） */
			std::function<void()> onReloaded_;


		private:
			UIAnimationParameter() {}
			~UIAnimationParameter() {}


		public:
			/**
			 * シングルトン取得
			 */
			static UIAnimationParameter& Get()
			{
				static UIAnimationParameter instance;
				return instance;
			}


			/**
			 * JSONファイルを読み込む
			 * @param filePath jsonファイルパス
			 * @return 成功したか
			 */
			bool Load(const std::string& filePath)
			{
				filePath_ = filePath;
				lastModifiedTime_ = GetFileModifiedTime(filePath_);
				return ParseFile();
			}


			/**
			 * 毎フレーム呼ぶ（ホットリロード監視）
			 */
			void Update(float deltaTime)
			{
				reloadTimer_ += deltaTime;
				if (reloadTimer_ < reloadInterval_) return;
				reloadTimer_ = 0.0f;

				uint64_t currentTime = GetFileModifiedTime(filePath_);
				if (currentTime != lastModifiedTime_) {
					lastModifiedTime_ = currentTime;
					if (ParseFile()) {
						if (onReloaded_) onReloaded_();
					}
				}
			}


			/**
			 * キー(CRC32ハッシュ)でアニメーション定義を取得
			 * @return 見つからなければnullptr
			 */
			const UIAnimationDef* Find(uint32_t key) const
			{
				auto it = defMap_.find(key);
				if (it != defMap_.end()) {
					return &it->second;
				}
				return nullptr;
			}


			/**
			 * リロード時のコールバック設定
			 */
			void SetOnReloaded(const std::function<void()>& callback)
			{
				onReloaded_ = callback;
			}


			/**
			 * リロード監視間隔を設定
			 */
			void SetReloadInterval(float sec) { reloadInterval_ = sec; }


		private:
			/**
			 * JSONファイルを解析してdefMap_に格納
			 */
			bool ParseFile()
			{
				// ファイル読み込み
				std::string jsonText;
				if (!ReadFileToString(filePath_, jsonText)) {
					return false;
				}

				nlohmann::json root;
				try {
					root = nlohmann::json::parse(jsonText);
				}
				catch (...) {
					return false;
				}

				defMap_.clear();

				// "animations" 配列を走査
				if (!root.contains("animations") || !root["animations"].is_array()) {
					return false;
				}

				for (const auto& item : root["animations"])
				{
					UIAnimationDef def;

					// キー文字列 → CRC32ハッシュ
					std::string keyStr = item.value("key", "");
					if (keyStr.empty()) continue;
					def.key = app::util::ComputeCrc32(keyStr.c_str());

					// 値の型.
					std::string valTypeStr = item.value("valueType", "float");
					def.valueType = ParseValueType(valTypeStr);

					// 開始・終了値
					ParseValues(item, def);

					// 共通パラメーター
					def.duration = item.value("duration", 0.3f);
					def.easingType = ParseEasingType(item.value("easing", "Linear"));
					def.loopMode = ParseLoopMode(item.value("loop", "Once"));

					defMap_[def.key] = def;
				}

				return true;
			}


			// --- ユーティリティ ---

			static UIAnimationDef::ValueType ParseValueType(const std::string& s)
			{
				if (s == "Vector2") return UIAnimationDef::ValueType::Vector2;
				if (s == "Vector3") return UIAnimationDef::ValueType::Vector3;
				if (s == "Vector4") return UIAnimationDef::ValueType::Vector4;
				return UIAnimationDef::ValueType::Float;
			}

			static app::util::EasingType ParseEasingType(const std::string& s)
			{
				if (s == "EaseIn")    return app::util::EasingType::EaseIn;
				if (s == "EaseOut")   return app::util::EasingType::EaseOut;
				if (s == "EaseInOut") return app::util::EasingType::EaseInOut;
				return app::util::EasingType::Linear;
			}

			static app::util::LoopMode ParseLoopMode(const std::string& s)
			{
				if (s == "Loop")     return app::util::LoopMode::Loop;
				if (s == "PingPong") return app::util::LoopMode::PingPong;
				return app::util::LoopMode::Once;
			}

			static void ParseValues(const nlohmann::json& item, UIAnimationDef& def)
			{
				switch (def.valueType)
				{
				case UIAnimationDef::ValueType::Float:
					def.startF = item.value("startValue", 0.0f);
					def.endF = item.value("endValue", 0.0f);
					break;

				case UIAnimationDef::ValueType::Vector2:
					ParseVec2(item, "startValue", def.startV2);
					ParseVec2(item, "endValue", def.endV2);
					break;

				case UIAnimationDef::ValueType::Vector3:
					ParseVec3(item, "startValue", def.startV3);
					ParseVec3(item, "endValue", def.endV3);
					break;

				case UIAnimationDef::ValueType::Vector4:
					ParseVec4(item, "startValue", def.startV4);
					ParseVec4(item, "endValue", def.endV4);
					break;
				}
			}

			static void ParseVec2(const nlohmann::json& parent, const char* name, Vector2& out)
			{
				if (!parent.contains(name)) return;
				const auto& v = parent[name];
				out.x = v.value("x", 0.0f);
				out.y = v.value("y", 0.0f);
			}

			static void ParseVec3(const nlohmann::json& parent, const char* name, Vector3& out)
			{
				if (!parent.contains(name)) return;
				const auto& v = parent[name];
				out.x = v.value("x", 0.0f);
				out.y = v.value("y", 0.0f);
				out.z = v.value("z", 0.0f);
			}

			static void ParseVec4(const nlohmann::json& parent, const char* name, Vector4& out)
			{
				if (!parent.contains(name)) return;
				const auto& v = parent[name];
				out.x = v.value("x", 0.0f);
				out.y = v.value("y", 0.0f);
				out.z = v.value("z", 0.0f);
				out.w = v.value("w", 0.0f);
			}


			/**
			 * ファイルを文字列として読み込む
			 */
			static bool ReadFileToString(const std::string& path, std::string& out)
			{
				FILE* fp = fopen(path.c_str(), "rb");
				if (!fp) return false;

				fseek(fp, 0, SEEK_END);
				long size = ftell(fp);
				fseek(fp, 0, SEEK_SET);

				out.resize(size);
				fread(&out[0], 1, size, fp);
				fclose(fp);
				return true;
			}

			/**
			 * ファイル更新時刻を取得
			 */
			static uint64_t GetFileModifiedTime(const std::string& path)
			{
#if defined(_WIN32)
				WIN32_FILE_ATTRIBUTE_DATA data;
				if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &data)) {
					ULARGE_INTEGER ull;
					ull.LowPart = data.ftLastWriteTime.dwLowDateTime;
					ull.HighPart = data.ftLastWriteTime.dwHighDateTime;
					return ull.QuadPart;
				}
#endif
				return 0;
			}
		};
	}
}