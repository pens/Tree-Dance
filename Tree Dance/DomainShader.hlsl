/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

struct DSIn {
    float3 position : POS;
    float3 normal : NORM;
    float3 tangent : TAN;
    float2 uv : TEX;
};
cbuffer cb : register(b0) {
    float4x4 view_proj;
    float3 eye_pos;
}
SamplerState samp_state : register(s0);
Texture2D displacement : register(t0);

[domain("tri")]
void main(float edge_tess[3] : SV_TessFactor, 
          float inside_tess : SV_InsideTessFactor, 
          float3 uvw : SV_DomainLocation, const OutputPatch<DSIn, 3> tri,
          out float3 position : POS, out float3 normal : NORM, 
          out float3 tangent : TAN, out float2 uv : TEX, 
          out float4 pos_h : SV_POSITION) {
    position = uvw.x * tri[0].position + uvw.y * tri[1].position + 
               uvw.z * tri[2].position;
    normal = uvw.x * tri[0].normal + uvw.y * tri[1].normal + 
             uvw.z * tri[2].normal;
    tangent = uvw.x * tri[0].tangent + uvw.y * tri[1].tangent + 
              uvw.z * tri[2].tangent;
    uv = uvw.x * tri[0].uv + uvw.y * tri[1].uv + uvw.z * tri[2].uv;

    normal = normalize(normal);
    //position += (displacement.SampleLevel(samp_state, uv, 0) - 1.0f) * normal;
    pos_h = mul(float4(position, 1.0f), view_proj);
}