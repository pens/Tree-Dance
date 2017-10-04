/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

cbuffer cb : register(b0) {
    float3 eye_pos;
    float tess_dist_max, tess_dist_min, tess_fact_max, tess_fact_min;
    uint bone_count;
}

Buffer<float4> world_transforms : register(t0);
Buffer<float4> animations : register(t1);

float4x4 GetMatrixFromBuffer(Buffer<float4> buffer, uint index);

void main(inout float3 position : POS, inout float3 normal : NORM,
          inout float3 tangent : TAN, inout float2 uv : TEX,
          in int4 bone_IDs : BONEIDS, in float4 weights : WEIGHTS,
          in uint inst_ID : SV_InstanceID, out float tess_fact : TESS) {
    float4 pos_t = 0;
    float3 norm_t = 0;
    float3 tan_t = 0;
    
    float4x4 world = GetMatrixFromBuffer(world_transforms, inst_ID * 2);
    float4x4 world_inv_trans = GetMatrixFromBuffer(world_transforms, 
                                                   inst_ID * 2 + 1);

    if (bone_IDs[0] == -1) {
        pos_t = float4(position, 1);
        norm_t = normal;
        tan_t = tangent;
    }
    else {
        for (uint i = 0; i < 4; ++i) {
            if (bone_IDs[i] > -1) {
                float4x4 transform = 
                    GetMatrixFromBuffer(animations,
                                        inst_ID * bone_count + bone_IDs[i]);
                pos_t += weights[i] * mul(float4(position, 1), transform);
                //NO NONUNIFORM SCALING
                norm_t += weights[i] * mul(normal, (float3x3)transform);
                tan_t += weights[i] * mul(tangent, (float3x3)transform);
            }
        }
    }

    position = mul(pos_t, world).xyz;
    normal = mul(norm_t, (float3x3)world_inv_trans);
    tangent = mul(tan_t, (float3x3)world);

    tess_fact = tess_fact_min + (tess_fact_max - tess_fact_min) *
        saturate((tess_dist_min - distance(position, eye_pos)) / 
                 (tess_dist_min - tess_dist_max));
}

float4x4 GetMatrixFromBuffer(Buffer<float4> buffer, uint index) {
    index *= 4;
    return float4x4(buffer[index], buffer[index + 1], buffer[index + 2], 
                    buffer[index + 3]);
}