/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#include "Animation.h"
#include <cmath>
#include <string>
using namespace std;
using namespace DirectX;

void Animation::SetBindPose(const Skeleton& skeleton, XMMATRIX* transforms) {
    for (auto i = 0; i < skeleton.bones.size(); ++i) {
        transforms[i] = skeleton.bones[i].local_bind_pose;
    }
}

void Animation::Animate(const Skeleton& skeleton, double time,
    XMMATRIX* transforms) {

    //TODO remove 2 lines:
    static const int fps = 24;
    time = fmod(time, duration);

    for (auto i = 0; i < channels.size(); ++i) {
        int frame1 = static_cast<int>(time * fps);
        int frame2 = frame1 + 1;
        float blend = static_cast<float>(time * fps - frame1) / fps;

        XMVECTOR T = XMVectorLerp(XMLoadFloat3(
            &channels[i][frame1].translation), XMLoadFloat3(
            &channels[i][frame2].translation), blend);
        XMVECTOR R = XMQuaternionSlerp(XMLoadFloat4(
            &channels[i][frame1].rotation), XMLoadFloat4(
            &channels[i][frame2].rotation), blend);
        float s1 = channels[i][frame1].scale;
        float s2 = channels[i][frame2].scale;
        XMVECTOR S = XMVectorLerp(XMVectorSet(s1, s1, s1, 0),
            XMVectorSet(s2, s2, s2, 0), blend);

        transforms[i] =
            XMMatrixAffineTransformation(S, XMQuaternionIdentity(), R, T);
    }
}

void Animation::FinalizeAnimation(const Skeleton& skeleton,
    XMMATRIX* transforms) {
    for (auto i = 0; i < skeleton.bones.size(); ++i) {
        if (skeleton.bones[i].parent_index > -1) {
            transforms[i] *= transforms[skeleton.bones[i].parent_index];
        }
    }
    for (auto i = 0; i < skeleton.bones.size(); ++i) {
        transforms[i] =
            skeleton.bones[i].inverse_global_bind_pose * transforms[i];
    }
}