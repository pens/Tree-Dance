/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

struct HSIn {
    float3 position : POS;
    float3 normal : NORM;
    float3 tangent : TAN;
    float2 uv : TEX;
    float tess_fact : TESS;
};
struct HSOut {
    float3 position : POS;
    float3 normal : NORM;
    float3 tangent : TAN;
    float2 uv : TEX;
};

void PatchConstFunc(InputPatch<HSIn, 3> patch, uint patch_ID : SV_PrimitiveID,
                    out float edge_tess[3] : SV_TessFactor,
                    out float inside_tess : SV_InsideTessFactor) {
    edge_tess[0] = 0.5f * (patch[1].tess_fact + patch[2].tess_fact);
    edge_tess[1] = 0.5f * (patch[2].tess_fact + patch[0].tess_fact);
    edge_tess[2] = 0.5f * (patch[0].tess_fact + patch[1].tess_fact);
    inside_tess = edge_tess[0];
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstFunc")]
HSOut main(InputPatch<HSIn, 3> patch, uint i : SV_OutputControlPointID,
           uint patch_ID : SV_PrimitiveID) {
    HSOut hs;
    hs.position = patch[i].position;
    hs.normal = patch[i].normal;
    hs.tangent = patch[i].tangent;
    hs.uv = patch[i].uv;
    return hs;
}