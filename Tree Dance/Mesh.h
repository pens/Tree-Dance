/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <string>
#include <vector>
#include <DirectXMath.h>
#include <d3d11.h>

struct Vertex {
    DirectX::XMFLOAT3 position, normal, tangent;
    DirectX::XMFLOAT2 uv;
    int bones[4];
    float weights[4];
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    DirectX::XMMATRIX global_transform;
    ID3D11Buffer* vertex_buffer, *index_buffer;
};