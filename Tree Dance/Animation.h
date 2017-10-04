/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <string>
#include <vector>
#include <DirectXMath.h>

struct Key {
    DirectX::XMFLOAT4 rotation;
    DirectX::XMFLOAT3 translation;
    float scale;
};

struct Bone {
    DirectX::XMMATRIX local_bind_pose;
    DirectX::XMMATRIX inverse_global_bind_pose;
    int parent_index;
};

struct Skeleton {
    std::vector<Bone> bones;
};

struct Animation {
    std::vector<std::vector<Key>> channels;
    double duration;
    void SetBindPose(const Skeleton& skeleton, DirectX::XMMATRIX* transforms);
    void Animate(const Skeleton& skeleton, double time,
        DirectX::XMMATRIX* transforms);
    void FinalizeAnimation(const Skeleton& skeleton,
        DirectX::XMMATRIX* transforms);
};