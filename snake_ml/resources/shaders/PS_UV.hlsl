struct PixelShaderInput
{
    float2 UV : TEXCOORD;
}; 

Texture2D DiffuseTexture         : register(t0);
SamplerState LinearRepeatSampler : register(s0);

float4 main(PixelShaderInput IN) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearRepeatSampler, IN.UV);
    return texColor;
}