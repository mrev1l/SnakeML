struct ModelViewProjection
{
	matrix MVP;
}; 

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexPosUV
{
	float3 Position : POSITION;
	float2 UV		: TEXCOORD;
};

struct VertexShaderOutput
{
	float2 UV		: TEXCOORD;
	float4 Position : SV_Position;
};

VertexShaderOutput main(VertexPosUV IN)
{
	VertexShaderOutput OUT;

	OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
	OUT.UV = IN.UV;

	return OUT;
}