#pragma once

namespace nsK2EngineLow
{
    /**
     * 円形ゲージスプライトレンダラー。
     * fillAmount: 0.0=空, 1.0=1周, 2.0=2周分。
     * startAngle: ラジアン単位、0=上(12時方向), PI/2=右, PI=下。
     * arcSpan   : ラジアン単位、Math::PI2=真円, Math::PI=半円。
     * innerRadius: 0=塗りつぶし円, 0.4=リング状ゲージ。
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
            // 空エリアの色
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

        void Init(const char* filePath,								//ファイルパス
            const float w,											//画像の横幅
            const float h,											//画像の縦幅
            AlphaBlendMode alphaBlendMode = AlphaBlendMode_Trans);	//デフォルトは半透明合成

        /** トランスフォーム設定 */
        void SetPosition(const Vector3& pos) { m_spriteRender.SetPosition(pos); }
        const Vector3& GetPosition() const { return m_spriteRender.GetPosition(); }

        void SetScale(const Vector3& scale) { m_spriteRender.SetScale(scale); }
        void SetScale(float scale) { m_spriteRender.SetScale(scale); }
        const Vector3& GetScale() const { return m_spriteRender.GetScale(); }

        void SetRotation(const Quaternion& rot) { m_spriteRender.SetRotation(rot); }
        const Quaternion& GetRotation() const { return m_spriteRender.GetRotation(); }

        void SetPivot(const Vector2& pivot) { m_spriteRender.SetPivot(pivot); }
        const Vector2& GetPivot() const { return m_spriteRender.GetPivot(); }

        /** ゲージパラメータ設定 */
        void  SetFillAmount(float v) { m_gaugeCB.fillAmount = v; }
        float GetFillAmount() const { return m_gaugeCB.fillAmount; }

        void  SetStartAngle(float rad) { m_gaugeCB.startAngle = rad; }
        float GetStartAngle() const { return m_gaugeCB.startAngle; }

        void  SetArcSpan(float rad) { m_gaugeCB.arcSpan = rad; }
        float GetArcSpan() const { return m_gaugeCB.arcSpan; }

        void SetFillColor(const Vector4& color) { m_gaugeCB.fillColor = color; }
        void SetFillColor2(const Vector4& color) { m_gaugeCB.fillColor2 = color; }
        void SetEmptyColor(const Vector4& color) { m_gaugeCB.emptyColor = color; }

        void  SetInnerRadius(float r) { m_gaugeCB.innerRadius = r; }
        float GetInnerRadius() const { return m_gaugeCB.innerRadius; }

        void  SetOuterRadius(float r) { m_gaugeCB.outerRadius = r; }
        float GetOuterRadius() const { return m_gaugeCB.outerRadius; }


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
        // 内部で使用するSpriteRenderインスタンス
        SpriteRender m_spriteRender;
        // ゲージ用の定数バッファデータ
        GaugeCBData  m_gaugeCB;
    };

}