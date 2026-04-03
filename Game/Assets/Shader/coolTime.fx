/*!
 * @brief	画面を表示するためのシェーダー
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;    //MVP行列
    float4 mulColor; //乗算カラー
};

//追加した角度のレジスタ
cbuffer degreeCb : register(b1)
{
    float degree;       //割合
    float alpha;        //アルファ値
    bool  addRflag;     //赤を加算するかどうか
}

struct VSInput
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//角度
//float Angle : register(t1);
//カラーテクスチャ
//Texture2D<float4> Texture : register(t0);
//サンプラー
//sampler TextureSampler : register(s0);

Texture2D<float4> colorTexture : register(t0);	         //カラーテクスチャ
Texture2D<float4> staminaBarAlphaTexture : register(t20);//アルファ値操作用のテクスチャ
sampler Sampler : register(s0);

PSInput VSMain(VSInput In) 
{
	PSInput psIn;
	psIn.pos = mul( mvp, In.pos );
	psIn.uv = In.uv;
    
	return psIn;
}

//ピクセルシェーダー
//引数はこれで固定
float4 PSMain(PSInput In) : SV_Target0
{
    float4 colorTex = colorTexture.Sample(Sampler, In.uv);
    
    float4 alphaTex = staminaBarAlphaTexture.Sample(Sampler, In.uv);
    float PI = 3.14159f;
    //中心点
    float2 Center = { 0.5f, 0.48f };
    //上方向
    float2 Up = { 0.5f, 1.0f };
    
    float2 Vector1 = { 0.0f, -1.0f };
    float2 Vector2 = In.uv - Center;
    Vector1 = normalize(Vector1);
    Vector2 = normalize(Vector2);
    
    float Deg = acos(dot(Vector1, Vector2));
    
    Deg = abs(Deg);
    
    //中心からピクセルの座標へのベクトルと
    //中心から上方向へのベクトルの内積を求め
    //逆コサインをとり、設定した角度より小さければ表示
    //大きければピクセルを破棄する
    if (Vector2.x < Vector1.x)
    {
        Deg = PI + (PI - Deg);
    }
    
    if( Deg >= degree)
    {
        //ピクセルを破棄する
        clip(-1);
    }
    
    if(alphaTex.g <= 0.9f)
    {
        colorTex.a = alpha;
    }
    
    return colorTex * mulColor;
}