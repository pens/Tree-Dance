/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <DirectXMath.h>

struct Camera {
    Camera();
    Camera(float fov, float aspect_ratio, float near_z, float far_z, 
        DirectX::XMFLOAT3 position, DirectX::XMVECTOR rotation);
    DirectX::XMMATRIX GetProjection() const;
    DirectX::XMMATRIX GetView() const;
    DirectX::XMVECTOR rotation{};
    DirectX::XMFLOAT3 position{};
    float fov{}, aspect_ratio{}, near_z{}, far_z{};
};
