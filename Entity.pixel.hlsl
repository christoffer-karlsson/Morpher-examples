#include "Config.hlsli"
#include "Functions/DirectionalLight.hlsli"
#include "Functions/ShadowTest.hlsli"
#include "Functions/Light.hlsli"
#include "Functions/FadeShadow.hlsli"

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 model_pos : MODEL_POS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float3 camera_pos : CAMERA_POS;
    float3 camera_ray : CAMERA_RAY;
    float4 light_position[HLSL_SHADOW_MAP_SLOTS] : LIGHT_POSITION;
    float4 world_light_position : WORLD_LIGHT_POSITION;
    uint group_index : GROUP_INDEX;
};

cbuffer CBPS : register(b0)
{
    // Point lights
    PointLight pointlight[HLSL_POINT_LIGHT_SLOTS];
    uint pointlightCount;
    bool pointlightUse;
    float __pad_0;
    float __pad_1;
	// World light 
    float3 worldLightDir;
    float __pad_2;
    float4 worldLightAmbient;
    float4 worldLightDiffuse;
	// Render settings
    float4 fogColor;
    bool fogUse;
    float fogDistance;
    uint shadowMapCount;
    bool textureUse;
	// Group color
    float4 groupColor[HLSL_ENTITY_COLOR_SLOTS];
};

Texture2D worldShadow : register(t0);
SamplerComparisonState worldShadowSampler : register(s0);

Texture2D shadowMap[HLSL_SHADOW_MAP_SLOTS] : register(t1);
SamplerComparisonState shadowMapSampler[HLSL_SHADOW_MAP_SLOTS] : register(s1);

Texture2D tex : register(t13);
SamplerState texSampler : register(s13);

float4 main(PS_INPUT input) : SV_TARGET
{
	// Color from the group color index.
    float alpha = groupColor[input.group_index].w;
    float4 color = float4(0.0f, 0.0f, 0.0f, alpha);
    float4 material = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (textureUse)
    {
        material = tex.Sample(texSampler, input.uv);
    }
    else
    {
        material = groupColor[input.group_index];
        material.xyz *= 0.3f;
    }

    color += DirectionalLight(material, input.normal, worldLightDir, worldLightAmbient, worldLightDiffuse);

    float shadow = 1.0f;

    float3 l = normalize(input.world_light_position.xyz - input.model_pos.xyz);
    float ndotl = dot(normalize(input.normal), l);

	// See if any light is close to the world light shadow, if so, don't use world shadow.
    /*if (pointlightCount > 0)
    {
        if (distance(input.light_position[0], input.model_pos) > 50.0f)
            shadow = ShadowTest(input.world_light_position, ndotl, worldShadow, shadow_sampler);
    }
    else
    {
        shadow = ShadowTest(input.world_light_position, ndotl, worldShadow, shadow_sampler);
    }*/

    shadow = ShadowTest(input.world_light_position, ndotl, worldShadow, worldShadowSampler);
    
    if (shadow < 1.0f)
        color.xyz *= 0.95f;

    [unroll]
    for (int i = 0; i < HLSL_SHADOW_MAP_SLOTS; i++)
    {
        l = normalize(input.light_position[i].xyz - input.model_pos.xyz);
        ndotl = dot(normalize(input.normal), l);

        shadow = ShadowTest(input.light_position[i], ndotl, shadowMap[i], shadowMapSampler[i]);

        float depth = input.light_position[i].z / input.light_position[i].w;

        if (shadow < 1.0f)
            color.xyz *= FadeShadow(0.5f, 0.99f, depth);
    }

    if (pointlightUse)
    {
        for (int i = 0; i < pointlightCount; i++)
            color.xyz += Light(pointlight[i], input.normal, groupColor[input.group_index], input.model_pos);
    }

    if (fogUse)
    {
        float cam_distance = distance(input.camera_pos, input.model_pos);
        float fog_lerp = clamp(cam_distance / fogDistance, 0.0f, 1.0f);
        color = lerp(color, fogColor, fog_lerp);
    }

    return color;
}