/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#include "Camera.h"
using namespace DirectX;

Camera::Camera() {}

Camera::Camera(float fov, float aspect_ratio, float near_z, float far_z,
    XMFLOAT3 position, XMVECTOR rotation) {
    this->fov = fov;
    this->aspect_ratio = aspect_ratio;
    this->near_z = near_z;
    this->far_z = far_z;
    this->position = position;
    this->rotation = rotation;
}

XMMATRIX Camera::GetProjection() const {
    return XMMatrixPerspectiveFovLH(fov, aspect_ratio, near_z, far_z);
}

XMMATRIX Camera::GetView() const {
    return XMMatrixLookToLH(XMLoadFloat3(&position),
        XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotation),
        XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotation));
}
