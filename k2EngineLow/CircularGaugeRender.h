#pragma once

namespace nsK2EngineLow
{
    /**
     * @brief 円形ゲージを描画するスプライトレンダラー。
     *
     * シェーダー（CircularGauge.fx）を使用して、テクスチャなしで
     * 数学的に円弧を描画する。HPゲージやタイマーUIなどに使用。
     *
     * fillAmount : 0.0=空っぽ, 1.0=1周分, 2.0=2周分。
     * startAngle : ラジアン単位。0=上(12時方向), PI/2=右, PI=下, 3PI/2=左。
     * arcSpan    : ラジアン単位。Math::PI2=真円, Math::PI=半円。
     * innerRadius: 0=塗りつぶし円, 0.4以上=リング状ゲージ。
     * outerRadius: 1.0=スプライト全体, 0.8=少し小さめ。
     */
    class CircularGaugeRender : public IRenderer
    {
    public:
        /**
         * b1レジスタ用の定数バッファ構造体。
         * circularGauge.fxと一致させる必要あり。
         */
        struct GaugeCBData
        {
            // 1周目の塗りつぶし色
            Vector4 fillColor   = { 0.929f, 0.894f, 0.635f, 1.0f };
            // 2周目の塗りつぶし色
            Vector4 fillColor2  = { 1.0f, 0.8f, 0.0f, 1.0f };
            // 空っぽエリアの色
            Vector4 emptyColor  = { 0.2f, 0.2f, 0.2f, 1.0f };
            // 開始角度 (ラジアン)
            float startAngle  = 0.0f;
            // 塗りつぶし量 (0.0 - 2.0)
            float fillAmount  = 1.0f;
            // 弧の長さ (Math::PI2 = 真円)
            float arcSpan     = 6.28318530f;
            // 内径 (0=円, 0.4=リング)
            float innerRadius = 0.4f;
            // 外径 (1.0=全体、0.8=少し小さめ)
            float outerRadius = 1.0f;
            // パディング    
            float _pad[3] = {};
        };

        /**
         * @brief 初期化処理。
         * @param filePath       テクスチャファイルパス（不要な場合はnullptrを渡す）
         * @param w              スプライトの横幅（ピクセル）
         * @param h              スプライトの縦幅（ピクセル）
         * @param alphaBlendMode アルファブレンドモード（デフォルト：半透明合成）
         */
        void Init(const char* filePath,								
            const float w,											
            const float h,											
            AlphaBlendMode alphaBlendMode = AlphaBlendMode_Trans);

        /** ワールド座標を設定する */
        void SetPosition(const Vector3& pos) { m_spriteRender.SetPosition(pos); }
        /** ワールド座標を取得する */
        const Vector3& GetPosition() const { return m_spriteRender.GetPosition(); }

        /** スケールを設定する（Vector3） */
        void SetScale(const Vector3& scale) { m_spriteRender.SetScale(scale); }
        /** スケールを設定する（均一スケール） */
        void SetScale(float scale) { m_spriteRender.SetScale(scale); }
        /** スケールを取得する */
        const Vector3& GetScale() const { return m_spriteRender.GetScale(); }

        /** 回転を設定する */
        void SetRotation(const Quaternion& rot) { m_spriteRender.SetRotation(rot); }
        /** 回転を取得する */
        const Quaternion& GetRotation() const { return m_spriteRender.GetRotation(); }

        /** ピボット（回転・スケールの基点）を設定する */
        void SetPivot(const Vector2& pivot) { m_spriteRender.SetPivot(pivot); }
        /** ピボットを取得する */
        const Vector2& GetPivot() const { return m_spriteRender.GetPivot(); }


        /** ゲージパラメータ設定 */
        /** 塗りつぶし量を設定（0.0〜2.0） */
        void  SetFillAmount(float v) { m_gaugeCB.fillAmount = v; }
        /** 塗りつぶし量を取得 */
        float GetFillAmount() const { return m_gaugeCB.fillAmount; }

        /** 描画開始角度を設定（ラジアン） */
        void  SetStartAngle(float rad) { m_gaugeCB.startAngle = rad; }
        /** 描画開始角度を取得 */
        float GetStartAngle() const { return m_gaugeCB.startAngle; }

        /** 円弧の広さを設定（ラジアン） */
        void  SetArcSpan(float rad) { m_gaugeCB.arcSpan = rad; }
        /** 円弧の広さを取得 */
        float GetArcSpan() const { return m_gaugeCB.arcSpan; }

        /** 1周目の塗りつぶし色を設定 */
        void SetFillColor(const Vector4& color) { m_gaugeCB.fillColor = color; }
        /** 2周目の塗りつぶし色を設定 */
        void SetFillColor2(const Vector4& color) { m_gaugeCB.fillColor2 = color; }
        /** 空っぽエリアの色を設定 */
        void SetEmptyColor(const Vector4& color) { m_gaugeCB.emptyColor = color; }

        /** 内径の比率を設定（0=塗りつぶし円、0.4以上=リング状） */
        void  SetInnerRadius(float r) { m_gaugeCB.innerRadius = r; }
        /** 内径の比率を取得 */
        float GetInnerRadius() const { return m_gaugeCB.innerRadius; }

        /** 外径の比率を設定（1.0=スプライト全体） */
        void  SetOuterRadius(float r) { m_gaugeCB.outerRadius = r; }
        /** 外径の比率を取得 */
        float GetOuterRadius() const { return m_gaugeCB.outerRadius; }

        /**
         * @brief 定数バッファデータへの参照を取得する。
         * @note  直接編集する場合のみ使用すること。
         *        通常は各Setterを使用する。
         */
        GaugeCBData& GetGaugeCBData() { return m_gaugeCB; }

        /** 更新処理 */
        void Update()
        {
            m_spriteRender.Update();
        }

        /** 描画登録 */
        void Draw(RenderContext& rc)
        {
            m_spriteRender.Draw(rc);
        }

        /** 2Dレンダリングパスでの実描画。*/
        void OnRender2D(RenderContext& rc) override
        {
            m_spriteRender.OnRender2D(rc);
        }

    private:
        /** 内部で使用するスプライトレンダラー */
        SpriteRender m_spriteRender;
        /** GPUに送信するゲージパラメータ（定数バッファ） */
        GaugeCBData  m_gaugeCB;
    };

}