#pragma once
#include <vector>
#include <fbxsdk.h>

struct Vertex {
    float position[3], tangent[3], normal[3], uv[2];
    int bones[4];
    float weights[4];
    bool operator==(Vertex& rhs) {
        for (auto i = 0; i < 4; ++i) {
            if (bones[i] != rhs.bones[i] || weights[i] != rhs.weights[i])
                return false;
        }
        for (auto i = 0; i < 3; ++i) {
            if (position[i] != rhs.position[i] || normal[i] != rhs.normal[i] ||
                tangent[i] != rhs.tangent[i])
                return false;
        }
        for (auto i = 0; i < 2; ++i) {
            if (uv[i] != rhs.uv[i])
                return false;
        }
        return true;
    }
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    FbxAMatrix global_transform;
};

struct Texture {
    std::vector<char> data;
    int height, width, depth;
};

struct Material {
    Texture albedo;
    Texture normal;
    Texture roughness;
    Texture specular;
    Texture displacement;
};

struct Key {
    float rotation[4], translation[3], scale;
};

struct Bone {
    FbxAMatrix local_bind_pose;
    FbxAMatrix inverse_global_bind_pose;
    int parent_index;
    FbxNode* node;
};

struct Skeleton {
    std::vector<Bone> bones;
};

struct Animation {
    std::vector<std::vector<Key>> channels;
    double duration;
};

struct Model {
    std::vector<Mesh> meshes;
    Skeleton skeleton;
    std::vector<Animation> animations;
    Material material;
};