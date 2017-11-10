#include "include/Camera.hlsli"
#include "include/Fragment.hlsli"
#include "include/LightCalc.hlsli"

#define USE_GRID_TEXTURE

cbuffer cb0 : register(b0) { Camera camera; };
cbuffer cb1 : register(b1) { DirectionalLight globalLight; };

SamplerState           linearClamp     : register(s0);
SamplerState           linearWrap      : register(s1);
SamplerComparisonState comparison      : register(s2);

Texture2D              shadowMap       : register(t3);
#ifdef USE_GRID_TEXTURE
Texture2D              gridTexture     : register(t4);
#endif

Texture2D              diffuseTexture  : register(t12);
Texture2D              normalTexture   : register(t13);
Texture2D              specularTexture : register(t14);
Texture2D              glowTexture     : register(t15);

struct Targets
{
    float4 color      : SV_Target0;
    float4 glow       : SV_Target1;
    float4 viewNormal : SV_Target2;
};

Targets PS(Fragment fragment) 
{
    Targets targets;
    
    float3 normal = calcNormal(normalTexture.Sample(linearClamp, fragment.uv).xyz, fragment.normal, fragment.binormal, fragment.tangent);
    float specularExponent = specularTexture.Sample(linearClamp, fragment.uv).r;

    float shadowFactor = calcShadowFactor(comparison, shadowMap, globalLight, 2);
    float3 viewDir = normalize(camera.position - fragment.position);

    float3 lightSum = float3(0,0,0);
    lightSum += calcLight(globalLight, fragment.position, normal, viewDir, specularExponent);
    lightSum += calcAllLights(fragment.ndcPosition, fragment.position, normal, viewDir, specularExponent);
    
    #ifdef USE_GRID_TEXTURE
    float3 color = gridTexture.Sample(linearWrap, fragment.gridUV).xyz;
    #else
    float3 color = diffuseTexture.Sample(linearClamp, fragment.uv).xyz;
    #endif

    targets.color = float4(lightSum * color, 1); //500~
    targets.glow = glowTexture.Sample(linearClamp, fragment.uv); //300~
    targets.viewNormal = fragment.viewNormal; //300~

    return targets;
}