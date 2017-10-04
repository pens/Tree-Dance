/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <string>
#include <vector>
#include <d3d11.h>

struct Texture {
    std::vector<char> data;
    int height, width, depth;
    ID3D11ShaderResourceView* srv;
};

struct Material {
    Texture albedo;
    Texture normal;
    Texture roughness;
    Texture specular;
    Texture displacement;
};
