struct TextureParameters
{
	int Idx;
};

struct PixelShaderInput
{
	float2 UV : TEXCOORD;
};

Texture2DArray						DiffuseTexture		: register(t0);
SamplerState						LinearRepeatSampler	: register(s0);
ConstantBuffer<TextureParameters>	TextureParams		: register(b1);

float4 main(PixelShaderInput IN) : SV_TARGET
{
	float3 texSampleCoord = { IN.UV.x, IN.UV.y, TextureParams.Idx };
	float4 texColor = DiffuseTexture.Sample(LinearRepeatSampler, texSampleCoord);
	return texColor;
}