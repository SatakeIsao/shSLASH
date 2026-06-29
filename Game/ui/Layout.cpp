/**
 * Layout.cpp
 * UIのレイアウト管理
 */
#include "stdafx.h"
#include "Layout.h"
#include <fstream>
#include <sys/stat.h>
#include <Windows.h>


namespace
{
    // UTF-8 (JSONの文字列) を Shift-JIS (Windowsアプリ用) に変換する
    std::wstring Utf8ToShiftJis(const std::string& utf8Str)
    {
        if (utf8Str.empty()) return std::wstring();

        // 1. UTF-8 を Unicode (UTF-16) に変換
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), &wstrTo[0], size_needed);

        return wstrTo;
    }


    /**
     * ==========================================
     * パース関連
     * ==========================================
     */
    Vector3 ParseVector3(const nlohmann::json& arr)
    {
        return Vector3(
            arr[0].get<float>(),
            arr[1].get<float>(),
            arr[2].get<float>()
        );
    }


    Vector4 ParseVector4(const nlohmann::json& arr)
    {
        return Vector4(
            arr[0].get<float>(),
            arr[1].get<float>(),
            arr[2].get<float>(),
            arr[3].get<float>()
        );
    }


    Vector4 ParseColor(const nlohmann::json& arr)
    {
        return Vector4(
            arr[0].get<float>() / 255.0f,
            arr[1].get<float>() / 255.0f,
            arr[2].get<float>() / 255.0f,
            arr[3].get<float>() / 255.0f
        );
    }


    Quaternion ParseRotation(const float rotation)
    {
        Quaternion q;
        q.SetRotationDegZ(rotation);
        return q;
    }


    /**
     * ==========================================
     * 初期化関連
     * ==========================================
     */
    template <typename T>
    void InitializeUIParts(T* parts, const nlohmann::json& item)
    {
        K2_ASSERT(false, "未実装\n");
    }

    void InitializeUIParts(app::ui::UIIcon* image, const nlohmann::json& item)
    {
        const std::string assetName = item["asset"].get<std::string>();
        const float w = item["width"].get<float>();
        const float h = item["height"].get<float>();
        const Vector3 position = ParseVector3(item["position"]);
        const Vector3 scale = ParseVector3(item["scale"]);
        const Quaternion rotation = ParseRotation(item["rotation"].get<float>());
        const Vector4 color = ParseVector4(item["color"]);

        // shaderフィールドがあればカスタムシェーダーで初期化
        if (item.contains("shader"))
        {
            const std::string shaderPath = item["shader"].get<std::string>();
            SpriteInitData initData;
            initData.m_ddsFilePath[0] = assetName.c_str();
            initData.m_fxFilePath = shaderPath.c_str();
            initData.m_width = static_cast<UINT>(w);
            initData.m_height = static_cast<UINT>(h);
            initData.m_alphaBlendMode = AlphaBlendMode_Trans;
            image->Initialize(initData);
        }
        else
        {
            image->Initialize(assetName.c_str(), w, h);
        }

        //image->Initialize(assetName.c_str(), w, h);
        image->transform.localPosition = position;
        image->transform.localScale = scale;
        image->transform.localRotation = rotation;
        image->color = color;
    }
    void InitializeUIParts(app::ui::UIText* text, const nlohmann::json& item)
    {
        const Vector3 position = ParseVector3(item["position"]);
        const Vector3 scale = ParseVector3(item["scale"]);
        const Vector4 color = ParseVector3(item["color"]);
        const auto str = item["text"].get<std::string>();
        const auto wstr = Utf8ToShiftJis(str);

        text->SetText(wstr.c_str());
        text->transform.localPosition = position;
        text->transform.localScale = scale;
        text->color = color;
    }
    void InitializeUIParts(app::ui::UIDigit* text, const nlohmann::json& item)
    {
        const std::string assetName = item["asset"].get<std::string>();
        const int digitCount = item["digit"].get<int>();
        const float w = item["width"].get<float>();
        const float h = item["height"].get<float>();
        const Vector3 position = ParseVector3(item["position"]);
        const Vector3 scale = ParseVector3(item["scale"]);
        const Quaternion rotation = ParseRotation(item["rotation"].get<float>());

        // 初期値の数値は0としておく
        text->Initialize(assetName.c_str(), digitCount, 0, w, h, position, scale, rotation);
    }
    void InitializeUIParts(app::ui::UINumberSprite* sprite, const nlohmann::json& item)
    {
        const std::string atlasPath = item["asset"].get<std::string>();
        const int digitCount = item["digit"].get<int>();
        const float w = item["width"].get<float>();
        const float h = item["height"].get<float>();
        const Vector3 position = ParseVector3(item["position"]);
        const Vector3 scale = ParseVector3(item["scale"]);
        const Quaternion rotation = ParseRotation(item["rotation"].get<float>());

        sprite->Initialize(atlasPath.c_str(), digitCount, 0, w, h, position, scale, rotation);

        if (item.contains("xBias")) {
            sprite->SetXBias(item["xBias"].get<float>());
        }
        if (item.contains("xScaleBias") || item.contains("yScaleBias")) {
            const float xs = item.contains("xScaleBias") ? item["xScaleBias"].get<float>() : 0.0f;
            const float ys = item.contains("yScaleBias") ? item["yScaleBias"].get<float>() : 0.0f;
            sprite->SetScaleBias(xs, ys);
        }
        if (item.contains("uvOffsets") || item.contains("uvScales")) {
            float offsets[10] = {}, scales[10] = {};
            const float* pOffsets = nullptr;
            const float* pScales  = nullptr;
            if (item.contains("uvOffsets")) {
                const auto& arr = item["uvOffsets"];
                for (int i = 0; i < 10 && i < static_cast<int>(arr.size()); i++)
                    offsets[i] = arr[i].get<float>();
                pOffsets = offsets;
            }
            if (item.contains("uvScales")) {
                const auto& arr = item["uvScales"];
                for (int i = 0; i < 10 && i < static_cast<int>(arr.size()); i++)
                    scales[i] = arr[i].get<float>();
                pScales = scales;
            }
            sprite->SetDigitUVTable(pOffsets, pScales);
        }
    }
}


