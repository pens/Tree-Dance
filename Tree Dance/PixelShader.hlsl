/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

SamplerState samp_state : register(s0);
Texture2D albedo : register(t0);
Texture2D specular : register(t1);
Texture2D normals : register(t2);
Texture2D roughness : register(t3);

void main(float3 position : POS, float3 normal : NORM, float3 tangent : TAN,
          float2 uv : TEX, float4 positionH : SV_POSITION,
          out float4 gbuffer1 : SV_TARGET0, out float4 gbuffer2 : SV_TARGET1) {
    float3 norm_t = 2.0f * normals.Sample(samp_state, uv).rgb - 1.0f;
    float3 N = normalize(normal);
    float3 T = normalize(tangent - dot(tangent, N) * N);
    float3 B = cross(N, T);
    float3 norm_w = (mul(norm_t, float3x3(T, B, N)) + 1) / 2;

    gbuffer1 = float4(albedo.Sample(samp_state, uv).rgb, 
                      specular.Sample(samp_state, uv).r);
    gbuffer2 = float4(norm_w, roughness.Sample(samp_state, uv).r);
}
