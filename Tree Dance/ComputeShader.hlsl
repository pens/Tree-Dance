/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

struct Light {
    float3 color;
    float intensity;
    float3 position;
    float range;
    float3 direction;
    float spot_angle;
};
cbuffer cb : register(b0) {
    float4x4 view_inv;
    float3 eye_pos;
    float proj_11, proj_22, proj_33, proj_43;
    int height, width;
}
Texture2D gbuffer1 : register(t0);
Texture2D gbuffer2 : register(t1);
Texture2D zbuffer : register(t2);
StructuredBuffer<Light> lights : register(t3);
RWTexture2D<float4> render_target : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    float depth = zbuffer[id.xy].r;
    float3 pos = proj_43 / (depth - proj_33);
    pos.xy = pos.z * float2(((float)id.x / width * 2 - 1) / proj_11,
        ((1 - (float)id.y / height) * 2 - 1) / proj_22);
    pos = mul(float4(pos, 1), view_inv).xyz;

    float3 albedo = gbuffer1[id.xy].rgb;
    float3 F0 = gbuffer1[id.xy].a * albedo;
    float3 N = normalize(2 * gbuffer2[id.xy].rgb - 1);
    float roughness = gbuffer2[id.xy].a;

    float3 V = normalize(eye_pos - pos);
    float NdV = saturate(dot(N, V));
    float a2 = pow(roughness, 4);
    float k = pow(roughness, 2) / 8;
    float Gv = (a2 / (4 * (NdV * (1 - k) + k)));

    float3 light;
    uint count, stride;
    lights.GetDimensions(count, stride);

    for (uint i = 0; i < count; ++i) {
        float3 L;
        float dist = 0;
        if (lights[i].range > 0) {
            L = lights[i].position - pos;
            dist = length(L);
            L /= dist;
        }
        else 
            L = normalize(-lights[i].direction);

        float NdL = saturate(dot(N, L));

        if (NdL > 0.f && dist <= lights[i].range) {
            float3 H = normalize(L + V);
            float NdH = saturate(dot(N, H));
            float VdH = saturate(dot(V, H));

            float D = 1 / pow(NdH * NdH * (a2 - 1) + 1, 2);
            float3 F = F0 + (1 - F0) * exp2((-5.55473 * VdH - 6.98316) * VdH);
            float G = Gv / (NdL * (1 - k) + k);
            float3 spec = D * F * G;

            float atten = 1;
            if (lights[i].range > 0)
                atten = 16 * pow(1 - pow(dist / lights[i].range, 4), 2) /
                (dist * dist + 1);
            if (lights[i].spot_angle > 0)
                atten *= pow(saturate(dot(-L, lights[i].direction)), 
                             lights[i].spot_angle);

            light += 
                (NdL + spec) * lights[i].color * lights[i].intensity * atten;
        }
    }

    render_target[id.xy] = float4(albedo * light, 1);
}