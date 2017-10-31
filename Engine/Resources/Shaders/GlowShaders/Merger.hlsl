#include "../ShaderConstants.hlsli"

Texture2D backBuffer : register(t0);
Texture2D toMerge : register(t1);
sampler Sampler : register(s0);

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
};

VSOut VS(uint id: SV_VertexID)
{
    VSOut vsout;
    
	vsout.uv = float2((id << 1) & 2, id & 2);
	vsout.pos = float4(vsout.uv * float2(2, -2) + float2(-1, 1), 0, 1);

    return vsout;
}

float4 PS(VSOut input) : SV_Target0
{
    float3 color = backBuffer.Sample(Sampler, input.uv);
    float3 glowColor = toMerge.Sample(Sampler, input.uv);

      

    color = adjustSaturation(color, GLOW_ORIGINAL_SATURATION) * GLOW_ORIGINAL_INTENSITY;
    glowColor = adjustSaturation(glowColor, GLOW_SATURATION) * GLOW_INTENSITY;


    color *= (1 - saturate(glowColor));

    color = saturate(color + glowColor);

    return float4(color, 1);
}