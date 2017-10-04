/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#include "Core.h"
#include <cmath>
using namespace DirectX;

Core::Core() {}

void Core::Init(HWND hwnd) {
    camera_ = Camera{ 3.14f / 2.0f, 16.0f / 9.0f, .1f, 1000.0f, { 0, 0, -10 },
        XMQuaternionIdentity() };

    renderer_.Init(hwnd);

    tree_.Load("../Assets/tree/tree.model");
    tree_trans_.num_instances = 21;
    tree_trans_.transforms.resize(tree_.skeleton.bones.size() *
                                  tree_trans_.num_instances);
    tree_trans_.world_matrices.resize(2 * tree_trans_.num_instances);
    for (auto i = 0; i < tree_trans_.num_instances; ++i) {
        tree_trans_.world_matrices[2 * i] = tree_.meshes[0].global_transform *
            XMMatrixTranslation(static_cast<float>(cos(i * 0.3)) * 15.0f,
                                0.0f,
                                static_cast<float>(sin(i * 0.3)) * 15.0f);
        tree_trans_.world_matrices[2 * i + 1] = XMMatrixTranspose(
            XMMatrixInverse(nullptr, tree_trans_.world_matrices[2 * i]));
    }
    renderer_.LoadModel(&tree_);

    tree_large_.Load("../Assets/tree_large/tree_large.model");
    tree_large_trans_.num_instances = 1;
    tree_large_trans_.transforms.resize(tree_large_.skeleton.bones.size());
    tree_large_trans_.world_matrices.resize(2);
    tree_large_trans_.world_matrices[0] =
        tree_large_.meshes[0].global_transform;
    tree_large_trans_.world_matrices[1] =
            XMMatrixTranspose(
            XMMatrixInverse(nullptr, tree_large_trans_.world_matrices[0]));
    renderer_.LoadModel(&tree_large_);

    barrier_.Load("../Assets/barrier/barrier.model");
    barrier_trans_.num_instances = 1;
    barrier_trans_.world_matrices.resize(2);
    barrier_trans_.world_matrices[0] =
    barrier_trans_.world_matrices[1] =
        XMMatrixIdentity();
    renderer_.LoadModel(&barrier_);

    terrain_.Load("../Assets/terrain/terrain.model");
    terrain_trans_.num_instances = 1;
    terrain_trans_.world_matrices.resize(2);
    terrain_trans_.world_matrices[0] = XMMatrixScaling(5.0f, 5.0f, 5.0f);
    terrain_trans_.world_matrices[1] =
        XMMatrixTranspose(
        XMMatrixInverse(nullptr, terrain_trans_.world_matrices[0]));
    renderer_.LoadModel(&terrain_);

    lights.resize(10);
    for (int i = 0; i < lights.size(); ++i) {
        lights[i].position = {static_cast<float>(cos(i)) * 10.0f,
                              3.0f,
                              static_cast<float>(sin(i)) * 10.0f };
        lights[i].color = {static_cast<float>((cos(i * 0.3) + 1) / 2),
                           static_cast<float>((cos(i * 0.6) + 1) / 2),
                           static_cast<float>((cos(i * 0.9) + 1) / 2)};
        lights[i].intensity = 2.0f;
        lights[i].range = 100.0f;
    }
    renderer_.SetLights(lights);
}

void Core::Deinit() {
    renderer_.Deinit();
}

float yaw = 0;
float pitch = .5f;
float zoom = -15;
double oldtime = 0;

float Core::Run() {
    timer_.Tick();
    time += timer_.GetElapsed();

    if (input_.IsKeyDown('W')) {
        pitch += static_cast<float>(timer_.GetElapsed());
    } else if (input_.IsKeyDown('S')) {
        pitch -= static_cast<float>(timer_.GetElapsed());
    }
    if (input_.IsKeyDown('A')) {
        yaw += static_cast<float>(timer_.GetElapsed());
    } else if (input_.IsKeyDown('D')) {
        yaw -= static_cast<float>(timer_.GetElapsed());
    }
    if (input_.IsKeyDown('Q')) {
        zoom -= 5.0f * static_cast<float>(timer_.GetElapsed());
    } else if (input_.IsKeyDown('E')) {
        zoom += 5.0f * static_cast<float>(timer_.GetElapsed());
    }
    camera_.rotation = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0);
    XMFLOAT3 posf = {0, 0, zoom};
    XMVECTOR pos = XMLoadFloat3(&posf);
    XMStoreFloat3(&camera_.position, XMVector3Rotate(pos, camera_.rotation));
    renderer_.SetCamera(camera_);

    if (time - oldtime > 1.0 / 24) {
        oldtime = time;
        tree_.animations[0].SetBindPose(tree_.skeleton,
                                              tree_trans_.transforms.data());
        tree_.animations[0].Animate(tree_.skeleton, time,
                                          tree_trans_.transforms.data());
        tree_.animations[0].FinalizeAnimation(tree_.skeleton,
                                               tree_trans_.transforms.data());
        
        tree_.animations[0].SetBindPose(tree_.skeleton,
                                              tree_trans_.transforms.data() + tree_.skeleton.bones.size());
        tree_.animations[0].Animate(tree_.skeleton, time + tree_.animations[0].duration / 2.0,
                                          tree_trans_.transforms.data() + tree_.skeleton.bones.size());
        tree_.animations[0].FinalizeAnimation(tree_.skeleton,
                                               tree_trans_.transforms.data() + tree_.skeleton.bones.size());

        for (int i = 2; i < tree_trans_.num_instances; ++i)
            for (int j = 0; j < tree_.skeleton.bones.size(); ++j)
                tree_trans_.transforms[i * tree_.skeleton.bones.size() + j] =
                    tree_trans_.transforms[(i % 2) * tree_.skeleton.bones.size() + j];

        tree_large_.animations[0].SetBindPose(tree_large_.skeleton,
                                           tree_large_trans_.transforms.data());
        tree_large_.animations[0].Animate(tree_large_.skeleton, time,
                                          tree_large_trans_.transforms.data());
        tree_large_.animations[0].FinalizeAnimation(tree_large_.skeleton,
                                           tree_large_trans_.transforms.data());
    }

    renderer_.Clear();
    renderer_.Draw(&tree_, &tree_trans_);
    renderer_.Draw(&tree_large_, &tree_large_trans_);
    renderer_.Draw(&barrier_, &barrier_trans_);
    renderer_.Draw(&terrain_, &terrain_trans_);
    renderer_.Present();
    return static_cast<float>(timer_.GetElapsed(true));
}

void Core::HandleMSG(const MSG& msg) {
    input_.HandleMSG(msg);
}