namespace app
{
    namespace ui
    {

        void Layout::Update()
        {
            menu_->Update();

#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
            // ホットリロードチェック
            struct stat st;
            if (stat(filePath_.c_str(), &st) == 0) {
                if (lastUpdateTime_ != st.st_mtime) {
                    lastUpdateTime_ = st.st_mtime;
                    Reload();
                }
            }
#endif // APP_ENABLE_LAYOUT_HOTRELOAD
        }


        void Layout::Render(RenderContext& rc)
        {
            menu_->Render(rc);
        }


        void Layout::Reload()
        {
            // パース一回限りのキャッシュ: 同じパスを共有する複数のLayoutインスタンス
            // （例: 12個のDamagePopUIObjects）がディスクI/OとJSONパースを一度だけ行う。
            // デバッグビルドではmtimeを使いホットリロード時に古いエントリを無効化する。
            struct CachedJson { nlohmann::json data; time_t mtime = 0; };
            static std::unordered_map<std::string, CachedJson> s_cache;

            nlohmann::json* pJson = nullptr;
            auto it = s_cache.find(filePath_);

#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
            struct stat st;
            bool statOk = (stat(filePath_.c_str(), &st) == 0);
            if (it != s_cache.end() && statOk && it->second.mtime != st.st_mtime) {
                s_cache.erase(it);
                it = s_cache.end();
            }
#endif

            if (it != s_cache.end()) {
                pJson = &it->second.data;
            } else {
                std::ifstream file(filePath_);
                if (!file.is_open()) return;
                CachedJson entry;
                file >> entry.data;
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
                if (statOk) entry.mtime = st.st_mtime;
#endif
                pJson = &s_cache.emplace(filePath_, std::move(entry)).first->second.data;
            }

            auto& j = *pJson;

            // すでにMenuやCanvasがある場合は作り直しを行う
            if (menu_->GetCanvas() == nullptr) {
                menu_->SetCanvas(new UICanvas());
            }

            auto* canvas = menu_->GetCanvas();
            auto& elements = j["canvas"]["elements"];

            for (auto& item : elements) {
                std::string type = item["type"];
                std::string name = item["name"];

                // すでに存在するUIならパラメータ更新のみ
                const uint32_t key = Hash32(name.c_str());
                if (menu_->HasUI(key)) {
                    menu_->UnregisterUI(key);
                    canvas->RemoveUI(key);
                }
                auto* ui = CreateUI(canvas, type, key, item);
                menu_->RegisterUI(key, ui);
            }

            menu_->InitializeLogic();
        }


        UIBase* Layout::CreateUI(UICanvas* canvas, const std::string& type, const uint32_t key, const nlohmann::json& item)
        {
            UIBase* ui = nullptr;
            if (type == "UIIcon") {
                canvas->CreateUI<UIIcon>(key);
                auto* image = canvas->FindUI<UIIcon>(key);
                InitializeUIParts(image, item);
                ui = image;
            }
            else if (type == "UIText") {
                canvas->CreateUI<UIText>(key);
                auto* text = canvas->FindUI<UIText>(key);
                InitializeUIParts(text, item);
                ui = text;
            }
            else if (type == "UIDigit") {
                canvas->CreateUI<UIDigit>(key);
                auto* digit = canvas->FindUI<UIDigit>(key);
                InitializeUIParts(digit, item);
                ui = digit;
            }
            else if (type == "UINumberSprite") {
                canvas->CreateUI<UINumberSprite>(key);
                auto* sprite = canvas->FindUI<UINumberSprite>(key);
                InitializeUIParts(sprite, item);
                ui = sprite;
            }
            //if (type == "UIButton") ...
            //if (type == "UIGauge")  ...

            if (ui && item.contains("isDraw"))
                ui->isDraw = item["isDraw"].get<bool>();

            return ui;
        }
    }
}