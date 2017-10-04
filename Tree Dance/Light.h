/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <DirectXMath.h>

struct Light {
    DirectX::XMFLOAT3 color;
    float intensity;
    DirectX::XMFLOAT3 position;
    float range;
    DirectX::XMFLOAT3 direction;
    float spot_angle;
};
