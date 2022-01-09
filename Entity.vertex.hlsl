#include "Config.hlsli"

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    uint group_index : GROUP_INDEX;
};

struct VS_OUTPUT
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

matrix mvp;
matrix model;
float3 camPosition;
uint shadowMapCount;
matrix shadowMapMVP[HLSL_SHADOW_MAP_SLOTS];
matrix worldLightMVP;

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.model_pos = (float3) mul(float4(input.position, 1.0f), model);
    output.position = mul(float4(input.position, 1.0f), mvp);
    output.color = input.color;
    output.normal = mul(input.normal, (float3x3) model);
    output.camera_pos = camPosition;
    output.group_index = input.group_index;
    output.uv = input.uv;

    [unroll]
    for (int i = 0; i < HLSL_SHADOW_MAP_SLOTS; i++)
        output.light_position[i] = mul(float4(input.position, 1.0f), shadowMapMVP[i]);

    output.world_light_position = mul(float4(input.position, 1.0f), worldLightMVP);

    output.camera_ray = camPosition.xyz - output.model_pos.xyz;

    return output;
}